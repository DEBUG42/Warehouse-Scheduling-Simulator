# 仓库调度系统 GUI 项目结构说明

## 项目目录结构

```
Warehouse-sch/
├── src/                    # 源代码文件目录
│   ├── gui/               # GUI相关实现
│   │   ├── main.cpp      # 主程序入口
│   │   ├── MainWindow.cpp # 主窗口实现
│   │   ├── SimulationView.cpp # 模拟视图实现
│   │   ├── DeviceRenderer.cpp # 设备渲染器实现
│   │   ├── TaskList.cpp  # 任务列表实现
│   │   ├── UIControls.cpp # UI控件实现
│   │   ├── Toolbar.cpp   # 工具栏实现
│   │   ├── StatusPanel.cpp # 状态面板实现
│   │   ├── TrackRenderer.cpp # 轨道渲染器实现
│   │   ├── VehicleRenderer.cpp # 车辆渲染器实现
│   │   ├── WarehouseRenderer.cpp # 仓库渲染器实现
│   │   └── IconGenerator.cpp # 图标生成器实现
│   ├── core/             # 核心功能实现
│   └── utils/            # 工具类实现
│
├── include/               # 头文件目录
│   ├── gui/              # GUI相关头文件
│   │   ├── MainWindow.hpp # 主窗口声明
│   │   ├── SimulationView.hpp # 模拟视图声明
│   │   ├── DeviceRenderer.hpp # 设备渲染器声明
│   │   ├── TaskList.hpp  # 任务列表声明
│   │   ├── UIControls.hpp # UI控件声明
│   │   ├── Toolbar.hpp   # 工具栏声明
│   │   ├── StatusPanel.hpp # 状态面板声明
│   │   ├── TrackRenderer.hpp # 轨道渲染器声明
│   │   ├── VehicleRenderer.hpp # 车辆渲染器声明
│   │   ├── WarehouseRenderer.hpp # 仓库渲染器声明
│   │   ├── SimObject.hpp # 模拟对象基类
│   │   ├── DeviceState.hpp # 设备状态定义
│   │   ├── WarehouseState.hpp # 仓库状态定义
│   │   └── WarehouseUtils.hpp # 工具函数声明
│   ├── core/             # 核心功能头文件
│   └── utils/            # 工具类头文件
│
├── assets/               # 资源文件目录
│   ├── fonts/           # 字体文件
│   ├── images/          # 图片资源
│   └── icons/           # 图标资源
│
├── docs/                 # 文档目录
│   ├── 项目结构说明.md    # 本文档
│   ├── 开发记录.md       # 开发日志
│   ├── PROJECT_STRUCTURE.md # 项目结构说明（旧版）
│   └── ToDO.md          # 待办事项
│
├── build/                # 构建脚本目录
│   └── build.ps1        # PowerShell构建脚本
│
├── third_party/          # 第三方库目录
│
├── .vscode/             # VSCode配置目录
│
├── CMakeLists.txt        # CMake配置文件
├── README.md            # 项目说明文档
└── .gitignore           # Git忽略文件
```

## 目录说明

### src/

存放所有 C++源文件(.cpp)，按功能模块分类：

- gui/: GUI 相关实现
- core/: 核心功能实现
- utils/: 工具类实现

### include/

存放所有 C++头文件(.hpp)，按功能模块分类：

- gui/: GUI 相关头文件
- core/: 核心功能头文件
- utils/: 工具类头文件

### assets/

存放项目所需的资源文件：

- fonts/: 字体文件
- images/: 图片资源
- icons/: 图标资源

### docs/

存放项目文档：

- 项目结构说明.md: 本文档，说明项目结构
- 开发记录.md: 记录开发过程中的重要更新和决策
- PROJECT_STRUCTURE.md: 旧版项目结构说明
- ToDO.md: 待办事项列表

### build/

存放构建相关的脚本文件：

- build.ps1: PowerShell 构建脚本，用于自动化构建过程

### third_party/

存放第三方库依赖

### .vscode/

存放 VSCode 编辑器配置文件

## 文件命名规范

1. 源文件和头文件采用相同的名称，仅扩展名不同
2. 文件名使用大驼峰命名法（PascalCase）
3. 类名与文件名保持一致

## 模块说明

### GUI 模块

- MainWindow: 主窗口管理
- SimulationView: 模拟视图显示
- DeviceRenderer: 设备渲染
- WarehouseRenderer: 仓库渲染
- VehicleRenderer: 车辆渲染
- TrackRenderer: 轨道渲染
- UIControls: UI 控件管理
- Toolbar: 工具栏
- StatusPanel: 状态面板
- TaskList: 任务列表
- IconGenerator: 图标生成

### 核心模块

- 核心功能实现（待补充）

### 工具模块

- WarehouseUtils: 工具函数

## 构建系统

项目使用 CMake 作为构建系统，主要配置文件为根目录下的 CMakeLists.txt。构建过程可以通过 build 目录下的 PowerShell 脚本自动化执行。
