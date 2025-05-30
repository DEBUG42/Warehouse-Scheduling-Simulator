#include <SFML/Graphics.hpp>
#include <iostream>
#include "../include/gui/Toolbar.hpp"
#include "../include/gui/SimulationInterface.hpp"

class SimpleMockInterface : public SimulationInterface
{
public:
    SimulationMode currentMode = SimulationMode::TASK1;

    SimulationState getSimulationState() const override
    {
        SimulationState state;
        state.isPaused = false;
        state.simulationSpeedFactor = 1.0f;
        state.simulationTime = 0.0f;
        state.vehicleCount = 0;
        state.completedTaskCount = 0;
        state.pendingTaskCount = 0;
        return state;
    }

    std::vector<Vehicle *> getVehicleStates() const override
    {
        return {};
    }

    std::vector<DeviceBase *> getDeviceStates() const override
    {
        return {};
    }

    void setSimulationSpeedFactor(float speedFactor) override
    {
        std::cout << "Mock: Set speed factor to " << speedFactor << std::endl;
    }

    void pauseSimulation() override
    {
        std::cout << "Mock: Pause simulation" << std::endl;
    }

    void resumeSimulation() override
    {
        std::cout << "Mock: Resume simulation" << std::endl;
    }

    void resetSimulation() override
    {
        std::cout << "Mock: Reset simulation" << std::endl;
    }

    void registerStateUpdateCallback(StateUpdateCallback callback) override {}
    void registerVehicleUpdateCallback(VehicleUpdateCallback callback) override {}
    void registerDeviceUpdateCallback(DeviceUpdateCallback callback) override {}

    Vehicle *getVehicleStateById(int vehicleId) const override
    {
        return nullptr;
    }

    DeviceBase *getDeviceStateById(int deviceId) const override
    {
        return nullptr;
    }

    void updateBackend(float deltaTime) override {}

    // 新增的模式切换方法
    void setSimulationMode(SimulationMode mode) override
    {
        currentMode = mode;
        std::string modeStr;
        switch (mode)
        {
        case SimulationMode::TASK1:
            modeStr = "TASK1";
            break;
        case SimulationMode::TASK2_1:
            modeStr = "TASK2.1";
            break;
        case SimulationMode::TASK2_2:
            modeStr = "TASK2.2";
            break;
        case SimulationMode::TASK2_3:
            modeStr = "TASK2.3";
            break;
        }
        std::cout << "Mock: Simulation mode changed to " << modeStr << std::endl;
    }

    SimulationMode getCurrentSimulationMode() const override
    {
        return currentMode;
    }
};

int main()
{
    // 创建窗口
    sf::RenderWindow window(sf::VideoMode(1200, 100), "Mode Button Test");
    window.setFramerateLimit(60);

    // 加载字体
    sf::Font font;
    if (!font.loadFromFile("assets/fonts/arial.ttf"))
    {
        std::cerr << "Failed to load font" << std::endl;
        return 1;
    }

    // 创建模拟接口
    auto mockInterface = std::make_shared<SimpleMockInterface>();

    // 创建工具栏
    auto toolbar = std::make_unique<Toolbar>(font, 1200.0f, 50.0f);

    // 设置模式切换回调
    toolbar->setOnModeChanged([&mockInterface](SimulationMode mode)
                              { mockInterface->setSimulationMode(mode); });

    std::cout << "Mode Button Test Started. Click the mode buttons to test mode switching." << std::endl;
    std::cout << "Current mode: " << static_cast<int>(mockInterface->getCurrentSimulationMode()) << std::endl;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }

            // 处理工具栏事件
            sf::Vector2f mousePos = sf::Vector2f(sf::Mouse::getPosition(window));
            if (toolbar->handleEvent(event, mousePos))
            {
                // 事件被工具栏处理
            }
        }

        window.clear(sf::Color::Black);

        // 渲染工具栏
        toolbar->render(window, sf::Vector2f(0, 0));

        window.display();
    }

    return 0;
}
