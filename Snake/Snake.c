// Snake.c: C语言版本的主程序

#include "Snake.h"
#include <stdio.h>

#include <conio.h>
#include <windows.h>

// 初始化游戏
void init_game(GameState *game)
{
    // 初始化随机种子
    srand((unsigned int)time(NULL));

    // 初始化蛇
    game->snake.length = 3;
    game->snake.direction = DIR_RIGHT;

    // 蛇初始位置在中间偏左
    int start_x = WIDTH / 4;
    int start_y = HEIGHT / 2;

    for (int i = 0; i < game->snake.length; i++)
    {
        game->snake.body[i].x = start_x - i;
        game->snake.body[i].y = start_y;
    }

    // 初始化食物
    generate_food(game);

    // 初始化分数和游戏状态
    game->score = 0;
    game->game_over = false;
}

// 生成食物
void generate_food(GameState *game)
{
    bool valid_position;

    do
    {
        valid_position = true;
        game->food.x = rand() % WIDTH;
        game->food.y = rand() % HEIGHT;

        // 检查食物是否与蛇身重叠
        for (int i = 0; i < game->snake.length; i++)
        {
            if (game->food.x == game->snake.body[i].x &&
                game->food.y == game->snake.body[i].y)
            {
                valid_position = false;
                break;
            }
        }
    } while (!valid_position);
}

// 清屏函数
void clear_screen()
{
    system("cls");
}

// 绘制游戏界面
void draw_game(const GameState *game)
{
    clear_screen();

    // 绘制上边框
    for (int i = 0; i < WIDTH + 2; i++)
    {
        printf("#");
    }
    printf("\n");

    // 绘制游戏区域
    for (int y = 0; y < HEIGHT; y++)
    {
        printf("#"); // 左边框

        for (int x = 0; x < WIDTH; x++)
        {
            bool drawn = false;

            // 绘制蛇头
            if (x == game->snake.body[0].x && y == game->snake.body[0].y)
            {
                printf("@");
                drawn = true;
            }
            // 绘制蛇身
            else
            {
                for (int i = 1; i < game->snake.length; i++)
                {
                    if (x == game->snake.body[i].x && y == game->snake.body[i].y)
                    {
                        printf("o");
                        drawn = true;
                        break;
                    }
                }
            }

            // 绘制食物
            if (!drawn && x == game->food.x && y == game->food.y)
            {
                printf("*");
                drawn = true;
            }

            // 绘制空白
            if (!drawn)
            {
                printf(" ");
            }
        }

        printf("#\n"); // 右边框
    }

    // 绘制下边框
    for (int i = 0; i < WIDTH + 2; i++)
    {
        printf("#");
    }
    printf("\n");

    // 显示分数和提示
    printf("分数: %d\n", game->score);
    printf("使用 WASD 或方向键控制移动 (按 Q 退出)\n");

    if (game->game_over)
    {
        printf("游戏结束! 按 R 重新开始\n");
    }
}

// 更新游戏状态
void update_game(GameState *game)
{
    if (game->game_over)
    {
        return;
    }

    // 保存蛇尾位置（用于可能的增长）
    Position old_tail = game->snake.body[game->snake.length - 1];

    // 移动蛇身（从尾部向前移动）
    for (int i = game->snake.length - 1; i > 0; i--)
    {
        game->snake.body[i] = game->snake.body[i - 1];
    }

    // 移动蛇头
    Position new_head = game->snake.body[0];

    switch (game->snake.direction)
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

    // 检查边界碰撞
    if (new_head.x < 0 || new_head.x >= WIDTH ||
        new_head.y < 0 || new_head.y >= HEIGHT)
    {
        game->game_over = true;
        return;
    }

    // 检查自我碰撞
    for (int i = 1; i < game->snake.length; i++)
    {
        if (new_head.x == game->snake.body[i].x &&
            new_head.y == game->snake.body[i].y)
        {
            game->game_over = true;
            return;
        }
    }

    // 更新蛇头位置
    game->snake.body[0] = new_head;

    // 检查是否吃到食物
    if (new_head.x == game->food.x && new_head.y == game->food.y)
    {
        // 增加蛇长度
        if (game->snake.length < WIDTH * HEIGHT)
        {
            game->snake.body[game->snake.length] = old_tail;
            game->snake.length++;
        }

        // 增加分数
        game->score += 10;

        // 生成新食物
        generate_food(game);
    }
}

// 处理输入
bool handle_input(GameState *game)
{
    if (_kbhit())
    {
        int ch = _getch();

        // 处理方向键（Windows下方向键是两个字节）
        if (ch == 0 || ch == 224)
        {
            ch = _getch();
            switch (ch)
            {
            case 72: // 上箭头
                if (game->snake.direction != DIR_DOWN)
                    game->snake.direction = DIR_UP;
                break;
            case 80: // 下箭头
                if (game->snake.direction != DIR_UP)
                    game->snake.direction = DIR_DOWN;
                break;
            case 75: // 左箭头
                if (game->snake.direction != DIR_RIGHT)
                    game->snake.direction = DIR_LEFT;
                break;
            case 77: // 右箭头
                if (game->snake.direction != DIR_LEFT)
                    game->snake.direction = DIR_RIGHT;
                break;
            }
        }
        else
        {
            // 处理WASD和Q键
            switch (ch)
            {
            case 'w':
            case 'W':
                if (game->snake.direction != DIR_DOWN)
                    game->snake.direction = DIR_UP;
                break;
            case 's':
            case 'S':
                if (game->snake.direction != DIR_UP)
                    game->snake.direction = DIR_DOWN;
                break;
            case 'a':
            case 'A':
                if (game->snake.direction != DIR_RIGHT)
                    game->snake.direction = DIR_LEFT;
                break;
            case 'd':
            case 'D':
                if (game->snake.direction != DIR_LEFT)
                    game->snake.direction = DIR_RIGHT;
                break;
            case 'q':
            case 'Q':
                return false; // 退出游戏
            case 'r':
            case 'R':
                if (game->game_over)
                {
                    init_game(game); // 重新开始游戏
                }
                break;
            }
        }
    }

    return true; // 继续游戏
}

// 主函数
int main()
{
    GameState game;

    // 隐藏光标
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);

    // 初始化游戏
    init_game(&game);

    printf("贪吃蛇游戏 - 控制台字符界面\n");
    printf("按任意键开始游戏...\n");
    _getch();

    // 游戏主循环
    while (handle_input(&game))
    {
        update_game(&game);
        draw_game(&game);

        // 控制游戏速度
        Sleep(120); // 120毫秒
    }

    printf("游戏结束，谢谢游玩！\n");

    return 0;
}
