cmake_minimum_required(VERSION 3.15)
project(WarehouseSimulation VERSION 0.1.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 包含SFML
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

# 设置TGUI路径
set(TGUI_DIR "${CMAKE_SOURCE_DIR}/third_party/TGUI")
set(TGUI_BUILD_DIR "${TGUI_DIR}/build")

# 包含TGUI
option(TGUI_BUILD_GUI_BUILDER "Build the GUI builder" OFF)
add_subdirectory(${TGUI_DIR} ${TGUI_BUILD_DIR})

# 包含头文件目录
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${TGUI_DIR}/include
)

# 示例程序
add_executable(tgui_example
    src/TGUIWrapper.hpp
    src/GUI/UI/TGUIExample.cpp
)

# 链接库
target_link_libraries(tgui_example
    sfml-graphics
    sfml-window
    sfml-system
    tgui
)

# 设置编译选项
if(MSVC)
    target_compile_options(tgui_example PRIVATE /W4)
else()
    target_compile_options(tgui_example PRIVATE -Wall -Wextra -pedantic)
endif()

# 安装指令
install(TARGETS tgui_example
    RUNTIME DESTINATION bin
)

# 复制需要的资源文件
file(COPY ${TGUI_DIR}/themes DESTINATION ${CMAKE_BINARY_DIR})

# 添加测试
enable_testing()
add_test(NAME tgui_example_test COMMAND tgui_example) 