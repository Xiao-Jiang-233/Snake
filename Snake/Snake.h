// Snake.h: C语言版本的头文件

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

// TODO: 在此处引用程序需要的其他标头。

// 游戏区域大小
#define WIDTH 20
#define HEIGHT 20

// 方向枚举
typedef enum
{
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
} Direction;

// 位置结构体
typedef struct
{
    int x;
    int y;
} Position;

// 蛇结构体
typedef struct
{
    Position body[WIDTH * HEIGHT]; // 蛇的身体位置数组
    int length;                    // 蛇的长度
    Direction direction;           // 当前移动方向
} Snake;

// 游戏状态结构体
typedef struct
{
    Snake snake;
    Position food;
    int score;
    bool game_over;
} GameState;

// 函数声明
void init_game(GameState *game);
void generate_food(GameState *game);
void draw_game(const GameState *game);
void update_game(GameState *game);
bool handle_input(GameState *game);
void clear_screen();
