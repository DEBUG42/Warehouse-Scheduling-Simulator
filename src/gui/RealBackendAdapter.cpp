#include "../../include/gui/RealBackendAdapter.hpp"
#include <algorithm>
#include <iostream>

RealBackendAdapter::RealBackendAdapter()
    : m_scheduler(), m_currentMode(SimulationMode::TASK1) // 默认构造 Scheduler，默认模式为 TASK1
{
    // 可在此初始化后端场景、车辆、设备等
    initializeBackend();
}

RealBackendAdapter::~RealBackendAdapter() {}

SimulationInterface::SimulationState RealBackendAdapter::getSimulationState() const
{ // TODO: 从 m_scheduler 或后端获取仿真状态
    SimulationInterface::SimulationState state;
    // 示例填充
    state.isPaused = false;             // 需根据后端实际状态
    state.simulationSpeedFactor = 1.0f; // 需根据后端实际状态
    state.simulationTime = 0.0f;        // 需根据后端实际状态
    state.vehicleCount = static_cast<int>(m_scheduler.getVehicles().size());
    state.completedTaskCount = 0; // 需根据后端实际状态
    state.pendingTaskCount = 0;   // 需根据后端实际状态
    return state;
}

std::vector<Vehicle *> RealBackendAdapter::getVehicleStates() const
{ // 假设 m_scheduler.getVehicles() 返回 std::vector<Vehicle>&
    auto &vehicles = m_scheduler.getVehicles();
    std::vector<Vehicle *> ptrs;
    for (const auto &v : vehicles)
    {
        ptrs.push_back(const_cast<Vehicle *>(&v));
    }
    return ptrs;
}

std::vector<DeviceBase *> RealBackendAdapter::getDeviceStates() const
{ // 假设 m_scheduler.getDevices() 返回 std::vector<DeviceBase>&
    auto &devices = m_scheduler.getDevices();
    std::vector<DeviceBase *> ptrs;
    for (const auto &d : devices)
    {
        ptrs.push_back(const_cast<DeviceBase *>(&d));
    }
    return ptrs;
}

void RealBackendAdapter::setSimulationSpeedFactor(float speedFactor)
{
    // TODO: 调用后端接口设置仿真速度
    std::cout << "Set simulation speed: " << speedFactor << std::endl;
}

void RealBackendAdapter::pauseSimulation()
{
    // TODO: 调用后端接口暂停仿真
    std::cout << "Pause simulation" << std::endl;
}

void RealBackendAdapter::resumeSimulation()
{
    // TODO: 调用后端接口恢复仿真
    std::cout << "Resume simulation" << std::endl;
}

void RealBackendAdapter::resetSimulation()
{
    // TODO: 调用后端接口重置仿真
    std::cout << "Reset simulation" << std::endl;
}

void RealBackendAdapter::registerStateUpdateCallback(StateUpdateCallback callback)
{
    m_stateUpdateCallback = callback;
}

void RealBackendAdapter::registerVehicleUpdateCallback(VehicleUpdateCallback callback)
{
    m_vehicleUpdateCallback = callback;
}

void RealBackendAdapter::registerDeviceUpdateCallback(DeviceUpdateCallback callback)
{
    m_deviceUpdateCallback = callback;
}

Vehicle *RealBackendAdapter::getVehicleStateById(int vehicleId) const
{
    auto &vehicles = m_scheduler.getVehicles();
    auto it = std::find_if(vehicles.begin(), vehicles.end(), [vehicleId](const Vehicle &v)
                           { return v.id == vehicleId; });
    if (it != vehicles.end())
        return const_cast<Vehicle *>(&(*it));
    return nullptr;
}

DeviceBase *RealBackendAdapter::getDeviceStateById(int deviceId) const
{
    auto &devices = m_scheduler.getDevices();
    auto it = std::find_if(devices.begin(), devices.end(), [deviceId](const DeviceBase &d)
                           { return d.m_id == deviceId; });
    if (it != devices.end())
        return const_cast<DeviceBase *>(&(*it));
    return nullptr;
}

