cmake_minimum_required(VERSION 3.10)
project(FontTest VERSION 1.0)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找SFML包
find_package(SFML 2.6 COMPONENTS graphics window system REQUIRED)

# 创建可执行文件
add_executable(FontTest FontTest.cpp)

# 包含目录
target_include_directories(FontTest PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)

# 链接SFML库
target_link_libraries(FontTest PRIVATE sfml-graphics sfml-window sfml-system)

# 资源文件复制
add_custom_command(TARGET FontTest POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   ${CMAKE_SOURCE_DIR}/../../assets $<TARGET_FILE_DIR:FontTest>/assets)

# 复制SFML DLL文件
add_custom_command(TARGET FontTest POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different
                   ${SFML_GRAPHICS_DLL} $<TARGET_FILE_DIR:FontTest>)
add_custom_command(TARGET FontTest POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different
                   ${SFML_WINDOW_DLL} $<TARGET_FILE_DIR:FontTest>)
add_custom_command(TARGET FontTest POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different
                   ${SFML_SYSTEM_DLL} $<TARGET_FILE_DIR:FontTest>)

# 设置输出目录
set_target_properties(FontTest PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 编译选项
if(MSVC)
    target_compile_options(FontTest PRIVATE /W4)
else()
    target_compile_options(FontTest PRIVATE -Wall -Wextra)
endif() 