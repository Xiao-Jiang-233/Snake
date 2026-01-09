# CLAUDE.md

本文件为 Claude Code（claude.ai/code）提供在此代码库中工作的指导。

## 项目概述

这是一个使用 C 语言编写的 Windows 控制台贪吃蛇游戏。项目使用 CMake 构建系统，支持 x64/x86 的 Debug/Release 构建配置。

## 游戏玩法

### 控制方式

- **方向键**：上(↑)、下(↓)、左(←)、右(→)控制蛇移动方向
- **WASD 键**：W(上)、S(下)、A(左)、D(右)控制蛇移动方向
- **退出游戏**：Q 键或 ESC 键
- **转向限制**：蛇不能直接反向移动（例如：正在向右移动时不能立即向左转）

### 游戏规则

- **得分机制**：每吃到一个食物得 10 分
- **速度变化**：每得 50 分游戏速度增加（移动间隔减少 10 毫秒，最低 30 毫秒）
- **游戏结束条件**：
  1. 蛇撞到墙壁（游戏池边界）
  2. 蛇撞到自己的身体
  3. 游戏胜利：当蛇填满整个游戏区域时（所有单元格都被蛇占据）
- **蛇的成长**：每吃到一个食物，蛇长度增加 1，蛇尾不移动

### 游戏界面布局

- **游戏区域**：20×20 可玩区域，四周有墙壁边界
- **控制台尺寸**：80 列 ×30 行缓冲区
- **游戏区域位置**：从控制台第 8 列第 4 行开始显示
- **信息显示**：右侧显示得分、速度、游戏状态等信息

## 常用命令

### 构建项目

```bash
# 使用CMake预设配置构建（推荐）
cmake --preset x64-debug
cmake --build out/build/x64-debug

# 或使用传统方式
mkdir build && cd build
cmake ..
cmake --build .
```

### 运行游戏

```bash
# 构建后可执行文件位于
./out/build/x64-debug/Snake/Snake.exe

# 直接运行
./Snake/Snake.exe
```

### 清理构建

```bash
# 删除构建目录
rm -rf out/build
# 或
cmake --build out/build/x64-debug --target clean
```

## 构建系统

### CMake 预设配置

项目配置了 4 种预设构建配置：

- `x64-debug`: 64 位 Debug 版本
- `x64-release`: 64 位 Release 版本
- `x86-debug`: 32 位 Debug 版本
- `x86-release`: 32 位 Release 版本

使用`cmake --list-presets`查看所有可用预设。

### 编译器设置

- 编译器：MSVC (cl.exe)
- C 标准：C11
- 编码：UTF-8 支持已启用
- 生成器：Ninja

## 代码架构

### 文件结构

```text
Snake/
├── CMakeLists.txt          # 顶层CMake配置
├── CMakePresets.json       # CMake构建预设
├── Snake/                  # 源代码目录
│   ├── CMakeLists.txt      # 子项目CMake配置
│   └── Snake.c             # 主源代码文件（所有游戏逻辑）
└── out/                    # 构建输出目录
```

### 主要代码模块

所有游戏逻辑集中在单个文件 `Snake/Snake.c` 中，包含以下模块：

1. **控制台操作模块** (`init_console`, `set_cursor_position`, `printf_at`, `clear_screen`)

   - 使用 Windows API 进行控制台图形显示
   - 支持 UTF-8 编码和中文显示
   - 提供定位输出和颜色控制功能

2. **游戏池管理模块** (`pool_to_console`, `get_cell_console_position`, `draw_cell`)

   - 管理游戏网格（20×20 可玩区域+边框）
   - 处理坐标转换（游戏池坐标 ↔ 控制台坐标）

3. **游戏逻辑模块** (`init_game`, `update_game`, `handle_input`, `draw_game`)

   - 游戏状态初始化和管理
   - 蛇移动和碰撞检测
   - 食物生成和计分
   - 用户输入处理（WASD/方向键控制）

4. **辅助功能模块** (`get_next_position`, 各种工具函数)
   - 方向计算和位置更新
   - 游戏速度控制

### 关键数据结构

- `GameState`: 游戏状态（分数、速度、游戏结束标志）
- `Position`: 二维坐标位置
- `Direction`: 方向枚举（上、下、左、右）
- `CellType`: 单元格类型（空、食物、蛇头、蛇身、蛇尾、墙壁）
- `Snake`: 蛇的状态（头尾位置、长度、方向）

### 全局变量

- `pool[POOL_HEIGHT][POOL_WIDTH]`: 游戏池网格
- `game`: 当前游戏状态
- `hConsole`: Windows 控制台句柄
- `console_width`, `console_height`: 控制台尺寸

## 平台依赖

### Windows 专用 API

项目使用以下 Windows 特有头文件：

- `<windows.h>`: Windows API
- `<conio.h>`: 控制台输入输出
- 其他标准 C 库头文件

### Unicode 支持

- 使用宽字符（`wchar_t`）和宽字符串（`L"..."`）
- 设置控制台代码页为 UTF-8
- 配置区域设置为`zh_CN.UTF-8`

## 开发注意事项

1. **平台限制**: 代码仅适用于 Windows 平台，使用 Windows API 和 MSVC 编译器。
2. **单一文件架构**: 所有代码在一个文件中，便于理解和修改。
3. **控制台图形**: 使用控制台 API 实现简单的图形界面，非 GUI 应用。
4. **构建配置**: 使用 CMake 预设简化构建过程，支持多种架构和配置。
5. **编码规范**: 代码使用中文注释和变量名，注意 UTF-8 编码一致性。

## 调试与开发技巧

### 游戏速度调整

- 初始速度：120 毫秒移动间隔
- 速度变量：`game.speed`（毫秒值，越小越快）
- 调整位置：`init_game()`函数中修改初始值，或`update_game()`中修改速度变化逻辑

### 游戏区域调整

- 修改`GAME_WIDTH`和`GAME_HEIGHT`常量调整游戏区域大小
- 修改`GAME_AREA_X`和`GAME_AREA_Y`常量调整游戏区域在控制台中的位置

### 编译调试

- Debug 版本包含调试信息，便于使用 Visual Studio 调试器
- 可使用`printf_at()`函数在控制台特定位置输出调试信息
- 注意宽字符字符串使用`L"..."`格式

## 扩展建议

如需扩展项目，可考虑：

1. 拆分头文件和源文件以提高模块性
2. 添加跨平台支持（如使用 ncurses 等库）
3. 实现游戏难度级别和更多游戏模式
4. 添加单元测试框架
5. 改进图形显示（如使用更丰富的字符图形）
6. 添加游戏暂停功能
7. 实现最高分记录系统
8. 添加声音效果（Windows API 的 Beep 函数）
