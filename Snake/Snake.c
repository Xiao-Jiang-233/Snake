/*
 * 贪吃蛇游戏 (Snake Game)
 *
 * 文件: Snake.c
 * 描述: Windows控制台版本的贪吃蛇游戏，使用C语言编写，基于Windows API实现图形界面
 * 功能: 包含游戏逻辑、控制台图形显示、用户输入处理等完整实现
 * 编码: UTF-8 (支持中文字符显示)
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

// 游戏区域大小（内部可玩区域，不包含边框）
#define GAME_WIDTH 20                  // 游戏区域宽度（单元格数）
#define GAME_HEIGHT 20                 // 游戏区域高度（单元格数）

// 游戏池大小（包括边框）
#define POOL_WIDTH (GAME_WIDTH + 2)    // 游戏池宽度 = 游戏宽度 + 左右边框
#define POOL_HEIGHT (GAME_HEIGHT + 2)  // 游戏池高度 = 游戏高度 + 上下边框

// 控制台显示相关常量
#define CONSOLE_WIDTH 80               // 控制台缓冲区宽度（字符数）
#define CONSOLE_HEIGHT 30              // 控制台缓冲区高度（行数）

// 游戏区域在控制台中的起始位置（左上角坐标）
#define GAME_AREA_X 8                  // 游戏区域起始X坐标（列数）
#define GAME_AREA_Y 4                  // 游戏区域起始Y坐标（行数）

// 游戏标题
#define GAME_TITLE L"贪吃蛇游戏 - 文字版"  // 游戏标题（宽字符字符串）
#define GAME_TITLE_LENGTH 10               // 标题字符数（用于居中计算）

// 方向枚举 - 表示蛇的移动方向
typedef enum
{
    DIR_UP,      // 向上移动
    DIR_DOWN,    // 向下移动
    DIR_LEFT,    // 向左移动
    DIR_RIGHT    // 向右移动
} Direction;

// 游戏池单元格类型 - 表示游戏网格中每个单元格的状态
typedef enum
{
    CELL_EMPTY,            // 空单元格（可通行区域）
    CELL_FOOD,             // 食物（蛇的目标）
    CELL_SNAKE_HEAD,       // 蛇头（蛇的头部，控制移动方向）
    CELL_SNAKE_BODY_UP,    // 蛇身（向上移动方向）
    CELL_SNAKE_BODY_DOWN,  // 蛇身（向下移动方向）
    CELL_SNAKE_BODY_LEFT,  // 蛇身（向左移动方向）
    CELL_SNAKE_BODY_RIGHT, // 蛇身（向右移动方向）
    CELL_SNAKE_TAIL,       // 蛇尾（蛇的尾部，最后一段）
    CELL_WALL              // 墙壁（不可通行的边界）
} CellType;

// 位置结构体 - 表示二维坐标系中的点（用于游戏池和控制台坐标）
typedef struct
{
    int x;  // X坐标（水平方向）
    int y;  // Y坐标（垂直方向）
} Position;

// 蛇结构体 - 简化版本，只存储头尾位置（基于链表思想，但仅记录关键位置）
typedef struct
{
    Position head;            // 蛇头位置（当前头部坐标）
    Position tail;            // 蛇尾位置（当前尾部坐标）
    int length;               // 蛇的长度（包括头、身、尾）
    Direction direction;      // 当前移动方向（正在执行的方向）
    Direction next_direction; // 下一个方向（用于输入缓冲，防止连续转向）
} Snake;

// 游戏状态结构体 - 包含游戏运行所需的所有状态信息
typedef struct
{
    Snake snake;          // 蛇的状态（位置、长度、方向等）
    Position food;        // 食物位置
    int score;            // 当前得分
    bool game_over;       // 游戏结束标志（true表示游戏结束）
    int speed;            // 游戏速度（毫秒，控制蛇移动的延迟时间）
} GameState;

// =============================================
// 全局变量
// =============================================

static HANDLE hConsole = NULL;                 // Windows控制台句柄，用于所有控制台输出操作
static CellType pool[POOL_HEIGHT][POOL_WIDTH]; // 游戏池二维数组，存储每个单元格的当前状态
static GameState game;                         // 游戏状态实例，包含蛇、食物、分数等所有游戏数据
static int console_width = CONSOLE_WIDTH / 2;  // 实际控制台宽度（字符数，考虑宽字符显示）
static int console_height = CONSOLE_HEIGHT;    // 实际控制台高度（行数）

// 函数原型声明
static void init_game(void);                               // 初始化游戏状态和控制台
static void generate_food(void);                           // 在随机位置生成食物
static void draw_game(void);                               // 绘制整个游戏界面
static void update_game(void);                             // 更新游戏逻辑（蛇移动、碰撞检测等）
static bool handle_input(void);                            // 处理用户输入，返回false表示退出游戏
static void printf_at(int x, int y, WORD attributes, const wchar_t *fmt, ...); // 在控制台指定位置格式化输出
static Position get_next_position(Position pos);           // 根据当前位置类型获取下一个位置（用于蛇尾移动）

// =============================================
// Windows API控制台输出函数
// =============================================

/**
 * 初始化控制台环境
 *
 * 功能：获取控制台句柄、设置UTF-8编码、调整控制台窗口和缓冲区大小、隐藏光标。
 * 此函数必须在任何控制台输出操作之前调用，确保控制台处于正确的初始状态。
 *
 * 实现步骤：
 * 1. 获取标准输出句柄
 * 2. 设置控制台代码页为UTF-8以支持中文显示
 * 3. 获取当前控制台尺寸作为参考
 * 4. 设置控制台缓冲区大小（80x30）
 * 5. 设置控制台窗口大小（如果失败则尝试最大允许尺寸）
 * 6. 隐藏光标以提高视觉体验
 *
 * 注意：如果无法获取控制台句柄，程序将显示错误消息并退出。
 */
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

