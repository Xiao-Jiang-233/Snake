# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 项目概述

这是一个用C语言编写的控制台贪吃蛇游戏，使用CMake构建系统。项目针对Windows平台开发，使用了Windows控制台API (`conio.h`, `windows.h`)。

## 构建系统

项目使用CMake进行构建，支持多种配置预设（x64-debug、x64-release、x86-debug、x86-release）。

### 常用构建命令

```bash
# 配置项目（使用x64-debug预设）
cmake --preset x64-debug

# 构建项目
cmake --build --preset x64-debug

# 清理构建
cmake --build --preset x64-debug --target clean

# 运行游戏（构建后）
./out/build/x64-debug/Snake/Snake.exe
```

### 构建预设

- `x64-debug`: 64位调试版本
- `x64-release`: 64位发布版本
- `x86-debug`: 32位调试版本
- `x86-release`: 32位发布版本

构建输出目录：`out/build/${presetName}/Snake/Snake.exe`

## 项目结构

```
Snake/
├── CMakeLists.txt              # 根CMake配置文件
├── CMakePresets.json           # CMake预设配置
├── README.md                   # 项目说明
└── Snake/                      # 主源代码目录
    ├── CMakeLists.txt          # 可执行文件配置
    ├── Snake.h                 # 头文件（游戏数据结构、函数声明）
    ├── Snake.c                 # 源文件（游戏逻辑实现）
    └── Snake.exe               # 编译后的可执行文件（如果已构建）
```

### 代码架构

1. **数据结构**（定义在 `Snake.h`）：
   - `Direction`: 移动方向枚举（上、下、左、右）
   - `Position`: 二维坐标结构体
   - `Snake`: 蛇结构体，包含身体位置数组、长度和方向
   - `GameState`: 游戏状态结构体，包含蛇、食物、分数和游戏结束标志

2. **核心函数**：
   - `init_game()`: 初始化游戏状态
   - `generate_food()`: 随机生成食物位置
   - `draw_game()`: 绘制游戏界面到控制台
   - `update_game()`: 更新游戏逻辑（移动、碰撞检测、得分）
   - `handle_input()`: 处理键盘输入（WASD、方向键）
   - `clear_screen()`: 清空控制台屏幕（Windows API实现）

3. **主循环**（在 `main()` 函数中）：
   - 处理输入 → 更新游戏 → 绘制界面 → 延迟循环

## 开发注意事项

1. **平台依赖**：项目使用Windows特有的API（`conio.h`、`windows.h`、`Sleep()`、`_kbhit()`、`_getch()`），不直接支持其他操作系统。

2. **编码设置**：主函数中设置了控制台代码页为UTF-8（65001）以支持中文显示。

3. **游戏参数**：游戏区域大小在 `Snake.h` 中通过 `WIDTH` 和 `HEIGHT` 宏定义（默认20×20）。

4. **构建要求**：需要Windows平台、CMake 3.10+、MSVC编译器（cl.exe）或兼容的C编译器。

## 测试

项目目前没有包含自动化测试。游戏功能通过运行可执行文件进行手动测试。

## 扩展建议

如需添加新功能或修改现有代码：
1. 游戏逻辑集中在 `Snake.c` 的 `update_game()` 函数中
2. 渲染逻辑在 `draw_game()` 函数中
3. 输入处理在 `handle_input()` 函数中
4. 数据结构在 `Snake.h` 中定义