# Warehouse-sch 项目结构说明

本文档描述了 Warehouse-sch 项目的文件结构组织情况。

## 主要目录

- `Core/`: 核心仿真引擎和业务逻辑

  - `Device.hpp/cpp`: 设备模型实现
  - `SimulationEngine.hpp/cpp`: 仿真引擎实现
  - `Task.hpp`: 任务模型定义
  - `TaskQueue.hpp`: 任务队列管理
  - `Vehicle.hpp/cpp`: 车辆模型实现

- `GUI/`: 前端图形界面实现

  - `main.cpp`: 主程序入口点
  - `main_simplified.cpp`: 简化版入口点
  - `MainWindow.hpp/cpp`: 主窗口实现
  - `SimulationView.hpp/cpp`: 仿真场景视图
  - 渲染器组件:
    - `TrackRenderer.hpp/cpp`: 轨道渲染器
    - `VehicleRenderer.hpp/cpp`: 车辆渲染器
    - `WarehouseRenderer.hpp/cpp`: 仓库渲染器
    - `DeviceRenderer.hpp/cpp`: 设备渲染器
  - 接口定义:
    - `SimObject.hpp`: 基础模拟对象
    - `SimulationInterface.hpp`: 前后端接口定义
    - `MockSimulationInterface.hpp`: 模拟接口实现
    - `DeviceState.hpp`: 设备状态定义
  - UI 组件:
    - `StatusPanel.hpp/cpp`: 状态面板
    - `Toolbar.hpp/cpp`: 工具栏
    - `UIControls.hpp/cpp`: UI 控件库
    - `TaskList.hpp/cpp`: 任务列表
  - `docs/`: 文档目录
  - `legacy/`: 不再使用的旧版文件
  - `resources/`: 资源文件
  - `Test/`: 测试文件

- `Physics/`: 物理模拟组件

  - `MotionController.hpp/cpp`: 运动控制器
  - `TrackSegment.hpp`: 轨道段定义

- `ToDO/`: 任务清单和规划文档

- `legacy/`: 过时且不再使用的文件和目录

## 文件组织说明

1. 核心功能位于 `Core/` 和 `Physics/` 目录
2. 图形界面相关实现位于 `GUI/` 目录
3. 所有不再使用的文件都已移至 `legacy/` 目录或 `GUI/legacy/` 子目录
4. 文档和计划位于 `docs/` 和 `ToDO/` 目录

## 构建系统

项目使用 CMake 构建系统，主要构建文件为 `GUI/CMakeLists.txt`。构建命令：

```bash
cd GUI
./build.ps1
```

或者手动构建：

```bash
cd GUI
cmake -S . -B build
cmake --build build
```

## 历史文件

所有不再使用的文件都已移动到 legacy 目录中，包括：

- 旧版 homework 目录下的实现
- 旧版和实验性的轨道渲染器
- 临时功能实现
