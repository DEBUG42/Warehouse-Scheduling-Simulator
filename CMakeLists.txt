# filepath: d:\CodeMaster\GitHub\Warehouse-sch\GUI\CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(WarehouseSimulation VERSION 1.0)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找SFML包
find_package(SFML 2.6 COMPONENTS graphics window system REQUIRED)

# 设置源文件
set(SOURCES
    main.cpp
    MainWindow.cpp
    SimulationView.cpp
    TrackRenderer.cpp
    VehicleRenderer.cpp
    WarehouseRenderer.cpp
    DeviceRenderer.cpp
    StatusPanel.cpp
    Toolbar.cpp
    UIControls.cpp
    TaskList.cpp
)

# 设置头文件
set(HEADERS
    MainWindow.hpp
    SimulationView.hpp
    TrackRenderer.hpp
    VehicleRenderer.hpp
    WarehouseRenderer.hpp
    DeviceRenderer.hpp
    DeviceState.hpp
    WarehouseUtils.hpp
    StatusPanel.hpp
    Toolbar.hpp
    UIControls.hpp
    SimObject.hpp
    SimulationInterface.hpp
    MockSimulationInterface.hpp
    TestSimulationEngine.hpp
    ConfigReader.hpp
    TaskList.hpp
)

# 创建可执行文件

# 创建主可执行文件
add_executable(WarehouseSimulation ${SOURCES} ${HEADERS})

# 创建简化版可执行文件（使用main_simplified.cpp）
add_executable(WarehouseSimulationSimple main_simplified.cpp ${HEADERS} 
    SimulationView.cpp
    TrackRenderer.cpp
    VehicleRenderer.cpp
    WarehouseRenderer.cpp
    DeviceRenderer.cpp
)

# 包含目录
target_include_directories(WarehouseSimulation PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../Core
)
target_include_directories(WarehouseSimulationSimple PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../Core
)

# 链接SFML库
target_link_libraries(WarehouseSimulation PRIVATE sfml-graphics sfml-window sfml-system)
target_link_libraries(WarehouseSimulationSimple PRIVATE sfml-graphics sfml-window sfml-system)

# 资源文件复制
add_custom_command(TARGET WarehouseSimulation POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:WarehouseSimulation>/assets)

add_custom_command(TARGET WarehouseSimulationSimple POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:WarehouseSimulationSimple>/assets)

# 设置输出目录
set_target_properties(WarehouseSimulation PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

set_target_properties(WarehouseSimulationSimple PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 编译选项
if(MSVC)
    target_compile_options(WarehouseSimulation PRIVATE /W4)
    target_compile_options(WarehouseSimulationSimple PRIVATE /W4)
else()
    target_compile_options(WarehouseSimulation PRIVATE -Wall -Wextra)
    target_compile_options(WarehouseSimulationSimple PRIVATE -Wall -Wextra)
endif()

# 打印信息
message(STATUS "SFML版本: ${SFML_VERSION}")
message(STATUS "C++标准: ${CMAKE_CXX_STANDARD}")
message(STATUS "编译器: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
 