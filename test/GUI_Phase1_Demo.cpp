#include <SFML/Graphics.hpp>
#include <iostream>
#include <memory>
#include <vector>

// GUI组件包含
#include "gui/Toolbar.hpp"
#include "gui/StatusPanel.hpp"
#include "gui/VehicleInfoPanel.hpp"
#include "gui/UIControls.hpp"

// Core组件包含
#include "Core/Vehicle.hpp"

/**
 * @brief GUI优化第一阶段成果展示测试
 *
 * 展示内容：
 * 1. Toolbar时间显示优化 - HH:MM:SS.mmm格式
 * 2. StatusPanel比例调整 - 25:35:40布局
 * 3. VehicleInfoPanel新组件 - 车辆加减速信息显示
 */
class GUI_Phase1_Demo
{
private:
    sf::RenderWindow m_window;
    sf::Font m_font;

    // GUI组件
    std::unique_ptr<Toolbar> m_toolbar;
    std::unique_ptr<StatusPanel> m_statusPanel;
    std::unique_ptr<VehicleInfoPanel> m_vehicleInfoPanel;

    // 测试数据
    std::vector<Vehicle> m_testVehicles;
    float m_simulationTime;
    bool m_isPlaying;
    float m_timeScale;

    // 布局参数
    const float WINDOW_WIDTH = 1200.0f;
    const float WINDOW_HEIGHT = 800.0f;
    const float TOOLBAR_HEIGHT = 60.0f;
    const float PANEL_WIDTH = 300.0f;

public:
    GUI_Phase1_Demo()
        : m_window(sf::VideoMode(static_cast<unsigned int>(WINDOW_WIDTH),
                                 static_cast<unsigned int>(WINDOW_HEIGHT)),
                   "GUI Phase 1 Optimization Demo - Warehouse Scheduling System"),
          m_simulationTime(0.0f), m_isPlaying(false), m_timeScale(1.0f)
    {
        m_window.setFramerateLimit(60);

        // 加载字体
        if (!m_font.loadFromFile("assets/fonts/arial.ttf"))
        {
            std::cerr << "Error loading font!" << std::endl;
            return;
        }

        initializeComponents();
        createTestData();
    }

    void initializeComponents()
    {
        // 创建Toolbar - 展示时间格式优化
        m_toolbar = std::make_unique<Toolbar>(m_font, TOOLBAR_HEIGHT, WINDOW_WIDTH);
        m_toolbar->createFunctionButtons();

        // 设置Toolbar回调
        m_toolbar->setPlayPauseCallback([this]()
                                        {
            m_isPlaying = !m_isPlaying;
            m_toolbar->updatePlayPauseState(m_isPlaying);
            std::cout << "Simulation " << (m_isPlaying ? "Started" : "Paused") << std::endl; });

        m_toolbar->setTimeScaleCallback([this](float scale)
                                        {
            m_timeScale = scale;
            std::cout << "Time scale changed to: " << scale << "x" << std::endl; });

        m_toolbar->setResetViewCallback([this]()
                                        {
            m_simulationTime = 0.0f;
            resetTestData();
            std::cout << "View reset" << std::endl; });

        // 创建StatusPanel - 展示比例调整优化
        m_statusPanel = std::make_unique<StatusPanel>(m_font);
        m_statusPanel->resize(WINDOW_HEIGHT - TOOLBAR_HEIGHT);

        // 创建VehicleInfoPanel - 展示新功能
        float panelHeight = (WINDOW_HEIGHT - TOOLBAR_HEIGHT) * 0.6f; // 占60%高度
        m_vehicleInfoPanel = std::make_unique<VehicleInfoPanel>(m_font, PANEL_WIDTH, panelHeight);
    }

