#include <iostream>
#include <memory>
#include <SFML/Graphics.hpp>
#include "gui/MainWindow.hpp"
#include "gui/SimulationInterface.hpp"
#include "../src/Core/Vehicle.hpp"
#include "../src/Core/Task.hpp"

// 简单的测试仿真接口
class TestSimulationInterface : public SimulationInterface
{
private:
    float m_simulationTime = 0.0f;
    bool m_isPaused = false;
    float m_speedFactor = 1.0f;
    std::vector<Vehicle *> m_vehicles;
    std::vector<DeviceBase *> m_devices;

    VehicleUpdateCallback m_vehicleCallback;
    DeviceUpdateCallback m_deviceCallback;
    StateUpdateCallback m_stateCallback;

public:
    TestSimulationInterface()
    {
        // 创建测试车辆
        auto vehicle1 = new Vehicle();
        vehicle1->id = 1;
        vehicle1->is_loaded = false;
        vehicle1->position_m = 0.0f;
        m_vehicles.push_back(vehicle1);

        auto vehicle2 = new Vehicle();
        vehicle2->id = 2;
        vehicle2->is_loaded = true;
        vehicle2->position_m = 4000.0f;
        m_vehicles.push_back(vehicle2);
    }

    ~TestSimulationInterface()
    {
        // 清理车辆
        for (auto *vehicle : m_vehicles)
        {
            delete vehicle;
        }
        for (auto *device : m_devices)
        {
            delete device;
        }
    }

    // 实现SimulationInterface接口
    void registerVehicleUpdateCallback(VehicleUpdateCallback callback) override
    {
        m_vehicleCallback = callback;
    }

    void registerDeviceUpdateCallback(DeviceUpdateCallback callback) override
    {
        m_deviceCallback = callback;
    }

    void registerStateUpdateCallback(StateUpdateCallback callback) override
    {
        m_stateCallback = callback;
    }

    SimulationState getSimulationState() const override
    {
        SimulationState state;
        state.simulationTime = m_simulationTime;
        state.isPaused = m_isPaused;
        state.simulationSpeedFactor = m_speedFactor;
        state.vehicleCount = static_cast<int>(m_vehicles.size());
        state.completedTaskCount = 5;
        state.pendingTaskCount = 3;
        return state;
    }

    void setSimulationSpeedFactor(float factor) override
    {
        m_speedFactor = factor;
        std::cout << "Speed factor set to: " << factor << std::endl;
    }

    void pauseSimulation() override
    {
        m_isPaused = true;
        std::cout << "Simulation paused" << std::endl;
    }

    void resumeSimulation() override
    {
        m_isPaused = false;
        std::cout << "Simulation resumed" << std::endl;
    }

    // 实现缺失的纯虚函数
    std::vector<Vehicle *> getVehicleStates() const override
    {
        return m_vehicles;
    }

    std::vector<DeviceBase *> getDeviceStates() const override
    {
        return m_devices;
    }

    void resetSimulation() override
    {
        m_simulationTime = 0.0f;
        m_isPaused = false;
        // 更多重置逻辑可以根据需要添加
        std::cout << "Simulation reset" << std::endl;
    }

    Vehicle *getVehicleStateById(int vehicleId) const override
    {
        for (auto *v : m_vehicles)
        {
            if (v->getId() == vehicleId)
            {
                return v;
            }
        }
        return nullptr;
    }

    DeviceBase *getDeviceStateById(int deviceId) const override
    {
        for (auto *d : m_devices)
        {
            if (d->m_id == deviceId)
            { // 假设DeviceBase也有getId()
                return d;
            }
        }
        return nullptr;
    }

    // 测试方法
    void update()
    {
        m_simulationTime += 0.016f; // 约60FPS

        // 更新车辆位置（简单移动）
        for (auto *vehicle : m_vehicles)
        {
            vehicle->m_state.currentSpeed = 2.0f * m_speedFactor; // 模拟速度
            vehicle->m_state.position += vehicle->m_state.currentSpeed * 0.016f * m_speedFactor;
            // 模拟车辆状态变化
        }

        // 触发回调
        if (m_stateCallback)
        {
            m_stateCallback(getSimulationState());
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
};

int main()
{
    std::cout << "Starting Simple GUI Integration Test..." << std::endl;

    try
    {
        // 创建测试仿真接口
        auto testInterface = std::make_shared<TestSimulationInterface>();

        // 创建主窗口
        MainWindow window;
        window.initialize(testInterface);

        std::cout << "MainWindow initialized successfully!" << std::endl;

        sf::Clock clock;
        int frameCount = 0;
        const int maxFrames = 600; // 10秒测试（60FPS）

        // 运行测试循环
        while (window.isOpen() && frameCount < maxFrames)
        {
            float deltaTime = clock.restart().asSeconds();

            // 处理事件
            sf::Event event;
            while (window.pollEvent(event))
            {
                window.processEvent(event);

                if (event.type == sf::Event::Closed)
                {
                    window.close();
                    break;
                }

                // 测试键盘事件
                if (event.type == sf::Event::KeyPressed)
                {
                    if (event.key.code == sf::Keyboard::Space)
                    {
                        std::cout << "Space pressed - toggling simulation" << std::endl;
                        auto state = testInterface->getSimulationState();
                        if (state.isPaused)
                            testInterface->resumeSimulation();
                        else
                            testInterface->pauseSimulation();
                    }
                }
            }

            // 更新仿真
            testInterface->update();

            // 渲染
            window.renderFrame();
            window.display();

            frameCount++;

            // 每秒打印一次状态
            if (frameCount % 60 == 0)
            {
                std::cout << "Frame " << frameCount << " - Test running..." << std::endl;
            }
        }

        std::cout << "Test completed successfully after " << frameCount << " frames!" << std::endl;
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
    catch (...)
    {
        std::cerr << "Test failed with unknown exception" << std::endl;
        return 1;
    }
}
