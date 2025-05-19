cmake_minimum_required(VERSION 3.14)
project(WarehouseSimulation)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找SFML库
find_package(SFML 2.6 COMPONENTS system window graphics REQUIRED)

# 添加可执行文件
add_executable(WarehouseSimulation
    main.cpp
    MainWindow.cpp
    SimulationView.cpp
    TrackRenderer_fixed.cpp
    VehicleRenderer.cpp
    WarehouseRenderer.cpp
)

# 包含头文件目录
target_include_directories(WarehouseSimulation PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# 链接SFML库
target_link_libraries(WarehouseSimulation PRIVATE
    sfml-system
    sfml-window
    sfml-graphics
)

# 设置输出目录
set_target_properties(WarehouseSimulation PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 如果是Windows，设置一些特定配置
if(WIN32)
    # 复制SFML DLL到输出目录
    add_custom_command(TARGET WarehouseSimulation POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:sfml-system>
        $<TARGET_FILE:sfml-window>
        $<TARGET_FILE:sfml-graphics>
        $<TARGET_FILE_DIR:WarehouseSimulation>
    )
endif()

# 添加资源目录（如果有）
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/resources")
    add_custom_command(TARGET WarehouseSimulation POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/resources
        $<TARGET_FILE_DIR:WarehouseSimulation>/resources
    )
endif()

# 设置安装规则
install(TARGETS WarehouseSimulation DESTINATION bin)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/resources")
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/resources DESTINATION bin)
endif()