/**
 * 设置控制台光标位置
 *
 * 功能：将控制台光标移动到指定的(x, y)坐标位置。
 * 使用Windows API的SetConsoleCursorPosition函数实现。
 *
 * @param x 目标位置的X坐标（列数，0为最左侧）
 * @param y 目标位置的Y坐标（行数，0为最上方）
 */
static void set_cursor_position(int x, int y)
{
    COORD coord = {x, y};
    SetConsoleCursorPosition(hConsole, coord);
}

/**
 * 统一的控制台输出函数：定位、更改颜色、格式化输出
 *
 * 功能：在控制台指定位置以指定颜色输出格式化的宽字符文本。
 * 此函数封装了控制台光标定位、颜色设置和格式化输出的完整流程。
 *
 * 参数：
 *   @param x 输出位置的X坐标（控制台列数，0为最左侧）
 *   @param y 输出位置的Y坐标（控制台行数，0为最上方）
 *   @param attributes 文本属性（颜色、亮度等），使用Windows API的FOREGROUND_*常量组合
 *   @param fmt 格式化字符串（宽字符），支持标准printf格式说明符
 *   @param ... 可变参数列表，根据fmt中的格式说明符提供相应的参数
 *
 * 实现步骤：
 *   1. 使用SetConsoleCursorPosition将光标移动到指定位置（注意宽字符显示需要x*2）
 *   2. 使用SetConsoleTextAttribute设置文本颜色属性
 *   3. 使用vwprintf进行格式化输出（宽字符版本）
 *
 * 注意事项：
 *   - 此函数会改变光标位置和文本颜色，调用后控制台的状态会改变
 *   - 使用宽字符字符串(L"...")确保Unicode支持
 *   - 对于不需要格式化的纯文本输出，可以省略可变参数（但fmt参数仍需提供）
 *   - 由于控制台中文字符宽度为2个英文字符，X坐标需要乘以2进行显示对齐
 */
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

/**
 * 清空控制台屏幕
 *
 * 功能：清除控制台中的所有内容，并将光标重置到左上角(0,0)位置。
 * 使用系统命令"cls"实现清屏操作，然后调用set_cursor_position重置光标。
 *
 * 注意：此函数会清除控制台中的所有输出，包括游戏界面。
 */
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

/**
 * 获取游戏池单元格的控制台位置
 *
 * 功能：将游戏池坐标(x,y)转换为控制台屏幕坐标。
 * 游戏池坐标以(0,0)为左上角，控制台坐标以GAME_AREA_X和GAME_AREA_Y为偏移基准。
 *
 * @param pool_x 游戏池中的X坐标（单元格索引，0~POOL_WIDTH-1）
 * @param pool_y 游戏池中的Y坐标（单元格索引，0~POOL_HEIGHT-1）
 * @return Position 对应的控制台坐标位置
 */
static Position get_cell_console_position(int pool_x, int pool_y)
{
    Position pos;
    pos.x = GAME_AREA_X + pool_x;
    pos.y = GAME_AREA_Y + pool_y;
    return pos;
}

