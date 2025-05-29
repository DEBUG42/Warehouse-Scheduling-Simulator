#ifndef SIMULATION_INTERFACE_HPP
#define SIMULATION_INTERFACE_HPP

#include <vector>
#include <functional>
#include "../src/Core/Vehicle.hpp"
#include "../src/Core/Device.hpp"

/**
 * @class SimulationInterface
 * @brief 前后端通信接口类，负责处理GUI与仿真引擎之间的数据交换
 *
 * 定义了前后端通信的标准接口，包括：
 * 1. 获取车辆和设备状态
 * 2. 控制仿真速度和暂停/继续
 * 3. 注册状态更新回调
 */
class SimulationInterface
{
public:
    /// 仿真状态结构体
    struct SimulationState
    {
        bool isPaused;               ///< 是否暂停
        float simulationSpeedFactor; ///< 仿真速度因子 (e.g., 1.0 for normal, 2.0 for 2x)
        float simulationTime;        ///< 仿真时间(s)
        int vehicleCount;            ///< 车辆数量
        int completedTaskCount;      ///< 已完成任务数量
        int pendingTaskCount;        ///< 待处理任务数量
    };

    /// 状态更新回调函数类型
    using StateUpdateCallback = std::function<void(const SimulationState &)>;
    using VehicleUpdateCallback = std::function<void(const std::vector<Vehicle *> &)>;
    using DeviceUpdateCallback = std::function<void(const std::vector<DeviceBase *> &)>;

public:
    /// 虚析构函数
    virtual ~SimulationInterface() = default;

    /**
     * @brief 获取当前仿真状态
     * @return 仿真状态结构体
     */
    virtual SimulationState getSimulationState() const = 0; /**
                                                             * @brief 获取所有车辆状态
                                                             * @return 车辆状态向量
                                                             */
    virtual std::vector<Vehicle *> getVehicleStates() const = 0;

    /**
     * @brief 获取所有设备状态
     * @return 设备状态向量
     */
    virtual std::vector<DeviceBase *> getDeviceStates() const = 0;

    /**
     * @brief 设置仿真速度因子
     * @param speedFactor 仿真速度因子 (e.g., 1.0f for normal speed)
     */
    virtual void setSimulationSpeedFactor(float speedFactor) = 0;

    /**
     * @brief 暂停仿真
     */
    virtual void pauseSimulation() = 0;

    /**
     * @brief 恢复仿真
     */
    virtual void resumeSimulation() = 0;

    /**
     * @brief 重置仿真
     */
    virtual void resetSimulation() = 0;

    /**
     * @brief 注册仿真状态更新回调函数
     * @param callback 回调函数
     */
    virtual void registerStateUpdateCallback(StateUpdateCallback callback) = 0;

    /**
     * @brief 注册车辆状态更新回调函数
     * @param callback 回调函数
     */
    virtual void registerVehicleUpdateCallback(VehicleUpdateCallback callback) = 0;

    /**
     * @brief 注册设备状态更新回调函数
     * @param callback 回调函数
     */
    virtual void registerDeviceUpdateCallback(DeviceUpdateCallback callback) = 0; /**
                                                                                   * @brief 获取指定ID的车辆状态
                                                                                   * @param vehicleId 车辆ID
                                                                                   * @return 车辆状态，如果找不到则返回默认状态
                                                                                   */
    virtual Vehicle *getVehicleStateById(int vehicleId) const = 0;

    /**
     * @brief 获取指定ID的设备状态
     * @param deviceId 设备ID
     * @return 设备状态，如果找不到则返回默认状态
     */
    virtual DeviceBase *getDeviceStateById(int deviceId) const = 0;

    // TODO: Add methods for sending commands to the backend, e.g.,
    // virtual void sendTaskToBackend(const Core::Task& task) = 0;
    // virtual void emergencyStopVehicle(int vehicleId) = 0;
};

#endif // SIMULATION_INTERFACE_HPP
