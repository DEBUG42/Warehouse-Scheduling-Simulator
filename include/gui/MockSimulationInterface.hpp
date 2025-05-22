#ifndef MOCK_SIMULATION_INTERFACE_HPP
#define MOCK_SIMULATION_INTERFACE_HPP

#include "SimulationInterface.hpp"
#include "DeviceState.hpp"
#include "SimObject.hpp"
#include <random>
#include <chrono>
#include <thread>

/**
 * @class MockSimulationInterface
 * @brief 模拟的仿真接口实现，用于GUI测试
 *
 * 提供模拟的仿真数据，用于GUI开发和测试阶段，无需后端支持
 * 会自动模拟车辆沿轨道运行和设备状态变化
 */
class MockSimulationInterface : public SimulationInterface
{
public:
    /**
     * @brief 构造函数
     * @param vehicleCount 模拟车辆数量
     */
    MockSimulationInterface(int vehicleCount = 5)
        : m_vehicleCount(vehicleCount)
    {
        // 初始化随机数生成器
        std::random_device rd;
        m_rng = std::mt19937(rd());
        m_speedDist = std::uniform_real_distribution<float>(0.5f, 3.0f);
        m_posDist = std::uniform_real_distribution<float>(0.0f, 90000.0f);

        // 初始化仿真状态
        m_state.isPaused = false;
        m_state.speed = SimulationSpeed::SPEED_1X;
        m_state.simulationTime = 0.0f;
        m_state.vehicleCount = vehicleCount;
        m_state.completedTaskCount = 0;
        m_state.pendingTaskCount = 5;

        // 创建模拟车辆
        createMockVehicles(vehicleCount);

        // 创建模拟设备
        createMockDevices();

        // 启动更新线程
        m_running = true;
        m_updateThread = std::thread(&MockSimulationInterface::updateLoop, this);
    }

    /**
     * @brief 析构函数
     */
    virtual ~MockSimulationInterface() override
    {
        // 停止更新线程
        m_running = false;
        if (m_updateThread.joinable())
        {
            m_updateThread.join();
        }
    }

    /**
     * @brief 获取当前仿真状态
     * @return 仿真状态结构体
     */
    virtual SimulationState getSimulationState() const override
    {
        return m_state;
    }

    /**
     * @brief 获取所有车辆状态
     * @return 车辆状态向量
     */
    virtual std::vector<gui::VehicleState> getVehicleStates() const override
    {
        return m_vehicles;
    }

    /**
     * @brief 获取所有设备状态
     * @return 设备状态向量
     */
    virtual std::vector<gui::DeviceState> getDeviceStates() const override
    {
        return m_devices;
    }

    /**
     * @brief 设置仿真速度
     * @param speed 仿真速度枚举
     */
    virtual void setSimulationSpeed(SimulationSpeed speed) override
    {
        m_state.speed = speed;
    }

    /**
     * @brief 暂停仿真
     */
    virtual void pauseSimulation() override
    {
        m_state.isPaused = true;
    }

    /**
     * @brief 恢复仿真
     */
    virtual void resumeSimulation() override
    {
        m_state.isPaused = false;
    }

    /**
     * @brief 重置仿真
     */
    virtual void resetSimulation() override
    {
        m_state.simulationTime = 0.0f;
        m_state.completedTaskCount = 0;
        createMockVehicles(m_vehicleCount);
        for (auto &device : m_devices)
        {
            device.status = gui::DeviceStatus::IDLE;
            device.processingProgress = 0.0f;
            device.queuedTaskCount = 0;
        }
    }

    /**
     * @brief 注册仿真状态更新回调函数
     * @param callback 回调函数
     */
    virtual void registerStateUpdateCallback(StateUpdateCallback callback) override
    {
        m_stateCallback = callback;
    }

    /**
     * @brief 注册车辆状态更新回调函数
     * @param callback 回调函数
     */
    virtual void registerVehicleUpdateCallback(VehicleUpdateCallback callback) override
    {
        m_vehicleCallback = callback;
    }

