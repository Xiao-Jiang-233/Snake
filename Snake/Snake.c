// 贪吃蛇游戏
/*
 * 文件名: Snake.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <conio.h>
#include <windows.h>
#include <locale.h>
#include <wchar.h>
#include <stdarg.h>

// =============================================
// 常量定义
// =============================================

// 游戏区域大小（内部可玩区域）
#define GAME_WIDTH 20
#define GAME_HEIGHT 20

// 游戏池大小（包括边框）
#define POOL_WIDTH (GAME_WIDTH + 2)   // 左右各加1个边框
#define POOL_HEIGHT (GAME_HEIGHT + 2) // 上下各加1个边框

// 控制台显示相关常量
#define CONSOLE_WIDTH 80
#define CONSOLE_HEIGHT 30

// 游戏区域在控制台中的起始位置（左上角）
#define GAME_AREA_X 8
#define GAME_AREA_Y 4

// 游戏标题
#define GAME_TITLE L"贪吃蛇游戏 - 文字版"
#define GAME_TITLE_LENGTH 10 // 标题字符数

// 方向枚举
typedef enum
{
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
} Direction;

// 游戏池单元格类型
typedef enum
{
    CELL_EMPTY,            // 空单元格
    CELL_FOOD,             // 食物
    CELL_SNAKE_HEAD,       // 蛇头
    CELL_SNAKE_BODY_UP,    // 蛇身（向上）
    CELL_SNAKE_BODY_DOWN,  // 蛇身（向下）
    CELL_SNAKE_BODY_LEFT,  // 蛇身（向左）
    CELL_SNAKE_BODY_RIGHT, // 蛇身（向右）
    CELL_SNAKE_TAIL,       // 蛇尾
    CELL_WALL              // 墙壁
} CellType;

// 位置结构体
typedef struct
{
    int x;
    int y;
} Position;

// 蛇结构体 - 简化版本，只存储头尾位置
typedef struct
{
    Position head;            // 蛇头位置
    Position tail;            // 蛇尾位置
    int length;               // 蛇的长度
    Direction direction;      // 当前移动方向
    Direction next_direction; // 下一个方向（用于输入缓冲）
} Snake;

// 游戏状态结构体
typedef struct
{
    Snake snake;
    Position food;
    int score;
    bool game_over;
    int speed; // 游戏速度（毫秒）
} GameState;

// =============================================
// 全局变量
// =============================================

static HANDLE hConsole = NULL;                 // 控制台句柄
static CellType pool[POOL_HEIGHT][POOL_WIDTH]; // 游戏池
static GameState game;                         // 游戏状态
static int console_width = CONSOLE_WIDTH / 2;  // 控制台宽度
static int console_height = CONSOLE_HEIGHT;    // 控制台高度

// 函数原型声明
static void init_game(void);
static void generate_food(void);
static void draw_game(void);
static void update_game(void);
static bool handle_input(void);
static void printf_at(int x, int y, WORD attributes, const wchar_t *fmt, ...);
static Position get_next_position(Position pos);

// =============================================
// Windows API控制台输出函数
// =============================================

// 初始化控制台：获取句柄、设置编码、强制设置窗口大小、隐藏光标
static void init_console(void)
{
    // 获取控制台句柄
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE)
    {
        MessageBoxW(NULL, L"无法获取控制台句柄", L"错误", MB_OK | MB_ICONERROR);
        exit(1);
    }

    // 设置控制台代码页为UTF-8以支持中文显示
    SetConsoleOutputCP(CP_UTF8);
    setlocale(LC_ALL, "zh_CN.UTF-8");

    // 获取当前控制台信息
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (!GetConsoleScreenBufferInfo(hConsole, &csbi))
    {
        // 如果无法获取信息，使用默认值
        console_width = CONSOLE_WIDTH / 2;
        console_height = CONSOLE_HEIGHT;
    }
    else
    {
        // 使用当前控制台尺寸作为参考
        console_width = csbi.dwSize.X / 2;
        console_height = csbi.dwSize.Y;
    }

    // 设置控制台缓冲区大小（80列 × 30行）
    COORD bufferSize = {CONSOLE_WIDTH, CONSOLE_HEIGHT};
    if (!SetConsoleScreenBufferSize(hConsole, bufferSize))
    {
        // 如果设置失败，尝试使用当前缓冲区大小
        if (GetConsoleScreenBufferInfo(hConsole, &csbi))
        {
            bufferSize = csbi.dwSize;
            // 调整全局变量以匹配实际缓冲区大小
            console_width = bufferSize.X / 2;
            console_height = bufferSize.Y;
        }
    }

    // 设置控制台窗口大小（80列 × 30行）
    SMALL_RECT windowSize = {0, 0, CONSOLE_WIDTH - 1, CONSOLE_HEIGHT - 1}; // 从(0,0)到(79,29)
    if (!SetConsoleWindowInfo(hConsole, TRUE, &windowSize))
    {
        // 如果设置窗口大小失败，尝试调整到合适的尺寸
        // 首先获取最大允许的窗口尺寸
        COORD maxWindowSize = GetLargestConsoleWindowSize(hConsole);

        // 确保请求的尺寸不超过最大允许尺寸
        int requestedWidth = CONSOLE_WIDTH;
        int requestedHeight = CONSOLE_HEIGHT;

        if (requestedWidth > maxWindowSize.X)
            requestedWidth = maxWindowSize.X;
        if (requestedHeight > maxWindowSize.Y)
            requestedHeight = maxWindowSize.Y;

        windowSize.Left = 0;
        windowSize.Top = 0;
        windowSize.Right = requestedWidth - 1;
        windowSize.Bottom = requestedHeight - 1;
        SetConsoleWindowInfo(hConsole, TRUE, &windowSize);

        // 如果窗口尺寸小于预期，可能需要调整游戏布局
        if (requestedWidth < CONSOLE_WIDTH || requestedHeight < CONSOLE_HEIGHT)
        {
            // 这里可以添加逻辑来调整游戏显示位置
            // 目前只是简单记录
        }
    }

    // 隐藏光标
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}

// 设置控制台光标位置
static void set_cursor_position(int x, int y)
{
    COORD coord = {x, y};
    SetConsoleCursorPosition(hConsole, coord);
}

// 统一的控制台输出函数：定位、更改颜色、格式化输出
// 功能：在控制台指定位置以指定颜色输出格式化的宽字符文本
// 参数：
//   - x: 输出位置的X坐标（控制台列数，0为最左侧）
//   - y: 输出位置的Y坐标（控制台行数，0为最上方）
//   - attributes: 文本属性（颜色、亮度等），使用Windows API的FOREGROUND_*常量组合
//   - fmt: 格式化字符串（宽字符），支持标准printf格式说明符
//   - ...: 可变参数列表，根据fmt中的格式说明符提供相应的参数
// 实现：
//   1. 使用SetConsoleCursorPosition将光标移动到指定位置
//   2. 使用SetConsoleTextAttribute设置文本颜色属性
//   3. 使用vwprintf进行格式化输出（宽字符版本）
// 注意：
//   - 此函数会改变光标位置和文本颜色，调用后控制台的状态会改变
//   - 使用宽字符字符串(L"...")确保Unicode支持
//   - 对于不需要格式化的纯文本输出，可以省略可变参数
static void printf_at(int x, int y, WORD attributes, const wchar_t *fmt, ...)
{
    // 设置光标位置
    COORD coord = {x * 2, y};
    SetConsoleCursorPosition(hConsole, coord);

    // 设置文本属性（颜色）
    SetConsoleTextAttribute(hConsole, attributes);

    // 格式化输出
    va_list args;
    va_start(args, fmt);
    vwprintf(fmt, args);
    va_end(args);
}

// 清空控制台屏幕
static void clear_screen(void)
{
    // 使用系统命令清屏
    system("cls");
    // 设置光标到左上角
    set_cursor_position(0, 0);
}

// =============================================
// 游戏池定位和绘制函数
// =============================================

// 获取游戏池单元格的控制台位置（游戏池子定位函数）
static Position get_cell_console_position(int pool_x, int pool_y)
{
    Position pos;
    pos.x = GAME_AREA_X + pool_x;
    pos.y = GAME_AREA_Y + pool_y;
    return pos;
}

// 绘制游戏池中的一个单元格
static void draw_cell(int pool_x, int pool_y)
{
    CellType cell = pool[pool_y][pool_x];
    Position console_pos = get_cell_console_position(pool_x, pool_y);
    WORD attributes = 0;
    const wchar_t *wstr = L"  "; // 默认两个空格

    switch (cell)
    {
    case CELL_EMPTY:
        wstr = L"  ";
        attributes = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
        break;
    case CELL_FOOD:
        wstr = L"★"; // 星号作为食物
        attributes = FOREGROUND_RED | FOREGROUND_INTENSITY;
        break;
    case CELL_SNAKE_HEAD:
        wstr = L"头"; // 用"头"字表示蛇头
        attributes = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
        break;
    case CELL_SNAKE_BODY_UP:
    case CELL_SNAKE_BODY_DOWN:
    case CELL_SNAKE_BODY_LEFT:
    case CELL_SNAKE_BODY_RIGHT:
        wstr = L"蛇";
        attributes = FOREGROUND_GREEN;
        break;
    case CELL_SNAKE_TAIL:
        wstr = L"尾"; // 用"尾"字表示蛇尾
        attributes = FOREGROUND_GREEN;
        break;
    case CELL_WALL:
        wstr = L"墙"; // 用"墙"字表示墙壁
        // 白底黑字：白色背景，黑色前景（前景色为0表示黑色）
        attributes = BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY;
        break;
    }

    // 绘制单元格
    printf_at(console_pos.x, console_pos.y, attributes, L"%ls", wstr);
}

// =============================================
// 游戏池初始化和管理
// =============================================

// 初始化游戏池
static void init_pool(void)
{
    // 清空所有单元格
    for (int y = 0; y < POOL_HEIGHT; y++)
    {
        for (int x = 0; x < POOL_WIDTH; x++)
        {
            pool[y][x] = CELL_EMPTY;
        }
    }

    // 设置墙壁
    for (int x = 0; x < POOL_WIDTH; x++)
    {
        pool[0][x] = CELL_WALL;               // 上边框
        pool[POOL_HEIGHT - 1][x] = CELL_WALL; // 下边框
    }
    for (int y = 0; y < POOL_HEIGHT; y++)
    {
        pool[y][0] = CELL_WALL;              // 左边框
        pool[y][POOL_WIDTH - 1] = CELL_WALL; // 右边框
    }
}

// 在游戏池中设置单元格类型
static void set_cell_type(int x, int y, CellType type)
{
    if (x >= 0 && x < POOL_WIDTH && y >= 0 && y < POOL_HEIGHT)
    {
        pool[y][x] = type;
    }
}

// 获取游戏池中单元格类型
static CellType get_cell_type(int x, int y)
{
    if (x >= 0 && x < POOL_WIDTH && y >= 0 && y < POOL_HEIGHT)
    {
        return pool[y][x];
    }
    return CELL_WALL; // 越界视为墙壁
}

// =============================================
// 游戏逻辑函数
// =============================================

// 初始化游戏状态
static void init_game(void)
{
    // 初始化随机数种子
    srand((unsigned int)time(NULL) + 325u);

    // 初始化控制台
    init_console();

    // 初始化游戏状态
    game.score = 0;
    game.game_over = false;
    game.speed = 120; // 初始速度120毫秒

    // 初始化蛇
    game.snake.length = 3;
    game.snake.direction = DIR_RIGHT;
    game.snake.next_direction = DIR_RIGHT;

    // 蛇的初始位置（在游戏区域中央）
    int start_x = POOL_WIDTH / 2;
    int start_y = POOL_HEIGHT / 2;

    // 设置蛇头和蛇尾位置
    game.snake.head.x = start_x;
    game.snake.head.y = start_y;
    game.snake.tail.x = start_x - (game.snake.length - 1);
    game.snake.tail.y = start_y;

    // 初始化游戏池
    init_pool();

    // 在游戏池中设置蛇的位置
    // 设置蛇头
    set_cell_type(game.snake.head.x, game.snake.head.y, CELL_SNAKE_HEAD);

    // 设置蛇身（从头到尾，每个部分指向下一个更靠近头部的部分）
    Position prev = game.snake.head; // 前一个部分（更靠近头部）
    for (int i = 1; i < game.snake.length - 1; i++)
    {
        Position current;
        current.x = start_x - i;
        current.y = start_y;

        // 设置当前身体部分的方向（指向prev）
        CellType body_type;
        if (prev.x == current.x)
        {
            if (prev.y < current.y) // prev在上方，所以当前向下移动
                body_type = CELL_SNAKE_BODY_DOWN;
            else // prev在下方，当前向上移动
                body_type = CELL_SNAKE_BODY_UP;
        }
        else // prev.y == current.y
        {
            if (prev.x < current.x) // prev在左边，所以当前向右移动
                body_type = CELL_SNAKE_BODY_RIGHT;
            else // prev在右边，当前向左移动
                body_type = CELL_SNAKE_BODY_LEFT;
        }
        set_cell_type(current.x, current.y, body_type);
        prev = current;
    }

    // 设置蛇尾
    set_cell_type(game.snake.tail.x, game.snake.tail.y, CELL_SNAKE_TAIL);

    // 生成第一个食物
    generate_food();
}

// 生成食物
static void generate_food(void)
{
    int x, y;
    int attempts = 0;
    const int max_attempts = POOL_WIDTH * POOL_HEIGHT * 2;

    do
    {
        x = rand() % POOL_WIDTH;
        y = rand() % POOL_HEIGHT;
        attempts++;

        if (attempts > max_attempts)
        {
            // 如果找不到合适的位置，游戏胜利
            game.game_over = true;
            return;
        }
    } while (get_cell_type(x, y) != CELL_EMPTY);

    game.food.x = x;
    game.food.y = y;
    set_cell_type(x, y, CELL_FOOD);
}

// 绘制游戏界面
static void draw_game(void)
{
    // 清空控制台
    clear_screen();

    // 绘制标题（console_width是字符数，直接计算居中位置）
    printf_at((console_width - GAME_TITLE_LENGTH) / 2, 1, FOREGROUND_GREEN | FOREGROUND_INTENSITY,
              GAME_TITLE);

    // 绘制游戏池
    for (int y = 0; y < POOL_HEIGHT; y++)
    {
        for (int x = 0; x < POOL_WIDTH; x++)
        {
            draw_cell(x, y);
        }
    }

    // 右侧信息区域起始位置（从右边偏移20列，即10个字符位置）
    int right_info_x = console_width / 2 + 9;
    int info_y = GAME_AREA_Y + 2;

    // 绘制分数信息
    printf_at(right_info_x, info_y + 0, FOREGROUND_GREEN | FOREGROUND_INTENSITY,
              L"得分: %d", game.score);

    // 绘制速度信息
    printf_at(right_info_x, info_y + 1, FOREGROUND_BLUE | FOREGROUND_INTENSITY,
              L"速度: %dms", game.speed);

    // 绘制控制说明
    printf_at(right_info_x, info_y + 4,
              FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY,
              L"控制: WASD 或 方向键");
    printf_at(right_info_x, info_y + 5,
              FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY,
              L"退出: ESC 或 Q");

    // 绘制制作人信息
    printf_at(right_info_x, info_y + 16,
              FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
              L"制作: 刘庆棋");
    printf_at(right_info_x, info_y + 17,
              FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
              L"      宫子涵");
    printf_at(right_info_x, info_y + 18,
              FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
              L"      贾国威");

    // 如果游戏结束，显示游戏结束信息
    if (game.game_over)
    {
        // 计算游戏池中心位置
        int pool_center_x = GAME_AREA_X + POOL_WIDTH / 2;
        int pool_center_y = GAME_AREA_Y + POOL_HEIGHT / 2;

        printf_at(pool_center_x - 2, pool_center_y - 1,
                  FOREGROUND_RED | FOREGROUND_INTENSITY,
                  L"游戏结束！");

        printf_at(pool_center_x - 3, pool_center_y,
                  FOREGROUND_GREEN | FOREGROUND_INTENSITY,
                  L"最终得分: %d", game.score);

        printf_at(pool_center_x - 4, pool_center_y + 1,
                  FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
                  L"按任意键退出游戏...");
    }
}

// 根据方向获取对应的蛇身类型
static CellType direction_to_body_type(Direction dir)
{
    switch (dir)
    {
    case DIR_UP:
        return CELL_SNAKE_BODY_UP;
    case DIR_DOWN:
        return CELL_SNAKE_BODY_DOWN;
    case DIR_LEFT:
        return CELL_SNAKE_BODY_LEFT;
    case DIR_RIGHT:
        return CELL_SNAKE_BODY_RIGHT;
    default:
        return CELL_SNAKE_BODY_RIGHT; // 默认
    }
}

// 更新游戏逻辑 - 新版本，只使用头尾位置
static void update_game(void)
{
    if (game.game_over)
    {
        return;
    }

    // 应用输入的方向
    game.snake.direction = game.snake.next_direction;

    // 获取当前蛇头位置
    Position head = game.snake.head;
    Position new_head = head;

    // 根据方向计算新蛇头位置
    switch (game.snake.direction)
    {
    case DIR_UP:
        new_head.y--;
        break;
    case DIR_DOWN:
        new_head.y++;
        break;
    case DIR_LEFT:
        new_head.x--;
        break;
    case DIR_RIGHT:
        new_head.x++;
        break;
    }

    // 检查碰撞
    CellType cell_ahead = get_cell_type(new_head.x, new_head.y);

    if (cell_ahead == CELL_WALL || cell_ahead == CELL_SNAKE_BODY_UP || cell_ahead == CELL_SNAKE_BODY_DOWN ||
        cell_ahead == CELL_SNAKE_BODY_LEFT || cell_ahead == CELL_SNAKE_BODY_RIGHT || cell_ahead == CELL_SNAKE_TAIL)
    {
        // 撞墙或撞到自己身体，游戏结束
        game.game_over = true;
        return;
    }

    // 检查是否吃到食物
    bool ate_food = (cell_ahead == CELL_FOOD);

    // 更新游戏池和蛇的位置
    if (!ate_food)
    {
        // 没吃到食物，需要移动蛇尾
        // 先找到蛇尾的前一个位置（沿着蛇身向头部方向）
        Position next_from_tail = get_next_position(game.snake.tail);

        // 清除当前蛇尾
        set_cell_type(game.snake.tail.x, game.snake.tail.y, CELL_EMPTY);

        // 将前一个位置设为新的蛇尾
        set_cell_type(next_from_tail.x, next_from_tail.y, CELL_SNAKE_TAIL);

        // 更新蛇尾位置
        game.snake.tail = next_from_tail;
    }
    else
    {
        // 吃到食物，蛇长度增加，蛇尾不动
        game.snake.length++;
        game.score += 10;

        // 每得50分增加速度
        if (game.score % 50 == 0 && game.speed > 30)
        {
            game.speed -= 10;
        }

        // 生成新的食物
        generate_food();
    }

    // 将旧蛇头变为蛇身（根据移动方向）
    CellType old_head_type = direction_to_body_type(game.snake.direction);
    set_cell_type(head.x, head.y, old_head_type);

    // 设置新蛇头
    set_cell_type(new_head.x, new_head.y, CELL_SNAKE_HEAD);

    // 更新蛇头位置
    game.snake.head = new_head;
}

// 处理输入
static bool handle_input(void)
{
    if (_kbhit())
    {
        int ch = _getch();

        // 处理方向键（扩展键码）
        if (ch == 0 || ch == 224)
        {
            ch = _getch();
            switch (ch)
            {
            case 72: // 上箭头
                if (game.snake.direction != DIR_DOWN)
                    game.snake.next_direction = DIR_UP;
                break;
            case 80: // 下箭头
                if (game.snake.direction != DIR_UP)
                    game.snake.next_direction = DIR_DOWN;
                break;
            case 75: // 左箭头
                if (game.snake.direction != DIR_RIGHT)
                    game.snake.next_direction = DIR_LEFT;
                break;
            case 77: // 右箭头
                if (game.snake.direction != DIR_LEFT)
                    game.snake.next_direction = DIR_RIGHT;
                break;
            }
        }
        else
        {
            // 处理普通键
            switch (ch)
            {
            case 'w':
            case 'W':
                if (game.snake.direction != DIR_DOWN)
                    game.snake.next_direction = DIR_UP;
                break;
            case 's':
            case 'S':
                if (game.snake.direction != DIR_UP)
                    game.snake.next_direction = DIR_DOWN;
                break;
            case 'a':
            case 'A':
                if (game.snake.direction != DIR_RIGHT)
                    game.snake.next_direction = DIR_LEFT;
                break;
            case 'd':
            case 'D':
                if (game.snake.direction != DIR_LEFT)
                    game.snake.next_direction = DIR_RIGHT;
                break;
            case 'q':
            case 'Q':
            case 27:          // ESC
                return false; // 退出游戏
            }
        }
    }

    return true; // 继续游戏
}

// =============================================
// 辅助函数：根据位置获取下一个位置
// =============================================

// 根据当前位置的类型获取下一个位置
static Position get_next_position(Position pos)
{
    Position next = pos;
    CellType cell = get_cell_type(pos.x, pos.y);

    switch (cell)
    {
    case CELL_SNAKE_HEAD:
        // 蛇头根据当前方向移动
        switch (game.snake.direction)
        {
        case DIR_UP:
            next.y--;
            break;
        case DIR_DOWN:
            next.y++;
            break;
        case DIR_LEFT:
            next.x--;
            break;
        case DIR_RIGHT:
            next.x++;
            break;
        }
        break;

    case CELL_SNAKE_BODY_UP:
        next.y--; // 向上移动
        break;
    case CELL_SNAKE_BODY_DOWN:
        next.y++; // 向下移动
        break;
    case CELL_SNAKE_BODY_LEFT:
        next.x--; // 向左移动
        break;
    case CELL_SNAKE_BODY_RIGHT:
        next.x++; // 向右移动
        break;

    case CELL_SNAKE_TAIL:
        // 蛇尾需要找到相邻的蛇身来确定方向
        // 检查四个方向，找到任何类型的蛇身部分
        CellType left_cell = get_cell_type(pos.x - 1, pos.y);
        CellType right_cell = get_cell_type(pos.x + 1, pos.y);
        CellType up_cell = get_cell_type(pos.x, pos.y - 1);
        CellType down_cell = get_cell_type(pos.x, pos.y + 1);

        // 检查是否是蛇身（任何方向）
        if (left_cell == CELL_SNAKE_BODY_UP || left_cell == CELL_SNAKE_BODY_DOWN ||
            left_cell == CELL_SNAKE_BODY_LEFT || left_cell == CELL_SNAKE_BODY_RIGHT)
            next.x--; // 左边有蛇身，蛇尾向左移动
        else if (right_cell == CELL_SNAKE_BODY_UP || right_cell == CELL_SNAKE_BODY_DOWN ||
                 right_cell == CELL_SNAKE_BODY_LEFT || right_cell == CELL_SNAKE_BODY_RIGHT)
            next.x++; // 右边有蛇身，蛇尾向右移动
        else if (up_cell == CELL_SNAKE_BODY_UP || up_cell == CELL_SNAKE_BODY_DOWN ||
                 up_cell == CELL_SNAKE_BODY_LEFT || up_cell == CELL_SNAKE_BODY_RIGHT)
            next.y--; // 上方有蛇身，蛇尾向上移动
        else if (down_cell == CELL_SNAKE_BODY_UP || down_cell == CELL_SNAKE_BODY_DOWN ||
                 down_cell == CELL_SNAKE_BODY_LEFT || down_cell == CELL_SNAKE_BODY_RIGHT)
            next.y++; // 下方有蛇身，蛇尾向下移动
        break;

    default:
        // 对于其他类型，位置不变
        break;
    }

    return next;
}

// =============================================
// 主函数
// =============================================

int main()
{
    // 初始化游戏
    init_game();

    // 显示开始界面
    clear_screen();
    printf_at(console_width / 2 - GAME_TITLE_LENGTH / 2, console_height / 2 - 6,
              FOREGROUND_GREEN | FOREGROUND_INTENSITY,
              GAME_TITLE);
    printf_at(console_width / 2 - 5, console_height / 2 - 3,
              FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
              L"按任意键开始游戏...");
    _getch();

    // 游戏主循环
    while (!game.game_over && handle_input())
    {
        update_game();
        draw_game();

        // 控制游戏速度
        Sleep(game.speed);
    }

    // 显示最终画面
    draw_game();

    // 等待用户按键退出
    if (!_kbhit())
        _getch();

    return 0;
}
