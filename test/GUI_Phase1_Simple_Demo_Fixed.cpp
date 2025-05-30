#include <SFML/Graphics.hpp>
#include <iostream>
#include <memory>
#include <vector>
#define _USE_MATH_DEFINES
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// GUI组件包含
#include "gui/Toolbar.hpp"
#include "gui/StatusPanel.hpp"
#include "gui/VehicleInfoPanel.hpp"
#include "gui/UIControls.hpp"
#include "gui/SimulationView.hpp"

// Core组件包含
#include "../src/Core/Vehicle.hpp"
#include "../src/Core/Device.hpp"

/**
 * @brief GUI优化第一阶段增强演示测试
 *
 * 展示内容：
 * 1. Toolbar时间显示优化 - HH:MM:SS.mmm格式
 * 2. StatusPanel比例调整 - 25:35:40布局
 * 3. VehicleInfoPanel新组件 - 车辆加减速信息显示
 * 4. UIControls工具函数 - 时间格式化和颜色映射
 * 5. SimulationView - 仓库渲染和可控制车辆系统
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
    std::unique_ptr<SimulationView> simulationView;
    
    // 仿真数据
    std::unique_ptr<Vehicle> vehicle = std::make_unique<Vehicle>();
    std::vector<std::unique_ptr<Vehicle>> vehicles;
    std::vector<std::unique_ptr<DeviceBase>> devices;

    // 仿真状态
    float simulationTime = 0.0f;
    bool isRunning = false;
    int selectedVehicleId = -1; 
    
    // 布局参数
    static constexpr float TOOLBAR_HEIGHT = 50.0f;
    static constexpr float STATUS_PANEL_WIDTH = 350.0f;
    static constexpr float SIMULATION_VIEW_MARGIN = 10.0f;

public:
    SimpleDemoApp() : window(sf::VideoMode(1800, 800), "GUI Phase 1 - Enhanced Demo with Warehouse & Vehicles")
    {
        loadFont();
        initializeSimulationData();
        initializeComponents();
    }

    bool loadFont()
    {
        // 尝试多个字体路径
        std::vector<std::string> fontPaths = {
            "C:/Windows/Fonts/COOPBL.TTF",
            "C:/Windows/Fonts/arial.ttf",
            "C:/Windows/Fonts/calibri.ttf",
            "C:/Windows/Fonts/tahoma.ttf",
            "C:/Windows/Fonts/verdana.ttf",
            "assets/fonts/arial.ttf"
        };

        for (const auto &fontPath : fontPaths)
        {
            if (font.loadFromFile(fontPath))
            {
                std::cout << "Font loaded successfully: " << fontPath << std::endl;
                return true;
            }
        }

        std::cerr << "Failed to load any font! UI text will not display correctly." << std::endl;
        return false;
    }
    
    void initializeSimulationData()
    {
        // 按照VehiclePathPositionTest.cpp的方式创建一辆车辆，展示完整的轨道和车辆渲染
        vehicle->id = 1;
        vehicle->position_m = 32.0f; // 参考实现的起始位置
        vehicle->velocity_mps = 0.0f;
        vehicle->max_speed = 2.0f;
        vehicle->is_loaded = false;
        vehicle->is_executing = false;
        vehicle->towards_device = -1;
        vehicle->next_available_time = 0.0f;

        // 初始化状态
        vehicle->m_state.position = vehicle->position_m;
        vehicle->m_state.currentSpeed = 0.0f;
        vehicle->m_state.motionState = Vehicle::MotionState::Stopped;
        vehicle->m_state.currentTask = nullptr;
        vehicle->m_state.operationTimer = 0.0f;
    }
    
    void initializeComponents()
    {
        // 创建工具栏 - 修正参数顺序：(font, width, height)
        toolbar = std::make_unique<Toolbar>(font, window.getSize().x, TOOLBAR_HEIGHT);

        // 创建状态面板
        statusPanel = std::make_unique<StatusPanel>(font);
        statusPanel->resize(window.getSize().y - TOOLBAR_HEIGHT);

        // 创建车辆信息面板
        vehicleInfoPanel = std::make_unique<VehicleInfoPanel>(font, STATUS_PANEL_WIDTH, 400.0f);

        // 创建仿真视图 - 使用正确的构造函数
        simulationView = std::make_unique<SimulationView>(font);

        // 初始化仿真视图 - 使用参考实现的参数设置
        float simulationViewWidth = window.getSize().x - STATUS_PANEL_WIDTH - SIMULATION_VIEW_MARGIN * 3;
        float simulationViewHeight = window.getSize().y - TOOLBAR_HEIGHT - SIMULATION_VIEW_MARGIN * 2;
        sf::Vector2f simulationViewSize(simulationViewWidth, simulationViewHeight);

        // 使用与VehiclePathPositionTest.cpp相同的初始化方法
        simulationView->initialize(font, nullptr, simulationViewSize); 
        
        // 设置显示选项（与测试文件一致）
        simulationView->setShowGrid(false);                            // 默认关闭网格
        simulationView->setShowWarehouses(true);                       // 显示仓库
        simulationView->setShowVehicles(true);                         // 显示车辆
        simulationView->setShowDebugInfo(false);                       // 默认关闭调试信息

        // 正确的做法：通过 SimulationView 的 updateVehicles 方法更新车辆数据
        std::vector<Vehicle *> vehiclePtrs;
        vehiclePtrs.push_back(vehicle.get());
        simulationView->updateVehicles(vehiclePtrs);

        // 更新设备数据（当前为空）
        std::vector<DeviceBase *> devicePtrs;
        simulationView->updateDevices(devicePtrs);

        // 设置工具栏回调
        toolbar->setOnPlayPauseToggled([this]()
                                       {
            isRunning = !isRunning;
            std::cout << "Simulation State: " << (isRunning ? "Running" : "Paused") << std::endl; });

        toolbar->setOnTimeScaleChanged([this](float speed)
                                       { std::cout << "Speed adjusted to: " << speed << "x" << std::endl; });

        // 设置初始状态
        statusPanel->setSimulationTime(0.0f);
        statusPanel->setVehicleCount(1);
        statusPanel->setCompletedTaskCount(0);
        statusPanel->setPendingTaskCount(5);
    }
    
    void handleEvents()
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
            if (toolbar->handleEvent(event, mousePos))
            {
                continue; // 工具栏消费了事件，跳过其他处理
            } 
            
            // 仿真视图事件处理 - 只在仿真区域内处理事件
            if (simulationView)
            {
                // 检查鼠标是否在仿真视图区域内
                float simulationViewLeft = SIMULATION_VIEW_MARGIN;
                float simulationViewTop = TOOLBAR_HEIGHT + SIMULATION_VIEW_MARGIN;
                float simulationViewWidth = window.getSize().x - STATUS_PANEL_WIDTH - SIMULATION_VIEW_MARGIN * 3;
                float simulationViewHeight = window.getSize().y - TOOLBAR_HEIGHT - SIMULATION_VIEW_MARGIN * 2;

                bool inSimulationArea = (mousePos.x >= simulationViewLeft &&
                                         mousePos.x <= simulationViewLeft + simulationViewWidth &&
                                         mousePos.y >= simulationViewTop &&
                                         mousePos.y <= simulationViewTop + simulationViewHeight);

                // 只有在仿真区域内才处理仿真视图事件
                if (inSimulationArea)
                {
                    // 处理仿真视图事件
                    simulationView->handleViewEvent(event, mousePos);
                    
                    // 检查是否有车辆被选中 - 使用现有方法
                    auto selectedVehicle = simulationView->getSelectedObject();
                    if (selectedVehicle)
                    {
                        int newSelectedId = selectedVehicle->id;
                        if (newSelectedId != selectedVehicleId)
                        {
                            selectedVehicleId = newSelectedId;
                            std::cout << "Selected vehicle ID: " << selectedVehicleId << std::endl;
                        }
                    }
                    else if (selectedVehicleId != -1)
                    {
                        selectedVehicleId = -1;
                        std::cout << "Deselected vehicle" << std::endl;
                    }
                    continue; // 仿真视图消费了事件，跳过其他处理
                }
            }

            if (event.type == sf::Event::KeyPressed)
            {
                switch (event.key.code)
                {
                case sf::Keyboard::Space:
                    isRunning = !isRunning;
                    toolbar->setPlaying(isRunning);
                    std::cout << "Simulation State: " << (isRunning ? "Running" : "Paused") << std::endl;
                    break;
                case sf::Keyboard::R:
                    simulationTime = 0.0f;
                    isRunning = false;
                    toolbar->setPlaying(false);
                    resetSimulation();
                    std::cout << "Reset Simulation" << std::endl;
                    break;
                case sf::Keyboard::Escape:
                    window.close();
                    break;

                // 显示控制键（与VehiclePathPositionTest.cpp一致）
                case sf::Keyboard::T:
                    if (simulationView)
                    {
                        bool currentState = simulationView->getShowGrid();
                        simulationView->setShowGrid(!currentState);
                        std::cout << (currentState ? "隐藏坐标网格\n" : "显示坐标网格\n");
                    }
                    break;
                case sf::Keyboard::G:
                    if (simulationView)
                    {
                        bool currentState = simulationView->getShowWarehouses();
                        simulationView->setShowWarehouses(!currentState);
                        std::cout << (currentState ? "隐藏仓库\n" : "显示仓库\n");
                    }
                    break;
                case sf::Keyboard::V:
                    if (simulationView)
                    {
                        bool currentState = simulationView->getShowVehicles();
                        simulationView->setShowVehicles(!currentState);
                        std::cout << (currentState ? "隐藏车辆\n" : "显示车辆\n");
                    }
                    break;
                case sf::Keyboard::C:
                    if (simulationView)
                    {
                        bool currentState = simulationView->getShowDebugInfo();
                        simulationView->setShowDebugInfo(!currentState);
                        std::cout << (currentState ? "隐藏调试信息\n" : "显示调试信息\n");
                    }
                    break;

                // 车辆控制键
                case sf::Keyboard::Num1:
                case sf::Keyboard::Num2:
                case sf::Keyboard::Num3:
                {
                    int vehicleId = event.key.code - sf::Keyboard::Num0;
                    selectedVehicleId = vehicleId;
                    std::cout << "Selected vehicle " << vehicleId << " via keyboard" << std::endl;
                }
                break;
                case sf::Keyboard::W:
                case sf::Keyboard::A:
                case sf::Keyboard::S:
                case sf::Keyboard::D:
                    moveSelectedVehicle(event.key.code);
                    break;
                }
            }
        }
    }
    
    void update(float deltaTime)
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
            
            // 更新仿真视图
            if (simulationView)
            {
                simulationView->updateViewTransforms(deltaTime * timeScale);
            }

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
        std::string eventType = (eventCounter % 2 == 0) ? "Acceleration" : "Deceleration";

        vehicleInfoPanel->recordAccelerationEvent(
            startTime, endTime, startSpeed, endSpeed, acceleration, eventType);

        std::cout << "Record " << eventType << " event: "
                  << startSpeed << "m/s -> " << endSpeed << "m/s" << std::endl;
    }
    
    void moveSelectedVehicle(sf::Keyboard::Key key)
    {
        // 现在我们只有一个车辆对象，直接操作它
        float currentPos = vehicle->position_m;
        float newPos = currentPos;

        const float moveSpeed = 5.0f; // 移动速度（米）- 减小步长以便精细控制

        switch (key)
        {
        case sf::Keyboard::W:
            newPos += moveSpeed; // 向前移动
            break;
        case sf::Keyboard::S:
            newPos -= moveSpeed; // 向后移动
            break;
        case sf::Keyboard::A:
            newPos -= moveSpeed; // 向左（后退）
            break;
        case sf::Keyboard::D:
            newPos += moveSpeed; // 向右（前进）
            break;
        default:
            return;
        }

        // 边界检查（保持在轨道范围内）
        // 获取轨道的实际总长度（毫米转米）
        float maxTrackLengthMm = 2 * 40000.0f + 2 * M_PI * 2500.0f; // ≈ 95700mm
        float maxTrackLength = maxTrackLengthMm / 1000.0f;          // 转换为米，≈95.7米

        if (newPos >= 0.0f && newPos <= maxTrackLength)
        {
            vehicle->position_m = newPos;
            vehicle->m_state.position = newPos;
            vehicle->m_state.motionState = Vehicle::MotionState::Cruising;

            std::cout << "Vehicle moved to position: " << newPos << "m" << std::endl;

            // 更新仿真视图中的车辆数据
            if (simulationView)
            {
                std::vector<Vehicle *> vehiclePtrs;
                vehiclePtrs.push_back(vehicle.get());
                simulationView->updateVehicles(vehiclePtrs);
            }
        }
    }
    
    void resetSimulation()
    {
        // 重置车辆位置和状态
        vehicle->position_m = 32.0f; // 使用参考实现的起始位置
        vehicle->velocity_mps = 0.0f;
        vehicle->is_loaded = false;
        vehicle->m_state.position = vehicle->position_m;
        vehicle->m_state.motionState = Vehicle::MotionState::Stopped;

        // 清除选择
        selectedVehicleId = -1;

        // 更新仿真视图数据
        if (simulationView)
        {
            std::vector<Vehicle *> vehiclePtrs;
            vehiclePtrs.push_back(vehicle.get());
            simulationView->updateVehicles(vehiclePtrs);

            // 设备重置暂时跳过，因为我们没有创建设备
            std::vector<DeviceBase *> devicePtrs;
            simulationView->updateDevices(devicePtrs);
        }

        // 重置统计
        statusPanel->setCompletedTaskCount(0);
        statusPanel->setPendingTaskCount(5);
        std::cout << "Simulation reset" << std::endl;
    }
    
    void render()
    {
        window.clear(sf::Color(245, 245, 245)); // 浅灰色背景

        // 首先渲染仓库仿真视图（背景层）
        if (simulationView)
        {
            // 设置仿真视图的视口（排除工具栏和状态面板区域）
            float simulationViewLeft = SIMULATION_VIEW_MARGIN;
            float simulationViewTop = TOOLBAR_HEIGHT + SIMULATION_VIEW_MARGIN;
            float simulationViewWidth = window.getSize().x - STATUS_PANEL_WIDTH - SIMULATION_VIEW_MARGIN * 3;
            float simulationViewHeight = window.getSize().y - TOOLBAR_HEIGHT - SIMULATION_VIEW_MARGIN * 2;

            sf::FloatRect simulationViewport(
                simulationViewLeft / window.getSize().x,
                simulationViewTop / window.getSize().y,
                simulationViewWidth / window.getSize().x,
                simulationViewHeight / window.getSize().y);

            simulationView->updateViewport(simulationViewport);
            simulationView->renderWorld(window);
        }

        // 然后渲染GUI组件（顶层）- 确保在SimulationView之后渲染

        // 渲染工具栏（顶部）
        toolbar->render(window, sf::Vector2f(0, 0));

        // 渲染状态面板（右侧）
        sf::Vector2f statusPanelPos(window.getSize().x - STATUS_PANEL_WIDTH, TOOLBAR_HEIGHT);
        statusPanel->render(window, statusPanelPos);

        // 渲染车辆信息面板（左下角）
        sf::Vector2f vehicleInfoPos(10, TOOLBAR_HEIGHT + 10);
        vehicleInfoPanel->setPosition(vehicleInfoPos);
        window.draw(*vehicleInfoPanel);

        // 渲染演示说明文字（最顶层）
        renderInstructions();

        window.display();
    }

    void renderInstructions()
    {
        sf::Text instructions;
        instructions.setFont(font);
        instructions.setCharacterSize(14);
        instructions.setFillColor(sf::Color::Black);
        instructions.setPosition(10, window.getSize().y - 150);
        std::string text = "GUI Phase 1 Enhanced Demo - Warehouse & Vehicles\n"
                           "=== 仿真控制 ===\n"
                           "Space: 开始/暂停仿真\n"
                           "R: 重置仿真\n"
                           "ESC: 退出程序\n"
                           "\n=== 显示控制 ===\n"
                           "T: 切换坐标网格显示\n"
                           "G: 切换仓库显示\n"
                           "V: 切换车辆显示\n"
                           "C: 切换调试信息显示\n"
                           "\n=== 车辆控制 ===\n"
                           "1/2/3: 选择车辆(按编号)\n"
                           "WASD: 移动选中的车辆\n"
                           "鼠标: 点击选择车辆/设备\n"
                           "\n=== GUI组件功能 ===\n"
                           "• 工具栏: 时间显示(HH:MM:SS.mmm)\n"
                           "• 状态面板: 仿真统计信息\n"
                           "• 车辆信息面板: 加减速事件记录";
        instructions.setString(text);

        window.draw(instructions);
    }

    void run()
    {
        sf::Clock clock;
        std::cout << "GUI Phase 1 Optimization Demo Started" << std::endl;
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
