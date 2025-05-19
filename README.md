# Warehouse-Scheduling-Simulator

这是一个仓库物流穿梭车系统的 3D 仿真和调度项目。项目提供了穿梭车系统的可视化仿真、轨道布局呈现以及调度算法实现。

## 项目结构

项目主要分为三个部分：

- **Core**: 仿真核心和业务逻辑
- **GUI**: 图形用户界面和 3D 可视化
- **Physics**: 物理运动和轨道计算

详细的项目结构请参考 [PROJECT_STRUCTURE.md](PROJECT_STRUCTURE.md)

## 编译与运行

### 环境要求

- C++17 或更高版本
- CMake 3.10+
- SFML 2.6.2+

### 编译步骤

```powershell
cd GUI
./build.ps1
```

或手动编译:

```powershell
cd GUI
cmake -S . -B build
cmake --build build
```

## 主要功能

- 3D 可视化的仓库穿梭车系统仿真
- 轨道和车辆的物理模拟
- 多种调度算法实现和比较
- 支持不同数量穿梭车的调度模拟