    void createTestData()
    {
        // 创建测试车辆
        m_testVehicles.resize(3);

        for (size_t i = 0; i < m_testVehicles.size(); ++i)
        {
            Vehicle &vehicle = m_testVehicles[i];
            vehicle.id = static_cast<int>(i + 1);
            vehicle.m_state.position = i * 100.0f; // 分散位置
            vehicle.m_state.currentSpeed = 0.0f;
            vehicle.m_state.motionState = Vehicle::MotionState::Stopped;
            vehicle.m_state.operationTimer = 0.0f;
        }

        // 设置StatusPanel初始状态
        m_statusPanel->setVehicleCount(m_testVehicles.size());
        m_statusPanel->setCompletedTaskCount(5);
        m_statusPanel->setPendingTaskCount(12);

        // 选择第一辆车进行详细显示
        if (!m_testVehicles.empty())
        {
            m_vehicleInfoPanel->setVehicle(&m_testVehicles[0]);
        }
    }

    void resetTestData()
    {
        for (auto &vehicle : m_testVehicles)
        {
            vehicle.m_state.currentSpeed = 0.0f;
            vehicle.m_state.motionState = Vehicle::MotionState::Stopped;
        }

        m_vehicleInfoPanel->clearHistory();
        m_statusPanel->setSimulationTime(0.0f);
    }

    void simulateVehicleMovement(float deltaTime)
    {
        if (!m_isPlaying)
            return;

        // 模拟第一辆车的运动
        Vehicle &vehicle = m_testVehicles[0];
        static float motionTimer = 0.0f;
        static Vehicle::MotionState lastState = Vehicle::MotionState::Stopped;

        motionTimer += deltaTime * m_timeScale;

        // 简单的运动状态机
        if (motionTimer < 3.0f)
        {
            // 加速阶段
            vehicle.m_state.motionState = Vehicle::MotionState::Accelerating;
            vehicle.m_state.currentSpeed = motionTimer * 1.0f; // 1 m/s²加速度
        }
        else if (motionTimer < 8.0f)
        {
            // 匀速阶段
            vehicle.m_state.motionState = Vehicle::MotionState::Cruising;
            vehicle.m_state.currentSpeed = 3.0f; // 保持3 m/s
        }
        else if (motionTimer < 11.0f)
        {
            // 减速阶段
            vehicle.m_state.motionState = Vehicle::MotionState::Decelerating;
            vehicle.m_state.currentSpeed = 3.0f - (motionTimer - 8.0f) * 1.0f;
        }
        else
        {
            // 停止阶段
            vehicle.m_state.motionState = Vehicle::MotionState::Stopped;
            vehicle.m_state.currentSpeed = 0.0f;
            if (motionTimer > 13.0f)
            {
                motionTimer = 0.0f; // 重新开始循环
            }
        }

        // 记录状态变化事件
        if (lastState != vehicle.m_state.motionState)
        {
            std::string stateStr;
            switch (vehicle.m_state.motionState)
            {
            case Vehicle::MotionState::Accelerating:
                stateStr = "Accelerating";
                break;
            case Vehicle::MotionState::Cruising:
                stateStr = "Cruising";
                break;
            case Vehicle::MotionState::Decelerating:
                stateStr = "Decelerating";
                break;
            case Vehicle::MotionState::Stopped:
                stateStr = "Stopped";
                break;
            }

            // 记录加减速事件（简化版）
            if (vehicle.m_state.motionState == Vehicle::MotionState::Accelerating)
            {
                m_vehicleInfoPanel->recordAccelerationEvent(
                    m_simulationTime - deltaTime, m_simulationTime,
                    0.0f, vehicle.m_state.currentSpeed,
                    1.0f, "Accelerating");
            }

            lastState = vehicle.m_state.motionState;
        }

        // 更新位置
        vehicle.m_state.position += vehicle.m_state.currentSpeed * deltaTime * m_timeScale;

        // 更新VehicleInfoPanel统计信息
        m_vehicleInfoPanel->updateStatistics(deltaTime * m_timeScale);
    }

    void run()
    {
        sf::Clock clock;

        while (m_window.isOpen())
        {
            float deltaTime = clock.restart().asSeconds();

            // 更新仿真时间
            if (m_isPlaying)
            {
                m_simulationTime += deltaTime * m_timeScale;
            }

            handleEvents();
            update(deltaTime);
            render();
        }
    }

private:
    void handleEvents()
    {
        sf::Event event;
        while (m_window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                m_window.close();
            }

            sf::Vector2f mousePos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(m_window));

