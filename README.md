# 仓库调度系统 | Warehouse Scheduling System

> **Language / 语言**: [🇨🇳 中文](#chinese) | [🇺🇸 English](#english)

---

## <a id="chinese"></a>🇨🇳 中文版本

### 项目概述

基于 SFML 和 C++ 开发的智能仓库调度仿真系统，用于模拟和可视化仓库设备的运行状态、车辆调度和任务执行过程。该系统提供了完整的前端 GUI 界面和后端仿真引擎，支持多种调度策略和实时数据分析。

### 🌟 核心功能

#### 仿真与可视化

- **实时 3D 轨道渲染** - 基于 SFML 的高性能轨道和设备系统可视化
- **动态车辆跟踪** - 实时显示车辆位置、速度、状态和任务信息
- **交互式控制面板** - 直观的用户界面控制仿真参数
- **多视角显示** - 支持平移、缩放和多角度观察

#### 调度与管理

- **智能任务调度** - 支持 TASK1、TASK2.1、TASK2.2、TASK2.3 多种调度模式
- **车辆状态管理** - 实时监控车辆运动状态、加速度和任务执行
- **设备状态监控** - 监控仓库接口设备的货物状态和运行情况
- **事件驱动系统** - 基于事件队列的精确时间管理

#### 数据分析

- **性能统计** - 任务完成率、设备利用率、车辆效率统计
- **日志记录** - 详细的操作日志和状态变化记录
- **数据导出** - 支持 CSV 格式的数据导出和分析

### 🏗️ 系统架构

```
Warehouse-sch/
├── 📁 src/                    # 源代码目录
│   ├── 📁 Core/              # 核心仿真引擎
│   │   ├── Scheduler.hpp     # 调度器核心
│   │   ├── Vehicle.hpp       # 车辆模型
│   │   ├── Task.hpp          # 任务管理
│   │   └── Device.hpp        # 设备管理
│   ├── 📁 gui/               # 图形用户界面
│   │   ├── MainWindow.hpp    # 主窗口
│   │   ├── SimulationView.hpp # 仿真视图
│   │   └── WarehouseRenderer.hpp # 仓库渲染器
│   └── 📁 Physics/           # 物理模拟
├── 📁 include/               # 头文件
├── 📁 test/                  # 测试用例
├── 📁 docs/                  # 项目文档
├── 📁 assets/                # 资源文件
└── 📁 bin/                   # 可执行文件
```

### 💻 系统要求

#### 硬件要求

- **CPU**: Intel Core i5-4000 系列或同等性能 AMD 处理器
- **内存**: 8GB RAM 或更高
- **显卡**: 支持 OpenGL 3.3 或更高版本的显卡
- **存储**: 至少 2GB 可用硬盘空间

#### 软件要求

- **操作系统**: Windows 10 (64 位) 或更高版本
- **运行时**: Visual C++ Redistributable 2019 或更高版本
- **开发环境** (源码构建):
  - Visual Studio 2022 或 MinGW-w64
  - CMake 3.20+
  - SFML 2.6.0+

### 🚀 快速开始

#### 方式一：直接运行

1. 从 [Releases](https://github.com/yourusername/Warehouse-sch/releases) 下载最新版本
2. 解压到任意目录
3. 双击运行 `WarehouseScheduler.exe`

#### 方式二：从源码构建

```bash
# 1. 克隆仓库
git clone https://github.com/yourusername/Warehouse-sch.git
cd Warehouse-sch

# 2. 安装依赖 (使用 vcpkg)
vcpkg install sfml:x64-windows

# 3. 构建项目
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=[vcpkg根目录]/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Release

# 4. 运行程序
./bin/WarehouseScheduler.exe
```

### 📖 使用指南

#### 基本操作

- **🎮 仿真控制**: 使用空格键开始/暂停仿真
- **👁️ 视图操作**:
  - `T` - 切换坐标网格显示
  - `G` - 切换仓库显示
  - `V` - 切换车辆显示
  - `C` - 切换调试信息
- **🚗 车辆控制**: 使用方向键手动控制车辆移动
- **📊 模式切换**: 通过工具栏切换不同的调度模式

#### 仿真模式

- **TASK1**: 基础 3 车辆随机目标调度
- **TASK2.1**: 3 车辆任务文件调度
- **TASK2.2**: 5 车辆复杂任务调度
- **TASK2.3**: 7 车辆高级调度优化

### 🔧 开发指南

#### 环境配置

```bash
# 安装必要的开发工具
# 1. Visual Studio 2022 Community (免费)
# 2. CMake (https://cmake.org/)
# 3. vcpkg 包管理器
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg && ./bootstrap-vcpkg.bat
```

#### 项目构建

```bash
# 配置构建环境
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=./vcpkg/scripts/buildsystems/vcpkg.cmake

# 编译项目
cmake --build build --config Release

# 运行测试
cd build && ctest
```

### 📚 文档资源

- 📋 [项目结构说明](docs/PROJECT_STRUCTURE.md)
- 🔍 [GUI 系统分析](docs/GUI_Program_Logic_Analysis.md)
- 🚀 [开发进度报告](docs/开发进度报告_0519_final.md)
- 🎯 [任务使用说明](TASK1_DEFAULT_MODE_USAGE.md)
- 📖 [完整 API 文档](docs/)

### 🤝 贡献指南

1. **Fork** 本仓库
2. **创建** 功能分支 (`git checkout -b feature/amazing-feature`)
3. **提交** 更改 (`git commit -m '添加某些功能'`)
4. **推送** 到分支 (`git push origin feature/amazing-feature`)
5. **创建** Pull Request

### 📄 许可证

本项目基于 MIT 许可证开源 - 查看 [LICENSE](LICENSE) 和 [LICENSE-zh.md](LICENSE-zh.md) 了解详情。

---

## <a id="english"></a>🇺🇸 English Version

### Project Overview

An intelligent warehouse scheduling simulation system developed with SFML and C++, designed to simulate and visualize warehouse equipment operations, vehicle scheduling, and task execution processes. The system provides a complete frontend GUI interface and backend simulation engine, supporting multiple scheduling strategies and real-time data analysis.

### 🌟 Core Features

#### Simulation & Visualization

- **Real-time 3D Track Rendering** - High-performance track and equipment system visualization based on SFML
- **Dynamic Vehicle Tracking** - Real-time display of vehicle position, speed, status, and task information
- **Interactive Control Panel** - Intuitive user interface for simulation parameter control
- **Multi-angle Display** - Support for panning, zooming, and multi-angle observation

#### Scheduling & Management

- **Intelligent Task Scheduling** - Support for TASK1, TASK2.1, TASK2.2, TASK2.3 multiple scheduling modes
- **Vehicle State Management** - Real-time monitoring of vehicle motion state, acceleration, and task execution
- **Device Status Monitoring** - Monitor warehouse interface device cargo status and operation
- **Event-driven System** - Precise time management based on event queues

#### Data Analysis

- **Performance Statistics** - Task completion rate, device utilization, vehicle efficiency statistics
- **Logging System** - Detailed operation logs and status change records
- **Data Export** - Support for CSV format data export and analysis

### 🏗️ System Architecture

```
Warehouse-sch/
├── 📁 src/                    # Source code directory
│   ├── 📁 Core/              # Core simulation engine
│   │   ├── Scheduler.hpp     # Scheduler core
│   │   ├── Vehicle.hpp       # Vehicle model
│   │   ├── Task.hpp          # Task management
│   │   └── Device.hpp        # Device management
│   ├── 📁 gui/               # Graphical user interface
│   │   ├── MainWindow.hpp    # Main window
│   │   ├── SimulationView.hpp # Simulation view
│   │   └── WarehouseRenderer.hpp # Warehouse renderer
│   └── 📁 Physics/           # Physics simulation
├── 📁 include/               # Header files
├── 📁 test/                  # Test cases
├── 📁 docs/                  # Project documentation
├── 📁 assets/                # Resource files
└── 📁 bin/                   # Executable files
```

### 💻 System Requirements

#### Hardware Requirements

- **CPU**: Intel Core i5-4000 series or equivalent AMD processor
- **Memory**: 8GB RAM or higher
- **Graphics**: Graphics card supporting OpenGL 3.3 or higher
- **Storage**: At least 2GB available disk space

#### Software Requirements

- **OS**: Windows 10 (64-bit) or higher
- **Runtime**: Visual C++ Redistributable 2019 or higher
- **Development Environment** (for source build):
  - Visual Studio 2022 or MinGW-w64
  - CMake 3.20+
  - SFML 2.6.0+

### 🚀 Quick Start

#### Method 1: Direct Execution

1. Download the latest version from [Releases](https://github.com/yourusername/Warehouse-sch/releases)
2. Extract to any directory
3. Double-click to run `WarehouseScheduler.exe`

#### Method 2: Build from Source

```bash
# 1. Clone repository
git clone https://github.com/yourusername/Warehouse-sch.git
cd Warehouse-sch

# 2. Install dependencies (using vcpkg)
vcpkg install sfml:x64-windows

# 3. Build project
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=[vcpkg_root]/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Release

# 4. Run program
./bin/WarehouseScheduler.exe
```

### 📖 User Guide

#### Basic Operations

- **🎮 Simulation Control**: Use spacebar to start/pause simulation
- **👁️ View Operations**:
  - `T` - Toggle coordinate grid display
  - `G` - Toggle warehouse display
  - `V` - Toggle vehicle display
  - `C` - Toggle debug information
- **🚗 Vehicle Control**: Use arrow keys for manual vehicle control
- **📊 Mode Switching**: Switch different scheduling modes via toolbar

#### Simulation Modes

- **TASK1**: Basic 3-vehicle random target scheduling
- **TASK2.1**: 3-vehicle task file scheduling
- **TASK2.2**: 5-vehicle complex task scheduling
- **TASK2.3**: 7-vehicle advanced scheduling optimization

### 🔧 Development Guide

#### Environment Setup

```bash
# Install necessary development tools
# 1. Visual Studio 2022 Community (free)
# 2. CMake (https://cmake.org/)
# 3. vcpkg package manager
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg && ./bootstrap-vcpkg.bat
```

#### Project Build

```bash
# Configure build environment
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=./vcpkg/scripts/buildsystems/vcpkg.cmake

# Compile project
cmake --build build --config Release

# Run tests
cd build && ctest
```

### 📚 Documentation Resources

- 📋 [Project Structure](docs/PROJECT_STRUCTURE.md)
- 🔍 [GUI System Analysis](docs/GUI_Program_Logic_Analysis.md)
- 🚀 [Development Progress Report](docs/开发进度报告_0519_final.md)
- 🎯 [Task Usage Guide](TASK1_DEFAULT_MODE_USAGE.md)
- 📖 [Complete API Documentation](docs/)

### 🤝 Contributing

1. **Fork** this repository
2. **Create** a feature branch (`git checkout -b feature/amazing-feature`)
3. **Commit** your changes (`git commit -m 'Add some amazing feature'`)
4. **Push** to the branch (`git push origin feature/amazing-feature`)
5. **Create** a Pull Request

### 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

### 📄 许可证

本项目采用 MIT 许可证 - 详见 [LICENSE-zh.md](LICENSE-zh.md) 文件（中文版）或 [LICENSE](LICENSE) 文件（英文版）

## 联系方式

- 邮箱：support@example.com
- 问题反馈：https://github.com/yourusername/Warehouse-sch/issues
- 文档更新：https://github.com/yourusername/Warehouse-sch/wiki

## 致谢

- [SFML](https://www.sfml-dev.org/)
- [vcpkg](https://github.com/microsoft/vcpkg)
- [CMake](https://cmake.org/)
- 所有贡献者