/**
 * 绘制游戏池中的一个单元格
 *
 * 功能：根据单元格类型在控制台对应位置绘制相应的字符和颜色。
 * 此函数将游戏池中的抽象单元格状态转换为可视化的控制台输出。
 *
 * 支持的单元格类型和显示方式：
 *   - CELL_EMPTY: 空白（两个空格）
 *   - CELL_FOOD:  红色星号"★"
 *   - CELL_SNAKE_HEAD: 绿色"头"字
 *   - CELL_SNAKE_BODY: 绿色"蛇"字（忽略具体方向）
 *   - CELL_SNAKE_TAIL: 绿色"尾"字
 *   - CELL_WALL:   白色背景黑色"墙"字
 *
 * @param pool_x 游戏池中的X坐标（单元格索引）
 * @param pool_y 游戏池中的Y坐标（单元格索引）
 */
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

/**
 * 初始化游戏池
 *
 * 功能：将游戏池二维数组所有单元格初始化为CELL_EMPTY，并设置四周边框为CELL_WALL。
 * 此函数在游戏开始时调用，创建游戏的基本网格结构。
 *
 * 实现步骤：
 *   1. 遍历所有单元格，设置为CELL_EMPTY
 *   2. 设置上边框和下边框为CELL_WALL
 *   3. 设置左边框和右边框为CELL_WALL
 */
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

/**
 * 在游戏池中设置单元格类型
 *
 * 功能：将游戏池中指定坐标的单元格设置为指定的类型。
 * 此函数包含边界检查，确保坐标在有效范围内。
 *
 * @param x    目标单元格的X坐标（0~POOL_WIDTH-1）
 * @param y    目标单元格的Y坐标（0~POOL_HEIGHT-1）
 * @param type 要设置的单元格类型（CellType枚举值）
 */
static void set_cell_type(int x, int y, CellType type)
{
    if (x >= 0 && x < POOL_WIDTH && y >= 0 && y < POOL_HEIGHT)
    {
        pool[y][x] = type;
    }
}

/**
 * 获取游戏池中单元格类型
 *
 * 功能：获取游戏池中指定坐标的单元格当前类型。
 * 此函数包含边界检查，如果坐标越界则返回CELL_WALL（视为墙壁）。
 *
 * @param x 目标单元格的X坐标
 * @param y 目标单元格的Y坐标
 * @return CellType 指定坐标的单元格类型，如果越界则返回CELL_WALL
 */
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

/**
 * 初始化游戏状态
 *
 * 功能：初始化游戏的完整状态，包括控制台环境、游戏池、蛇的初始位置、食物生成等。
 * 此函数是游戏启动时必须调用的第一个函数，负责设置所有全局变量和游戏数据结构。
 *
 * 实现步骤：
 *   1. 初始化随机数种子（用于食物生成）
 *   2. 初始化控制台环境（编码、窗口大小、光标等）
 *   3. 初始化游戏状态变量（分数、速度、游戏结束标志）
 *   4. 初始化蛇的状态（长度、方向、初始位置）
 *   5. 初始化游戏池（清空所有单元格并设置边框）
 *   6. 在游戏池中设置蛇的初始位置（头、身、尾）
 *   7. 生成第一个食物
 */
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

    // 设置蛇身
    set_cell_type(game.snake.head.x - 1, game.snake.head.y, CELL_SNAKE_BODY_RIGHT);

    // 设置蛇尾
    set_cell_type(game.snake.tail.x, game.snake.tail.y, CELL_SNAKE_TAIL);

    // 生成第一个食物
    generate_food();
}

/**
 * 生成食物
 *
 * 功能：在游戏池的随机空单元格中生成食物。
 * 使用随机数生成器选择坐标，确保不会在蛇身体或墙壁上生成食物。
 *
 * 实现步骤：
 *   1. 随机生成X和Y坐标
 *   2. 检查该位置是否为CELL_EMPTY
 *   3. 如果不是空单元格，则重试（最多尝试POOL_WIDTH * POOL_HEIGHT * 2次）
 *   4. 如果找不到合适位置，游戏结束（视为胜利）
 *   5. 在找到的空单元格设置CELL_FOOD类型，并更新game.food位置
 */
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

