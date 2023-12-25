# Gomoku

电子科技大学 2023 秋高级程序设计挑战班实验项目

五子棋 AI

## 文件结构

- `src/main.cpp`

    主函数，主要游戏逻辑

- `src/board.hpp`
    
    五子棋棋局的基本表示

- `src/ui.hpp`:

    UI 绘制与交互

- `src/player/base.hpp`

    `Player` 基类定义

- `src/player/minimax.hpp`

    极小极大值算法

- `src/player/minimax/evaluator.hpp`

    估值函数

- `src/player/minimax/zobrist.hpp`

    Zobrist 哈希

- `src/player/minimax/mcts.hpp`

    蒙特卡洛树搜索（尚未完善）

## 构建要求

- CMake 3.26
- 支持 C++23 标准的较新编译器

## 依赖

- raylib 4.5.0
- Boost 1.83.0
- fmt 10.1.0

## TODO

- 代码模块化（C++20 Modules）

    截至目前（2023/12/25），CMake 已发布 3.28 版本，使用模块编写 C++ 代码已有较好的支持

    抛开还在 development branch 的 GCC 14 不谈，CMake 支持使用 Clang 16+ ~~与 MSVC~~ 扫描模块依赖并进行编译

    然而，libc++ 对标准库模块（`std` 和 `std.compat`）的支持仍处于实验性阶段，[配置较为复杂](https://libcxx.llvm.org/Modules.html#using-in-external-projects)

    退而求其次，在多个文件使用全局模块片段以包含标准库标头及外部库时，会遇到奇怪的 ODR 问题（MSYS2 clang64 工具链处理安全 CRT 相关的函数声明时尤为明显）

    通过在仅在单一模块文件中使用全局模块片段，再导出 using 声明看似能解决问题，但标准库中的重载运算符会无法使用（跨模块 ADL）

    综上，本项目仍然使用传统的 header-only 方式组织代码文件

- 完善蒙特卡洛树搜索算法的实现

- 改善 minimax 搜索的效率

- UI 优化
    - 支持选择玩家类型
    - 美化棋盘与棋子贴图
    - 添加音效
    - 改善玩家等待体验（`AsyncPlayer` 相关）

- 使用 Rust 重写

    ~~使用 C++ 是因为挑战班的语言要求~~

    主要原因是 Rust 中 `enum` 与 `match` 相关的特性可以很好的解决我在 C++ 中使用 `::std::variant` 的痛苦
