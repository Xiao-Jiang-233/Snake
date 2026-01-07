# CMakeList.txt: Snake 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#

# 将源代码添加到此项目的可执行文件。
add_executable (Snake "Snake.c")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Snake PROPERTY C_STANDARD 11)
endif()

# 设置编译器编码选项以确保UTF-8支持
if (MSVC)
  target_compile_options(Snake PRIVATE /utf-8)
else()
  # GCC/Clang 编码选项
  target_compile_options(Snake PRIVATE -finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

# TODO: 如有需要，请添加测试并安装目标。
