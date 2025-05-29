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
    {
        if (!font.loadFromFile("assets/fonts/arial.ttf"))
        {
            std::cerr << "无法加载字体文件: assets/fonts/arial.ttf" << std::endl;
            // 尝试系统字体
            if (!font.loadFromFile("C:/Windows/Fonts/arial.ttf"))
            {
                std::cerr << "无法加载系统字体文件" << std::endl;
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
            std::cout << "仿真状态: " << (isRunning ? "运行" : "暂停") << std::endl;
        });
        
        toolbar->setOnTimeScaleChanged([this](float speed) {
            std::cout << "速度调整为: " << speed << "x" << std::endl;
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
                {
                case sf::Keyboard::Space:
                    isRunning = !isRunning;
                    toolbar->setPlaying(isRunning);
                    std::cout << "仿真状态: " << (isRunning ? "运行" : "暂停") << std::endl;
                    break;
                case sf::Keyboard::R:
                    simulationTime = 0.0f;
                    isRunning = false;
                    toolbar->setPlaying(false);
                    std::cout << "重置仿真" << std::endl;
                    break;
                case sf::Keyboard::Escape:
                    window.close();
                    break;
                }
            }
        }
    }

    void update(float deltaTime)
    {
        if (isRunning)
        {
            simulationTime += deltaTime;

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
        float acceleration = (endSpeed - startSpeed) / (endTime - startTime);

        std::string eventType = (eventCounter % 2 == 0) ? "加速" : "减速";

        vehicleInfoPanel->recordAccelerationEvent(
            startTime, endTime, startSpeed, endSpeed, acceleration, eventType);

        std::cout << "记录" << eventType << "事件: "
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
        instructions.setPosition(10, window.getSize().y - 100);

        std::string text = "GUI第一阶段优化演示\n"
                           "空格键: 开始/暂停仿真\n"
                           "R键: 重置仿真\n"
                           "ESC键: 退出";
        instructions.setString(text);

        window.draw(instructions);
    }

    void run()
    {
        sf::Clock clock;

        std::cout << "GUI第一阶段优化演示启动" << std::endl;
        std::cout << "展示功能:" << std::endl;
        std::cout << "1. Toolbar - 时间格式化显示 (HH:MM:SS.mmm)" << std::endl;
        std::cout << "2. StatusPanel - 25:35:40比例布局" << std::endl;
        std::cout << "3. VehicleInfoPanel - 车辆加减速信息记录" << std::endl;
        std::cout << "4. UIControls - 工具函数集合" << std::endl;
        std::cout << "\n操作说明: 空格键开始仿真，观察时间格式化效果" << std::endl;

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
        std::cerr << "演示程序错误: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}