            // Toolbar事件处理
            if (mousePos.y <= TOOLBAR_HEIGHT)
            {
                m_toolbar->handleEvent(event, mousePos);
            }

            // StatusPanel事件处理
            sf::FloatRect statusPanelArea(WINDOW_WIDTH - PANEL_WIDTH, TOOLBAR_HEIGHT,
                                          PANEL_WIDTH, WINDOW_HEIGHT - TOOLBAR_HEIGHT);
            if (statusPanelArea.contains(mousePos))
            {
                sf::Vector2f localPos = mousePos - sf::Vector2f(statusPanelArea.left, statusPanelArea.top);
                m_statusPanel->handleEvent(event, localPos);
            }

            // VehicleInfoPanel事件处理
            sf::FloatRect vehiclePanelArea(10, TOOLBAR_HEIGHT + 10, PANEL_WIDTH,
                                           (WINDOW_HEIGHT - TOOLBAR_HEIGHT) * 0.6f);
            if (vehiclePanelArea.contains(mousePos))
            {
                sf::Vector2f localPos = mousePos - sf::Vector2f(vehiclePanelArea.left, vehiclePanelArea.top);
                m_vehicleInfoPanel->handleEvent(event, localPos);
            }

            // 键盘事件
            if (event.type == sf::Event::KeyPressed)
            {
                switch (event.key.code)
                {
                case sf::Keyboard::Space:
                    m_isPlaying = !m_isPlaying;
                    m_toolbar->updatePlayPauseState(m_isPlaying);
                    break;
                case sf::Keyboard::R:
                    m_simulationTime = 0.0f;
                    resetTestData();
                    break;
                case sf::Keyboard::Num1:
                case sf::Keyboard::Num2:
                case sf::Keyboard::Num3:
                    // 切换选中的车辆
                    int vehicleIndex = event.key.code - sf::Keyboard::Num1;
                    if (vehicleIndex < static_cast<int>(m_testVehicles.size()))
                    {
                        m_vehicleInfoPanel->setVehicle(&m_testVehicles[vehicleIndex]);
                        std::cout << "Selected vehicle " << (vehicleIndex + 1) << std::endl;
                    }
                    break;
                }
            }
        }
    }

    void update(float deltaTime)
    {
        // 更新仿真
        simulateVehicleMovement(deltaTime);

        // 更新GUI组件
        m_toolbar->updateTimeDisplay(m_simulationTime);
        m_statusPanel->setSimulationTime(m_simulationTime);
        m_vehicleInfoPanel->updateInfo(m_simulationTime);
    }

    void render()
    {
        m_window.clear(sf::Color(240, 248, 255)); // 爱丽丝蓝背景

        // 绘制Toolbar
        m_toolbar->render(m_window, sf::Vector2f(0, 0));

        // 绘制StatusPanel
        sf::Vector2f statusPanelPos(WINDOW_WIDTH - PANEL_WIDTH, TOOLBAR_HEIGHT);
        m_statusPanel->render(m_window, statusPanelPos);

        // 绘制VehicleInfoPanel
        sf::Vector2f vehiclePanelPos(10, TOOLBAR_HEIGHT + 10);
        m_vehicleInfoPanel->setPosition(vehiclePanelPos);
        m_window.draw(*m_vehicleInfoPanel);

        // 绘制中央信息区域
        drawCentralInfo();

        // 绘制说明文字
        drawInstructions();

        m_window.display();
    }

    void drawCentralInfo()
    {
        // 绘制中央区域的信息展示
        sf::Text titleText("GUI Phase 1 Optimization Demo", m_font, 24);
        titleText.setFillColor(sf::Color(50, 50, 50));
        titleText.setStyle(sf::Text::Bold);
        titleText.setPosition(PANEL_WIDTH + 50, TOOLBAR_HEIGHT + 30);
        m_window.draw(titleText);

        // 功能说明
        std::vector<std::string> features = {
            "✓ Toolbar Time Display: HH:MM:SS.mmm format",
            "✓ StatusPanel Layout: 25:35:40 proportions",
            "✓ Enhanced Dividers: Thicker, darker lines",
            "✓ VehicleInfoPanel: Motion analysis & statistics",
            "✓ UIUtils: Unified formatting functions"};

        float yPos = TOOLBAR_HEIGHT + 80;
        for (const auto &feature : features)
        {
            sf::Text featureText(feature, m_font, 14);
            featureText.setFillColor(sf::Color(70, 130, 70));
            featureText.setPosition(PANEL_WIDTH + 70, yPos);
            m_window.draw(featureText);
            yPos += 25;
        }

        // 车辆状态可视化
        yPos += 30;
        sf::Text vehicleTitle("Vehicle Status Visualization:", m_font, 16);
        vehicleTitle.setFillColor(sf::Color(50, 50, 50));
        vehicleTitle.setStyle(sf::Text::Bold);
        vehicleTitle.setPosition(PANEL_WIDTH + 50, yPos);
        m_window.draw(vehicleTitle);

        yPos += 30;
        for (size_t i = 0; i < m_testVehicles.size(); ++i)
        {
            const Vehicle &vehicle = m_testVehicles[i];

            // 绘制车辆矩形
            sf::RectangleShape vehicleRect(sf::Vector2f(40, 20));
            vehicleRect.setPosition(PANEL_WIDTH + 70 + i * 60, yPos);

            // 根据运动状态设置颜色
            sf::Color vehicleColor;
            switch (vehicle.m_state.motionState)
            {
            case Vehicle::MotionState::Accelerating:
                vehicleColor = sf::Color(100, 255, 100); // 绿色
                break;
            case Vehicle::MotionState::Cruising:
                vehicleColor = sf::Color(100, 100, 255); // 蓝色
                break;
            case Vehicle::MotionState::Decelerating:
                vehicleColor = sf::Color(255, 200, 100); // 橙色
                break;
            case Vehicle::MotionState::Stopped:
                vehicleColor = sf::Color(200, 200, 200); // 灰色
                break;
            }
            vehicleRect.setFillColor(vehicleColor);
            vehicleRect.setOutlineThickness(1);
            vehicleRect.setOutlineColor(sf::Color::Black);
            m_window.draw(vehicleRect);

            // 车辆ID
            sf::Text idText(std::to_string(vehicle.id), m_font, 12);
            idText.setFillColor(sf::Color::Black);
            idText.setPosition(PANEL_WIDTH + 85 + i * 60, yPos + 25);
            m_window.draw(idText);

            // 速度信息
            sf::Text speedText(std::to_string(static_cast<int>(vehicle.m_state.currentSpeed * 10) / 10.0f) + " m/s",
                               m_font, 10);
            speedText.setFillColor(sf::Color(70, 70, 70));
            speedText.setPosition(PANEL_WIDTH + 70 + i * 60, yPos + 40);
            m_window.draw(speedText);
        }
    }

    void drawInstructions()
    {
        std::vector<std::string> instructions = {
            "Controls:",
            "SPACE - Play/Pause simulation",
            "R - Reset simulation",
            "1/2/3 - Select vehicle for detailed view",
            "Mouse - Interact with GUI components"};

        float yPos = WINDOW_HEIGHT - 120;
        for (const auto &instruction : instructions)
        {
            sf::Text instrText(instruction, m_font, 12);
            instrText.setFillColor(sf::Color(100, 100, 100));
            if (instruction == "Controls:")
            {
                instrText.setStyle(sf::Text::Bold);
                instrText.setFillColor(sf::Color(70, 70, 70));
            }
            instrText.setPosition(PANEL_WIDTH + 50, yPos);
            m_window.draw(instrText);
            yPos += 18;
        }
    }
};

int main()
{
    try
    {
        GUI_Phase1_Demo demo;
        demo.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}
