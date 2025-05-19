# 物流仓库仿真系统 GUI 模块实现

## 项目概述

物流仓库仿真系统是一个基于 SFML 和 C++的 3D 图形仿真界面，用于模拟展示物流小车在轨道上的运动、仓库设备状态以及物流调度过程。项目分为前端 GUI 和后端仿真引擎两部分，本代码库主要包含前端 GUI 部分的实现。

## 主要功能

- **3D 仿真场景**：使用"伪 3D"技术实现立体视觉效果，包括轨道、车辆和设备的渲染
- **实时状态显示**：展示车辆位置、速度、载货状态以及设备工作状态
- **交互控制**：支持视图缩放、平移、对象选择等交互操作
- **仿真控制**：提供启动/暂停、调整仿真速度等控制功能
- **前后端通信**：通过定义的接口与后端仿真引擎进行数据交换

## 系统架构

项目采用模块化设计，主要组件包括：

- **MainWindow**：主窗口类，负责整体布局和事件处理
- **SimulationView**：仿真视图类，负责 3D 场景的渲染
- **StatusPanel**：状态面板类，显示系统状态和详细信息
- **Toolbar**：工具栏类，提供用户交互控制
- **渲染器组件**：包括 TrackRenderer、VehicleRenderer 和 WarehouseRenderer 等
- **SimulationInterface**：定义前后端通信接口

## 已完成的改进

1. **代码整理与修复**：

   - 修复了 VehicleState 和 DeviceState 的结构定义不一致问题
   - 创建了更完善的测试程序和测试文档
   - 添加了详细的使用文档和开发记录

2. **测试系统完善**：

   - 创建了 GUITest_Fixed.cpp 全功能测试程序
   - 添加了测试构建脚本 build_test.ps1
   - 编写了详细的 TEST_DOCUMENT.md 测试文档

3. **文档系统更新**：

   - 添加了完整的使用文档.md
   - 创建了开发记录\_更新\_0519.md
   - 更新了项目 README.md

## 如何使用

### 系统要求

- Windows 10/11 或兼容系统
- SFML 2.6.2 库
- C++17 兼容编译器（如 g++或 MSVC）

### 编译和运行

#### 使用 PowerShell 脚本编译

```powershell
# 切换到项目目录
cd d:\CodeMaster\GitHub\C++SFML\Warehouse-sch\GUI

# 使用build.ps1脚本编译
.\build.ps1

# 运行
.\build\WarehouseGUI.exe
```

#### 使用 VS Code 任务编译

1. 打开 VS Code
2. 打开主文件 main.cpp
3. 按下 `Ctrl+Shift+B` 运行构建任务
4. 选择 "C/C++: g++.exe 生成活动文件" 任务

### 运行测试程序

```powershell
# 切换到测试目录
cd d:\CodeMaster\GitHub\C++SFML\Warehouse-sch\GUI\Test

# 使用测试构建脚本
.\build_test.ps1
```

## 项目结构

```
GUI/
├── main.cpp                 # 程序入口
├── MainWindow.hpp/cpp       # 主窗口类
├── SimulationView.hpp/cpp   # 仿真视图类
├── StatusPanel.hpp/cpp      # 状态面板类
├── Toolbar.hpp/cpp          # 工具栏类
├── SimulationInterface.hpp  # 仿真接口定义
├── MockSimulationInterface.hpp # 模拟仿真接口实现
├── TrackRenderer.hpp/cpp    # 轨道渲染器
├── VehicleRenderer.hpp/cpp  # 车辆渲染器
├── WarehouseRenderer.hpp/cpp # 设备/仓库渲染器
├── SimObject.hpp            # 仿真对象基类
├── DeviceState.hpp          # 设备状态定义
├── config.json              # 配置文件
├── build.ps1                # 构建脚本
├── CMakeLists.txt           # CMake配置文件
├── resources/               # 资源文件夹
│   ├── fonts/               # 字体文件
│   └── icons/               # 图标文件
├── Test/                    # 测试文件夹
│   ├── GUITest_Fixed.cpp    # GUI测试程序
│   ├── build_test.ps1       # 测试构建脚本
│   └── TEST_DOCUMENT.md     # 测试文档
├── legacy/                  # 旧代码存放文件夹
└── docs/                    # 文档文件夹
    ├── 编译说明.md          # 编译说明
    ├── 使用文档.md          # 使用指南
    ├── 功能说明文档.md       # 功能说明
    ├── 开发记录_更新_0519.md # 开发记录
    └── GUI开发说明/         # 开发说明文件夹
```

## 文档索引

项目包含以下主要文档：

1. [使用文档](./docs/使用文档.md)：详细的系统使用指南
2. [测试文档](./Test/TEST_DOCUMENT.md)：测试点和测试方法说明
3. [功能说明文档](./docs/功能说明文档.md)：系统功能概述
4. [开发记录](./docs/开发记录_更新_0519.md)：开发历程和问题解决方案

## 已知问题

1. VehicleState 和 DeviceState 的定义不一致，在 MockSimulationInterface 中使用了未在结构体中定义的字段
2. 目前测试程序使用自定义版本的结构体，实际项目代码需要统一数据结构

## 后续改进计划

1. 统一数据结构定义，确保整个项目使用一致的结构体
2. 完善单元测试系统，提高代码覆盖率
3. 优化渲染性能，支持更多车辆和设备的同时显示
4. 改进 UI 交互体验，添加更多可视化效果

本项目通过完善测试和文档系统，改进了 GUI 模块的可维护性和可测试性，为持续开发和改进奠定了基础。
