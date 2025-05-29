#include <SFML/Graphics.hpp>
#include <iostream>
#include <memory>
#include <vector>

// GUI组件包含
#include "gui/Toolbar.hpp"
#include "gui/StatusPanel.hpp"
#include "gui/VehicleInfoPanel.hpp"
#include "gui/UIControls.hpp"

/**
 * @brief GUI优化第一阶段简化演示测试
 *
 * 展示内容：
 * 1. Toolbar时间显示优化 - HH:MM:SS.mmm格式
 * 2. StatusPanel比例调整 - 25:35:40布局
 * 3. VehicleInfoPanel新组件 - 车辆加减速信息显示
 * 4. UIControls工具函数 - 时间格式化和颜色映射
 */

class SimpleDemoApp
{
private:
    sf::RenderWindow window;
    sf::Font font;

    // GUI组件
    std::unique_ptr<Toolbar> toolbar;
    std::unique_ptr<StatusPanel> statusPanel;
    std::unique_ptr<VehicleInfoPanel> vehicleInfoPanel;

    // 仿真状态
    float simulationTime = 0.0f;
    bool isRunning = false;
    // 布局参数
    static constexpr float TOOLBAR_HEIGHT = 50.0f;
    static constexpr float STATUS_PANEL_WIDTH = 300.0f;

public:
    SimpleDemoApp() : window(sf::VideoMode(1200, 800), "GUI Phase 1 - Simple Demo")
    {
        loadFont();
        initializeComponents();
    }

    bool loadFont()
    {        if (!font.loadFromFile("assets/fonts/arial.ttf"))
        {
            std::cerr << "Cannot load font file: assets/fonts/arial.ttf" << std::endl;
            // Try system font
            if (!font.loadFromFile("C:/Windows/Fonts/arial.ttf"))
            {
                std::cerr << "Cannot load system font file" << std::endl;
                return false;
            }
        }
        return true;
    }    void initializeComponents()
    {
        // 创建工具栏 - 修正参数顺序：(font, width, height)
        toolbar = std::make_unique<Toolbar>(font, window.getSize().x, TOOLBAR_HEIGHT);

        // 创建状态面板
        statusPanel = std::make_unique<StatusPanel>(font);
        statusPanel->resize(window.getSize().y - TOOLBAR_HEIGHT);
        
        // 创建车辆信息面板
        vehicleInfoPanel = std::make_unique<VehicleInfoPanel>(font, STATUS_PANEL_WIDTH, 400.0f);
          // 设置工具栏回调
        toolbar->setOnPlayPauseToggled([this]() {
            isRunning = !isRunning;
            std::cout << "Simulation State: " << (isRunning ? "Running" : "Paused") << std::endl;
        });
        
        toolbar->setOnTimeScaleChanged([this](float speed) {
            std::cout << "Speed adjusted to: " << speed << "x" << std::endl;
        });

        // 设置初始状态
        statusPanel->setSimulationTime(0.0f);
        statusPanel->setVehicleCount(3);
        statusPanel->setCompletedTaskCount(0);
        statusPanel->setPendingTaskCount(5);
    }    void handleEvents()
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }

            // 获取鼠标位置
            sf::Vector2i mousePixelPos = sf::Mouse::getPosition(window);
            sf::Vector2f mousePos = window.mapPixelToCoords(mousePixelPos);

            // 工具栏事件处理
            if (toolbar->handleEvent(event, mousePos)) {
                continue; // 工具栏消费了事件，跳过其他处理
            }

            if (event.type == sf::Event::KeyPressed)
            {
                switch (event.key.code)
                {                case sf::Keyboard::Space:
                    isRunning = !isRunning;
                    toolbar->setPlaying(isRunning);
                    std::cout << "Simulation State: " << (isRunning ? "Running" : "Paused") << std::endl;
                    break;
                case sf::Keyboard::R:
                    simulationTime = 0.0f;
                    isRunning = false;
                    toolbar->setPlaying(false);
                    std::cout << "Reset Simulation" << std::endl;
                    break;
                case sf::Keyboard::Escape:
                    window.close();
                    break;
                }
            }
        }
    }    void update(float deltaTime)
    {
        if (isRunning)
        {
            // 应用倍速系数
            float timeScale = toolbar->getCurrentSpeed();
            simulationTime += deltaTime * timeScale;

            // 更新工具栏时间显示
            toolbar->updateTimeDisplay(simulationTime);

            // 更新状态面板
            statusPanel->setSimulationTime(simulationTime);

            // 模拟车辆加减速事件（每3秒一次）
            static float lastEventTime = 0.0f;
            if (simulationTime - lastEventTime > 3.0f)
            {
                addSimulatedAccelerationEvent();
                lastEventTime = simulationTime;
            }
        }
    }

    void addSimulatedAccelerationEvent()
    {
        static int eventCounter = 0;
        eventCounter++;

        float startTime = simulationTime - 2.0f;
        float endTime = simulationTime;
        float startSpeed = 0.0f + (eventCounter % 3) * 0.5f;
        float endSpeed = 1.5f + (eventCounter % 4) * 0.3f;
        float acceleration = (endSpeed - startSpeed) / (endTime - startTime);        std::string eventType = (eventCounter % 2 == 0) ? "Acceleration" : "Deceleration";

        vehicleInfoPanel->recordAccelerationEvent(
            startTime, endTime, startSpeed, endSpeed, acceleration, eventType);

        std::cout << "Record " << eventType << " event: "
                  << startSpeed << "m/s -> " << endSpeed << "m/s" << std::endl;
    }

    void render()
    {
        window.clear(sf::Color(245, 245, 245)); // 浅灰色背景

        // 渲染工具栏
        toolbar->render(window, sf::Vector2f(0, 0));

        // 渲染状态面板
        sf::Vector2f statusPanelPos(window.getSize().x - STATUS_PANEL_WIDTH, TOOLBAR_HEIGHT);
        statusPanel->render(window, statusPanelPos);
        // 渲染车辆信息面板（在左侧）
        sf::Vector2f vehicleInfoPos(10, TOOLBAR_HEIGHT + 10);
        vehicleInfoPanel->setPosition(vehicleInfoPos);
        window.draw(*vehicleInfoPanel);

        // 渲染演示说明文字
        renderInstructions();

        window.display();
    }

    void renderInstructions()
    {
        sf::Text instructions;
        instructions.setFont(font);
        instructions.setCharacterSize(14);
        instructions.setFillColor(sf::Color::Black);
        instructions.setPosition(10, window.getSize().y - 100);        std::string text = "GUI Phase 1 Optimization Demo\n"
                           "Space: Start/Pause Simulation\n"
                           "R: Reset Simulation\n"
                           "ESC: Exit";
        instructions.setString(text);

        window.draw(instructions);
    }

    void run()
    {
        sf::Clock clock;        std::cout << "GUI Phase 1 Optimization Demo Started" << std::endl;
        std::cout << "Features:" << std::endl;
        std::cout << "1. Toolbar - Time format display (HH:MM:SS.mmm)" << std::endl;
        std::cout << "2. StatusPanel - 25:35:40 ratio layout" << std::endl;
        std::cout << "3. VehicleInfoPanel - Vehicle acceleration info recording" << std::endl;
        std::cout << "4. UIControls - Utility functions collection" << std::endl;
        std::cout << "\nInstructions: Press Space to start simulation, observe time formatting effect" << std::endl;

        while (window.isOpen())
        {
            float deltaTime = clock.restart().asSeconds();

            handleEvents();
            update(deltaTime);
            render();
        }
    }
};

int main()
{
    try
    {
        SimpleDemoApp app;
        app.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Demo program error: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}