    /**
     * @brief 注册设备状态更新回调函数
     * @param callback 回调函数
     */
    virtual void registerDeviceUpdateCallback(DeviceUpdateCallback callback) override
    {
        m_deviceCallback = callback;
    }

    /**
     * @brief 获取指定ID的车辆状态
     * @param vehicleId 车辆ID
     * @return 车辆状态，如果找不到则返回默认状态
     */
    virtual gui::VehicleState getVehicleState(int vehicleId) const override
    {
        std::string idStr = std::to_string(vehicleId);
        for (const auto &vehicle : m_vehicles)
        {
            if (vehicle.getId() == idStr)
                return vehicle;
        }
        gui::VehicleState defaultState;
        return defaultState;
    }

    /**
     * @brief 获取指定ID的设备状态
     * @param deviceId 设备ID
     * @return 设备状态，如果找不到则返回默认状态
     */
    virtual gui::DeviceState getDeviceState(int deviceId) const override
    {
        std::string idStr = std::to_string(deviceId);
        for (const auto &device : m_devices)
        {
            if (device.getId() == idStr)
                return device;
        }
        gui::DeviceState defaultState;
        return defaultState;
    }

private:
    /**
     * @brief 创建模拟车辆
     * @param count 车辆数量
     */
    void createMockVehicles(int count)
    {
        m_vehicles.clear();

        for (int i = 0; i < count; ++i)
        {
            gui::VehicleState vehicle;
            vehicle.speed = m_speedDist(m_rng); // 随机速度
            vehicle.isLoaded = (i % 3 == 0);    // 部分车辆载货
            m_vehicles.push_back(vehicle);
        }
    }

    /**
     * @brief 创建模拟设备
     */
    void createMockDevices()
    {
        m_devices.clear();
        for (int i = 1; i <= 18; ++i)
        {
            gui::DeviceState device;
            if (i % 2 == 1 && i <= 12)
                device.deviceType = gui::DeviceType::INPUT_STATION;
            else if (i % 2 == 0 && i <= 12)
                device.deviceType = gui::DeviceType::OUTPUT_STATION;
            else if (i >= 13 && i <= 15)
                device.deviceType = gui::DeviceType::OUTPUT_STATION;
            else
                device.deviceType = gui::DeviceType::INPUT_STATION;
            device.status = gui::DeviceStatus::IDLE;
            device.capacity = 1;
            device.currentLoad = 0;
            device.materialId = -1;
            device.processingProgress = 0.0f;
            device.queuedTaskCount = 0;
            device.position = sf::Vector2f(0.0f, 0.0f);
            m_devices.push_back(device);
        }
    }

    /**
     * @brief 更新循环（在单独线程中运行）
     */
    void updateLoop()
    {
        auto lastUpdateTime = std::chrono::steady_clock::now();

        while (m_running)
        {
            auto currentTime = std::chrono::steady_clock::now();
            float deltaTime = std::chrono::duration<float>(currentTime - lastUpdateTime).count();
            lastUpdateTime = currentTime;

            // 如果仿真未暂停，则更新
            if (!m_state.isPaused)
            {
                // 根据仿真速度调整时间流逝
                float timeScale = 1.0f;
                switch (m_state.speed)
                {
                case SimulationSpeed::SPEED_0_5X:
                    timeScale = 0.5f;
                    break;
                case SimulationSpeed::SPEED_1X:
                    timeScale = 1.0f;
                    break;
                case SimulationSpeed::SPEED_2X:
                    timeScale = 2.0f;
                    break;
                case SimulationSpeed::SPEED_5X:
                    timeScale = 5.0f;
                    break;
                case SimulationSpeed::SPEED_10X:
                    timeScale = 10.0f;
                    break;
                }

                // 更新仿真时间
                m_state.simulationTime += deltaTime * timeScale;

                // 更新车辆位置
                updateVehicles(deltaTime * timeScale);

                // 更新设备状态
                updateDevices(deltaTime * timeScale);

                // 随机完成任务
                if (m_state.simulationTime - m_lastTaskCompletionTime > 10.0f)
                {
                    m_state.completedTaskCount++;
                    m_lastTaskCompletionTime = m_state.simulationTime;
                }

                // 触发回调
                if (m_stateCallback)
                {
                    m_stateCallback(m_state);
                }

                if (m_vehicleCallback)
                {
                    m_vehicleCallback(m_vehicles);
                }

                if (m_deviceCallback)
                {
                    m_deviceCallback(m_devices);
                }
            }

            // 控制更新频率（每秒约30次）
            std::this_thread::sleep_for(std::chrono::milliseconds(33));
        }
    }