/**
 * 绘制游戏界面
 *
 * 功能：绘制完整的游戏界面，包括标题、游戏池、分数信息、控制说明和制作人信息。
 * 此函数负责将所有游戏状态可视化为控制台输出。
 *
 * 绘制内容：
 *   1. 清空控制台并绘制居中标题
 *   2. 遍历游戏池所有单元格，调用draw_cell绘制每个单元格
 *   3. 在右侧信息区域显示分数、速度、控制说明
 *   4. 显示制作人信息
 *   5. 如果游戏结束，显示游戏结束信息和最终得分
 *
 * 注意：此函数会频繁调用（每次游戏循环），应保持高效。
 */
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

/**
 * 根据方向获取对应的蛇身类型
 *
 * 功能：将Direction枚举值转换为对应的蛇身单元格类型（CellType）。
 * 此函数用于在蛇移动时将旧蛇头转换为对应方向的蛇身部分。
 *
 * @param dir 方向枚举值（DIR_UP/DIR_DOWN/DIR_LEFT/DIR_RIGHT）
 * @return CellType 对应的蛇身类型（CELL_SNAKE_BODY_*）
 */
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

/**
 * 更新游戏逻辑 - 新版本，只使用头尾位置
 *
 * 功能：更新游戏状态，包括蛇的移动、碰撞检测、食物检测和分数更新。
 * 此函数在每次游戏循环中调用，是实现游戏核心逻辑的关键函数。
 *
 * 实现步骤：
 *   1. 如果游戏已结束，直接返回
 *   2. 应用输入的方向缓冲（game.snake.next_direction）
 *   3. 根据当前方向计算新蛇头位置
 *   4. 检查碰撞（墙壁、蛇身体）
 *   5. 检查是否吃到食物
 *   6. 如果没吃到食物，移动蛇尾（清除旧蛇尾，找到新蛇尾）
 *   7. 如果吃到食物，增加长度、分数和速度，生成新食物
 *   8. 将旧蛇头变为蛇身，设置新蛇头位置
 *
 * 注意：此函数使用简化算法，只跟踪蛇头和蛇尾位置，通过游戏池单元格方向确定身体连接。
 */
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

/**
 * 处理用户输入
 *
 * 功能：检测和处理键盘输入，支持WASD键、方向键和退出键（ESC/Q）。
 * 此函数实现输入缓冲机制，防止蛇连续转向（如直接从上转向下）。
 *
 * 支持的输入：
 *   - 方向键：上(72)、下(80)、左(75)、右(77)
 *   - WASD键：W(上)、S(下)、A(左)、D(右)（不区分大小写）
 *   - 退出键：ESC(27)、Q（不区分大小写）
 *
 * 输入缓冲机制：
 *   只允许垂直于当前方向的新方向（防止蛇直接反向移动）
 *
 * @return bool 返回true表示继续游戏，false表示退出游戏
 */
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

/**
 * 根据当前位置的类型获取下一个位置
 *
 * 功能：根据指定位置的单元格类型，确定沿着蛇身方向的下一个位置。
 * 此函数主要用于在蛇移动时找到蛇尾的下一个位置（当蛇没有吃到食物时）。
 *
 * 处理逻辑：
 *   - CELL_SNAKE_HEAD: 根据当前蛇移动方向确定下一个位置
 *   - CELL_SNAKE_BODY_*: 根据身体部分的方向（向上/下/左/右）移动
 *   - CELL_SNAKE_TAIL: 检查四个方向，找到相邻的蛇身部分来确定移动方向
 *   - 其他类型: 位置保持不变
 *
 * @param pos 当前位置（游戏池坐标）
 * @return Position 沿着蛇身方向的下一个位置
 */
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

/**
 * 主函数
 *
 * 功能：程序的入口点，控制游戏的整体流程。
 * 此函数负责初始化游戏、显示开始界面、运行游戏主循环，并在游戏结束后显示最终画面。
 *
 * 程序流程：
 *   1. 调用init_game()初始化游戏状态和控制台
 *   2. 显示开始界面（标题和提示信息）
 *   3. 等待用户按任意键开始游戏
 *   4. 游戏主循环（处理输入、更新游戏状态、绘制界面）
 *   5. 显示最终画面（包括游戏结束信息）
 *   6. 等待用户按任意键退出程序
 *
 * 游戏主循环：
 *   while (!game.game_over && handle_input())
 *   {
 *       update_game();  // 更新游戏逻辑
 *       draw_game();    // 绘制界面
 *       Sleep(game.speed);  // 控制游戏速度
 *   }
 */
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