void RealBackendAdapter::updateBackend(float deltaTime)
{
    // TODO: 调用后端调度器/物理引擎推进仿真
    // m_scheduler.update(deltaTime);
    // 触发回调
    if (m_stateUpdateCallback)
        m_stateUpdateCallback(getSimulationState());
    if (m_vehicleUpdateCallback)
        m_vehicleUpdateCallback(getVehicleStates());
    if (m_deviceUpdateCallback)
        m_deviceUpdateCallback(getDeviceStates());
}

void RealBackendAdapter::initializeBackend()
{
    // TODO: 初始化后端场景、车辆、设备等
    // m_scheduler.initialize(...);
}

void RealBackendAdapter::setSimulationMode(SimulationMode mode)
{
    m_currentMode = mode;

    // 将模式信息传递给后端调度器
    m_scheduler.setSimulationMode(mode);

    // 输出调试信息
    std::string modeStr;
    switch (mode)
    {
    case SimulationMode::TASK1:
        modeStr = "TASK1";
        // ========== 后端实现接口 ==========
        // TODO: 后端同学请在这里实现具体的TASK1初始化逻辑
        // 建议实现的功能：
        // 1. 重置仓库布局为标准配置
        // 2. 设置基础的FIFO任务调度
        // 3. 配置标准车辆数量（例如：3辆车）
        // 4. 设置基础速度参数
        // 5. 清空之前的任务队列
        // 示例调用：
        // m_scheduler.resetToBasicMode();
        // m_scheduler.setVehicleCount(3);
        // m_scheduler.setSchedulingAlgorithm(BASIC_FIFO);
        // ===================================
        break;
    case SimulationMode::TASK2_1:
        modeStr = "TASK2.1";
        // ========== 后端实现接口 ==========
        // TODO: 后端同学请在这里实现具体的TASK2.1初始化逻辑
        // 建议实现的功能：
        // 1. 启用优化调度算法版本1
        // 2. 增加车辆数量或调整车辆参数
        // 3. 实现距离优先的任务分配
        // 4. 启用基础的冲突避免机制
        // 示例调用：
        // m_scheduler.setSchedulingAlgorithm(OPTIMIZED_V1);
        // m_scheduler.enableDistancePriority(true);
        // m_scheduler.setVehicleCount(4);
        // ===================================
        break;
    case SimulationMode::TASK2_2:
        modeStr = "TASK2.2";
        // ========== 后端实现接口 ==========
        // TODO: 后端同学请在这里实现具体的TASK2.2初始化逻辑
        // 建议实现的功能：
        // 1. 启用优化调度算法版本2
        // 2. 实现动态路径规划
        // 3. 启用负载均衡机制
        // 4. 配置高级任务优先级策略
        // 示例调用：
        // m_scheduler.setSchedulingAlgorithm(OPTIMIZED_V2);
        // m_scheduler.enableDynamicPathPlanning(true);
        // m_scheduler.enableLoadBalancing(true);
        // ===================================
        break;
    case SimulationMode::TASK2_3:
        modeStr = "TASK2.3";
        // ========== 后端实现接口 ==========
        // TODO: 后端同学请在这里实现具体的TASK2.3初始化逻辑
        // 建议实现的功能：
        // 1. 启用最高级调度算法
        // 2. 实现预测性任务调度
        // 3. 启用智能冲突解决
        // 4. 配置复杂场景优化
        // 5. 实现多目标优化（时间、能耗、效率等）
        // 示例调用：
        // m_scheduler.setSchedulingAlgorithm(ADVANCED_AI);
        // m_scheduler.enablePredictiveScheduling(true);
        // m_scheduler.enableMultiObjectiveOptimization(true);
        // ===================================
        break;
    }

    std::cout << "[RealBackendAdapter] Simulation mode switched to: " << modeStr << std::endl;
}

SimulationMode RealBackendAdapter::getCurrentSimulationMode() const
{
    return m_currentMode;
}
