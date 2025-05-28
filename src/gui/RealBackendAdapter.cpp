#include "gui/RealBackendAdapter.hpp"
#include <algorithm>
#include <iostream>

RealBackendAdapter::RealBackendAdapter()
    : m_scheduler() // 默认构造 Scheduler，实际可根据需要传参
{
    // 可在此初始化后端场景、车辆、设备等
    initializeBackend();
}

RealBackendAdapter::~RealBackendAdapter() {}

RealBackendAdapter::SimulationState RealBackendAdapter::getSimulationState() const
{
    // TODO: 从 m_scheduler 或后端获取仿真状态
    SimulationState state;
    // 示例填充
    state.isPaused = false;             // 需根据后端实际状态
    state.simulationSpeedFactor = 1.0f; // 需根据后端实际状态
    state.simulationTime = 0.0f;        // 需根据后端实际状态
    state.vehicleCount = static_cast<int>(m_scheduler.getVehicles().size());
    state.completedTaskCount = 0; // 需根据后端实际状态
    state.pendingTaskCount = 0;   // 需根据后端实际状态
    return state;
}

std::vector<Core::Vehicle *> RealBackendAdapter::getVehicleStates() const
{
    // 假设 m_scheduler.getVehicles() 返回 std::vector<Vehicle>&
    auto &vehicles = m_scheduler.getVehicles();
    std::vector<Core::Vehicle *> ptrs;
    for (auto &v : vehicles)
    {
        ptrs.push_back(&v);
    }
    return ptrs;
}

std::vector<Core::DeviceBase *> RealBackendAdapter::getDeviceStates() const
{
    // 假设 m_scheduler.getDevices() 返回 std::vector<DeviceBase>&
    auto &devices = m_scheduler.getDevices();
    std::vector<Core::DeviceBase *> ptrs;
    for (auto &d : devices)
    {
        ptrs.push_back(&d);
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

Core::Vehicle *RealBackendAdapter::getVehicleStateById(int vehicleId) const
{
    auto &vehicles = m_scheduler.getVehicles();
    auto it = std::find_if(vehicles.begin(), vehicles.end(), [vehicleId](const Core::Vehicle &v)
                           { return v.id == vehicleId; });
    if (it != vehicles.end())
        return const_cast<Core::Vehicle *>(&(*it));
    return nullptr;
}

Core::DeviceBase *RealBackendAdapter::getDeviceStateById(int deviceId) const
{
    auto &devices = m_scheduler.getDevices();
    auto it = std::find_if(devices.begin(), devices.end(), [deviceId](const Core::DeviceBase &d)
                           { return d.m_id == deviceId; });
    if (it != devices.end())
        return const_cast<Core::DeviceBase *>(&(*it));
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
