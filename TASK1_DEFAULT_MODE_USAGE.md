# TASK1 默认模式使用说明

## 功能概述

已成功实现前后端分离架构，并配置 TASK1 为默认启动模式，程序启动时会自动显示 3 辆运动的车辆。

## 主要特性

### 1. 默认 TASK1 模式启动

- ✅ 程序启动时自动进入 TASK1 模式
- ✅ 自动初始化 3 辆车辆
- ✅ 车辆随机分配目标设备（1-18，除设备 15）
- ✅ 车辆立即开始运动（Accelerating 状态）

### 2. 前后端分离架构

- ✅ GUI 前端可以独立创建：`SimpleDemoApp app;`
- ✅ 后端调度器单独连接：`app.setScheduler(&scheduler);`
- ✅ 支持运行时模式切换和车辆更新

### 3. 模式切换功能

- ✅ 工具栏默认显示 TASK1 模式
- ✅ 点击模式按钮可切换到其他模式
- ✅ 切换到 TASK1 时重新初始化车辆并开始运动
- ✅ 模式变化会通知后端

### 4. 车辆显示和运动

- ✅ 车辆在轨道上正常显示
- ✅ 实时加速减速运动
- ✅ VehicleInfoPanel 记录运动状态变化
- ✅ 支持车辆选择和详细信息查看

## 使用方法

### 启动程序

```bash
cd "d:\CodeMaster\GitHub\C++SFML\Warehouse_debug\Warehouse-Scheduling-Simulator"
.\compile_test6_4.bat
```

### 界面操作

1. **仿真控制**：

   - 空格键：开始/暂停仿真
   - ESC 键：退出程序

2. **显示控制**：

   - T 键：切换坐标网格显示
   - G 键：切换仓库显示
   - V 键：切换车辆显示
   - C 键：切换调试信息显示

3. **模式操作**：

   - 点击工具栏模式按钮切换 TASK1/TASK2_1/TASK2_2/TASK2_3
   - 模式切换会自动重置仿真时间并暂停仿真
   - 切换到 TASK1 会重新初始化 3 辆车辆并开始运动

4. **车辆监视**：
   - 1/2/3 键：选择对应编号的车辆
   - 鼠标点击：选择车辆或设备
   - 右侧面板显示实时车辆状态和加速信息
   - 使用倍速控制观察车辆运动（0.1x - 10x）

### 控制台输出示例

```
Font loaded successfully: C:/Windows/Fonts/COOPBL.TTF
Initializing default TASK1 mode...
Vehicle targets set: 11, 8, 2
Vehicles set to Accelerating state for immediate movement
前端工作已做好: 收到3 后端小车
GUI refreshed to display vehicles
Frontend ready for backend vehicle data...
Simulation Status: Running
Mode changed to: TASK2_1
仿真时间已重置，仿真已暂停，前端传入工作已经完成，后端确认
Mode changed to: TASK1
TASK1 vehicles targets: 1, 12, 17
Record Acceleration event: 0.5m/s -> 1.5m/s
```

## 技术实现

### 架构设计

```cpp
// 1. 独立前端初始化
SimpleDemoApp app;

// 2. 后端连接
app.setScheduler(&scheduler);

// 3. 默认TASK1模式设置
toolbar->setCurrentMode(SimulationMode::TASK1);

// 4. 车辆运动初始化
vehicles[i].m_state.motionState = Vehicle::MotionState::Accelerating;
```

### 关键改进

1. **构造函数分离**：`SimpleDemoApp()` 不再需要 scheduler 参数
2. **运行时连接**：`setScheduler()` 方法支持后期绑定
3. **状态同步**：`updateSchedulerDependentComponents()` 确保 GUI 更新
4. **模式切换重置**：每次模式切换自动重置时间并暂停仿真
5. **集成化设计**：移除手动测试功能，专注任务驱动的车辆控制

## 验证完成

✅ **测试功能移除**：已删除 `moveSelectedVehicle` 和 `resetSimulation` 方法及相关键盘控制（W/S/R 键）
✅ **前后端分离**：前端可独立初始化，后端通过 `setScheduler()` 方法连接
✅ **默认 TASK1 模式**：程序启动即进入 TASK1 模式并显示运动车辆
✅ **模式切换重置**：切换模式时自动重置仿真时间为 0 并暂停
✅ **编译运行成功**：所有功能正常工作，无编译错误 4. **默认运动**：车辆初始化后立即设置为 Accelerating 状态

## 成功验证

✅ 所有目标均已实现：

- 开机默认 TASK1 模式并显示车辆
- 车辆立即开始运动
- 模式切换向后端传递信息
- 前后端分离架构完整
- GUI 实时更新车辆状态

程序现在可以作为一个完整的仓库调度仿真器前端使用，支持独立初始化和后端动态连接。