    /**
     * @brief 更新车辆状态
     * @param deltaTime 时间增量
     */
    void updateVehicles(float deltaTime)
    {
        // 轨道总长度（毫米）
        const float trackLength = 2 * 40000.0f + 2 * M_PI * 2500.0f;
        for (auto &vehicle : m_vehicles)
        {
            // 直接让所有车辆都移动
            vehicle.status = gui::VehicleStatus::MOVING_TO_LOAD;
            vehicle.rawTrackPositionMm += vehicle.speed * 1000.0f * deltaTime; // 速度m/s转mm/s
            while (vehicle.rawTrackPositionMm >= trackLength)
            {
                vehicle.rawTrackPositionMm -= trackLength;
            }
            // 随机变化速度（小幅度）
            if (rand() % 100 < 5)
            {
                vehicle.speed = m_speedDist(m_rng);
            }
            // 随机改变载货状态
            if (rand() % 1000 < 2)
            {
                vehicle.isLoaded = !vehicle.isLoaded;
            }
            // 随机改变任务状态
            if (rand() % 1000 < 1)
            {
                vehicle.currentTaskId = (vehicle.currentTaskId.empty() || vehicle.currentTaskId == "-1") ? std::to_string(rand() % 20) : "-1";
            }
        }
    }

    /**
     * @brief 更新设备状态
     * @param deltaTime 时间增量
     */
    void updateDevices(float deltaTime)
    {
        for (auto &device : m_devices)
        {
            if (device.status == gui::DeviceStatus::IDLE)
            {
                device.status = gui::DeviceStatus::WORKING;
                device.processingProgress = 0.0f;
                device.currentLoad = 1;
                device.materialId = rand() % 1000;
            }
            else
            {
                device.status = gui::DeviceStatus::IDLE;
                device.processingProgress = 0.0f;
                device.currentLoad = 0;
                device.materialId = -1;
            }
            if (device.status == gui::DeviceStatus::WORKING)
            {
                device.processingProgress += deltaTime * 0.1f;
                if (device.processingProgress >= 1.0f)
                {
                    device.processingProgress = 0.0f;
                    device.currentLoad = 0;
                    device.materialId = -1;
                    if (rand() % 2 == 0)
                        device.status = gui::DeviceStatus::IDLE;
                    else
                    {
                        device.currentLoad = 1;
                        device.materialId = rand() % 1000;
                    }
                }
            }
            if (rand() % 500 < 1)
                device.queuedTaskCount = rand() % 3;
        }
    }

private:
    int m_vehicleCount;                        ///< 车辆数量
    SimulationState m_state;                   ///< 仿真状态
    std::vector<gui::VehicleState> m_vehicles; ///< 车辆状态
    std::vector<gui::DeviceState> m_devices;   ///< 设备状态

    bool m_running;             ///< 线程运行标志
    std::thread m_updateThread; ///< 更新线程

    StateUpdateCallback m_stateCallback;     ///< 状态更新回调
    VehicleUpdateCallback m_vehicleCallback; ///< 车辆更新回调
    DeviceUpdateCallback m_deviceCallback;   ///< 设备更新回调

    std::mt19937 m_rng;                                ///< 随机数生成器
    std::uniform_real_distribution<float> m_speedDist; ///< 速度分布
    std::uniform_real_distribution<float> m_posDist;   ///< 位置分布

    float m_lastTaskCompletionTime = 0.0f; ///< 上次完成任务的时间
};

#endif // MOCK_SIMULATION_INTERFACE_HPP
