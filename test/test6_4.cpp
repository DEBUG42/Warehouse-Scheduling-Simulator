
#include <SFML/Graphics.hpp>
#include <iostream>
#include <memory>
#include <vector>
#include <map>
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
#include "Core/Vehicle.hpp"
#include "Core/Task.hpp"
#include "Core/Device.hpp"
#include "Core/Logger.hpp"
#include "Core/Scheduler.hpp"
// task1用的文件
#include <chrono>
#include <thread>
#include <cstdlib>
#include <ctime>
// 前置声明updateVehicle1函数
void updateVehicle1(float current_time, float deltaTime, Vehicle *vehicle, Vehicle *leadingVehicle);

/**
 * @brief GUI优化第一阶段增强演示测试
 *
 * 展示内容：
 * 1. Toolbar时间显示优化 - HH:MM:SS.mmm格式
 * 2. StatusPanel比例调整 - 25:35:40布局s
 * 3. VehicleInfoPanel新组件 - 车辆加减速信息显示
 * 4. UIControls工具函数 - 时间格式化和颜色映射
 * 5. SimulationView - 仓库渲染和可控制车辆系统
 */

class SimpleDemoApp
{
public:
    sf::RenderWindow window;
    sf::Font font;                      // GUI组件
    Scheduler *scheduler_ptr = nullptr; // 改为指针，引用外部scheduler

    std::unique_ptr<StatusPanel> statusPanel;
    std::unique_ptr<VehicleInfoPanel> vehicleInfoPanel;
    std::unique_ptr<SimulationView> simulationView;

    // 删除重复的车辆存储，使用scheduler中的
    // std::vector<std::unique_ptr<Vehicle>> vehicles;
    // std::vector<std::unique_ptr<DeviceBase>> devices;

    // 仿真状态
    float simulationTime = 0.0f;
    bool isRunning = false;
    int selectedVehicleId = -1;

    // 车辆状态跟踪（用于检测状态变化和记录加减速事件）
    struct VehicleStateTracker
    {
        Vehicle::MotionState lastMotionState = Vehicle::MotionState::Stopped;
        float lastSpeed = 0.0f;
        float stateChangeTime = 0.0f;
        bool hasStateChanged = false;
    };
    std::map<int, VehicleStateTracker> vehicleTrackers; // 布局参数
    static constexpr float TOOLBAR_HEIGHT = 50.0f;
    static constexpr float STATUS_PANEL_WIDTH = 290.0f;
    static constexpr float SIMULATION_VIEW_MARGIN = 10.0f;
    static constexpr float VEHICLE_INFO_PANEL_WIDTH = 350.0f;  // 车辆信息面板宽度
    static constexpr float VEHICLE_INFO_PANEL_HEIGHT = 415.0f; // 车辆信息面板高度

public:
    SimulationMode m_mode = SimulationMode::TASK1;
    std::unique_ptr<Toolbar> toolbar;
    SimpleDemoApp() : window(sf::VideoMode(1800, 800), "GUI Phase 1 - Enhanced Demo with Warehouse & Vehicles")
    {
        loadFont();
        // initializeSimulationData(); // 删除，不再创建车辆
        initializeComponents();
    }

    // 设置scheduler的方法
    void setScheduler(Scheduler *scheduler)
    {
        scheduler_ptr = scheduler;
        // 重新初始化依赖scheduler的组件
        updateSchedulerDependentComponents();
    }

    // 公共方法：更新GUI显示（供外部调用）
    void refreshGUI()
    {
        updateSchedulerDependentComponents();
    }
    bool loadFont()
    {
        // 尝试多个字体路径 - 优先英文字体
        std::vector<std::string> fontPaths = {
            "C:/Windows/Fonts/COOPBL.TTF",
            "C:/Windows/Fonts/arial.ttf",
            "C:/Windows/Fonts/calibri.ttf",
            "C:/Windows/Fonts/tahoma.ttf",
            "C:/Windows/Fonts/verdana.ttf",
            "assets/fonts/arial.ttf"};

        for (const auto &fontPath : fontPaths)
        {
            if (font.loadFromFile(fontPath))
            {
                std::cout << "Font loaded successfully: " << fontPath << std::endl;
                return true;
            }
        }

        std::cerr << "Cannot load any font! UI text may not display correctly." << std::endl;
        return false;
    }

    void initializeComponents()
    {
        // 创建工具栏 - 修正参数顺序：(font, width, height)
        toolbar = std::make_unique<Toolbar>(font, window.getSize().x, TOOLBAR_HEIGHT);

        // 设置默认模式为TASK1
        toolbar->setCurrentMode(SimulationMode::TASK1);

        // 创建状态面板
        statusPanel = std::make_unique<StatusPanel>(font);
        statusPanel->resize(window.getSize().y - 10.0f);

        // 创建车辆信息面板
        vehicleInfoPanel = std::make_unique<VehicleInfoPanel>(font, VEHICLE_INFO_PANEL_WIDTH, VEHICLE_INFO_PANEL_HEIGHT);

        // 创建仿真视图 - 使用正确的构造函数
        simulationView = std::make_unique<SimulationView>(font);

        // 初始化仿真视图 - 使用参考实现的参数设置
        float simulationViewWidth = window.getSize().x - STATUS_PANEL_WIDTH - SIMULATION_VIEW_MARGIN * 3;
        float simulationViewHeight = window.getSize().y - TOOLBAR_HEIGHT - SIMULATION_VIEW_MARGIN * 2;
        sf::Vector2f simulationViewSize(simulationViewWidth, simulationViewHeight);

        // 使用与VehiclePathPositionTest.cpp相同的初始化方法
        simulationView->initialize(font, nullptr, simulationViewSize);
        simulationView->setShowGrid(false);      // 默认关闭网格
        simulationView->setShowWarehouses(true); // 显示仓库
        simulationView->setShowVehicles(true);   // 显示车辆
        simulationView->setShowDebugInfo(false); // 默认关闭调试信息

        // 设置工具栏回调
        toolbar->setOnPlayPauseToggled([this]()
                                       {
            isRunning = !isRunning;
            std::cout << "Simulation State: " << (isRunning ? "Running" : "Paused") << std::endl; });

        toolbar->setOnTimeScaleChanged([this](float speed)
                                       { std::cout << "Speed adjusted to: " << speed << "x" << std::endl; });

        // 如果scheduler还未设置，暂时跳过依赖scheduler的初始化
        if (scheduler_ptr != nullptr)
        {
            updateSchedulerDependentComponents();
        }
    }

    // 更新依赖scheduler的组件
    void updateSchedulerDependentComponents()
    {
        if (scheduler_ptr == nullptr)
        {
            std::cout << "如果没有规划器指针，我设置的是直接跳过对她的依赖" << std::endl;
            return;
        }

        // 使用scheduler中的车辆数据（可能为空）
        auto &vehicles = scheduler_ptr->vehicle_manager_ptr->getAllVehicles();
        std::vector<Vehicle *> vehiclePtrs;
        for (auto &vehicle : vehicles)
        {
            vehiclePtrs.push_back(&vehicle);
        }
        simulationView->updateVehicles(vehiclePtrs);

        // 更新设备数据（当前为空）
        std::vector<DeviceBase *> devicePtrs;
        simulationView->updateDevices(devicePtrs);

        // 设置初始状态
        statusPanel->setSimulationTime(0.0f);
        statusPanel->setVehicleCount(vehicles.size()); // 可能为0
        statusPanel->setCompletedTaskCount(0);
        statusPanel->setPendingTaskCount(0); // 修改为0，因为没有预设任务

        std::cout << "前端工作已做好: 收到" << vehicles.size() << " 后端小车" << std::endl;
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

            // 1. 首先处理全局键盘事件（不受区域限制）
            if (event.type == sf::Event::KeyPressed)
            {
                switch (event.key.code)
                {
                case sf::Keyboard::Space:
                    isRunning = !isRunning;
                    toolbar->setPlaying(isRunning);
                    std::cout << "Simulation Status: " << (isRunning ? "Running" : "Paused") << std::endl;
                    continue; // 键盘事件已处理，跳过其他处理

                case sf::Keyboard::Escape:
                    window.close();
                    continue;

                // 显示控制键
                case sf::Keyboard::T:
                    if (simulationView)
                    {
                        bool currentState = simulationView->getShowGrid();
                        simulationView->setShowGrid(!currentState);
                        std::cout << (currentState ? "Hide coordinate grid\n" : "Show coordinate grid\n");
                    }
                    continue;
                case sf::Keyboard::G:
                    if (simulationView)
                    {
                        bool currentState = simulationView->getShowWarehouses();
                        simulationView->setShowWarehouses(!currentState);
                        std::cout << (currentState ? "Hide warehouse\n" : "Show warehouse\n");
                    }
                    continue;
                case sf::Keyboard::V:
                    if (simulationView)
                    {
                        bool currentState = simulationView->getShowVehicles();
                        simulationView->setShowVehicles(!currentState);
                        std::cout << (currentState ? "Hide vehicles\n" : "Show vehicles\n");
                    }
                    continue;
                case sf::Keyboard::C:
                    if (simulationView)
                    {
                        bool currentState = simulationView->getShowDebugInfo();
                        simulationView->setShowDebugInfo(!currentState);
                        std::cout << (currentState ? "Hide debug info\n" : "Show debug info\n");
                    }
                    continue; // 车辆控制键
                case sf::Keyboard::Num1:
                case sf::Keyboard::Num2:
                case sf::Keyboard::Num3:
                {
                    if (scheduler_ptr == nullptr)
                    {
                        std::cout << "Warning: Scheduler not set, cannot select vehicle" << std::endl;
                        continue;
                    }
                    int vehicleId = event.key.code - sf::Keyboard::Num0;
                    auto &vehicles = scheduler_ptr->vehicle_manager_ptr->getAllVehicles();
                    if (vehicles.empty())
                    {
                        std::cout << "No vehicles available from backend" << std::endl;
                        continue;
                    }
                    if (vehicleId > 0 && vehicleId <= vehicles.size())
                    {
                        selectedVehicleId = vehicleId;
                        std::cout << "Selected vehicle " << vehicleId << std::endl;
                    }
                }
                    continue;
                }
            }

            // 2. 工具栏事件处理（鼠标事件）
            if (toolbar->handleEvent(event, mousePos))
            {
                continue; // 工具栏消费了事件，跳过其他处理
            } // 仿真视图事件处理 - 只在仿真区域内处理事件
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
                                         mousePos.y <= simulationViewTop + simulationViewHeight); // 只有在仿真区域内才处理仿真视图事件
                if (inSimulationArea)
                {
                    // 处理仿真视图事件
                    simulationView->handleViewEvent(event, mousePos);

                    // 只有当发生鼠标点击事件时才检查车辆选择，避免键盘选择被覆盖
                    if (event.type == sf::Event::MouseButtonPressed)
                    {
                        auto selectedVehicle = simulationView->getSelectedObject();
                        if (selectedVehicle)
                        {
                            int newSelectedId = selectedVehicle->id;
                            if (newSelectedId != selectedVehicleId)
                            {
                                selectedVehicleId = newSelectedId;
                                std::cout << "Mouse selected vehicle ID: " << selectedVehicleId << std::endl;
                            }
                        }
                        else if (selectedVehicleId != -1)
                        {
                            selectedVehicleId = -1;
                            std::cout << "Mouse deselected vehicle" << std::endl;
                        }
                    }
                    continue; // 仿真视图消费了事件，跳过其他处理
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

            // 更新scheduler时间
            if (scheduler_ptr != nullptr)
            {
                scheduler_ptr->current_time = simulationTime;
            }

            // 更新工具栏时间显示
            toolbar->updateTimeDisplay(simulationTime);

            // 更新状态面板
            statusPanel->setSimulationTime(simulationTime);

            // 更新仿真视图
            if (simulationView)
            {
                simulationView->updateViewTransforms(deltaTime * timeScale);
            }

            // 检测车辆状态变化并记录加减速事件
            checkVehicleStateChanges();
        }
    }

    // 检测车辆状态变化并记录加减速事件
    void checkVehicleStateChanges()
    {
        if (scheduler_ptr == nullptr)
            return;

        auto &vehicles = scheduler_ptr->vehicle_manager_ptr->getAllVehicles();
        for (auto &vehicle : vehicles)
        {
            int vehicleId = vehicle.id;

            // 初始化跟踪器（如果不存在）
            if (vehicleTrackers.find(vehicleId) == vehicleTrackers.end())
            {
                vehicleTrackers[vehicleId] = VehicleStateTracker();
                vehicleTrackers[vehicleId].lastMotionState = vehicle.m_state.motionState;
                vehicleTrackers[vehicleId].lastSpeed = vehicle.m_state.currentSpeed;
                vehicleTrackers[vehicleId].stateChangeTime = simulationTime;
                continue;
            }

            auto &tracker = vehicleTrackers[vehicleId];

            // 检测状态变化
            if (tracker.lastMotionState != vehicle.m_state.motionState)
            {
                // 记录上一阶段的加减速事件
                if (tracker.hasStateChanged)
                { // 不是第一次状态变化
                    std::string eventType = "";
                    switch (tracker.lastMotionState)
                    {
                    case Vehicle::MotionState::Accelerating:
                        eventType = "Accelerating";
                        break;
                    case Vehicle::MotionState::Decelerating:
                        eventType = "Decelerating";
                        break;
                    case Vehicle::MotionState::Cruising:
                        eventType = "Cruising";
                        break;
                    case Vehicle::MotionState::Stopped:
                        eventType = "Stopped";
                        break;
                    }

                    if (!eventType.empty())
                    {
                        float acceleration = (vehicle.m_state.currentSpeed - tracker.lastSpeed) /
                                             (simulationTime - tracker.stateChangeTime);

                        vehicleInfoPanel->recordAccelerationEvent(
                            vehicleId,
                            tracker.stateChangeTime,
                            simulationTime,
                            tracker.lastSpeed,
                            vehicle.m_state.currentSpeed,
                            acceleration,
                            eventType);

                        std::cout << "Vehicle " << vehicleId << " " << eventType
                                  << ": " << tracker.lastSpeed << "m/s -> "
                                  << vehicle.m_state.currentSpeed << "m/s" << std::endl;
                    }
                }

                // 更新跟踪器
                tracker.lastMotionState = vehicle.m_state.motionState;
                tracker.lastSpeed = vehicle.m_state.currentSpeed;
                tracker.stateChangeTime = simulationTime;
                tracker.hasStateChanged = true;
            }
        }
    }

    void render()
    {
        window.clear(sf::Color(245, 245, 245)); // 浅灰色背景        // 首先渲染仓库仿真视图（背景层）
        if (simulationView)
        {
            // 设置仿真视图的视口（排除工具栏、状态面板和车辆信息面板区域）
            float simulationViewLeft = VEHICLE_INFO_PANEL_WIDTH + SIMULATION_VIEW_MARGIN * 2;
            float simulationViewTop = TOOLBAR_HEIGHT + SIMULATION_VIEW_MARGIN;
            float simulationViewWidth = window.getSize().x - STATUS_PANEL_WIDTH - VEHICLE_INFO_PANEL_WIDTH - SIMULATION_VIEW_MARGIN * 4;
            float simulationViewHeight = window.getSize().y - TOOLBAR_HEIGHT - SIMULATION_VIEW_MARGIN * 2;

            sf::FloatRect simulationViewport(
                simulationViewLeft / window.getSize().x,
                simulationViewTop / window.getSize().y,
                simulationViewWidth / window.getSize().x,
                simulationViewHeight / window.getSize().y);

            simulationView->updateViewport(simulationViewport);
            simulationView->renderWorld(window);
        }

        // 然后渲染GUI组件（顶层）- 确保在SimulationView之后渲染        // 渲染工具栏（顶部）
        toolbar->render(window, sf::Vector2f(0, 0));

        // 渲染状态面板（右侧，自动贴在右边缘）
        statusPanel->render(window, window.getSize());

        // 渲染车辆信息面板（左侧，不与轨道重叠）
        sf::Vector2f vehicleInfoPos(SIMULATION_VIEW_MARGIN, TOOLBAR_HEIGHT + SIMULATION_VIEW_MARGIN);
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
        std::string text = "GUI Frontend - Warehouse Visualization\n"
                           "=== Simulation Control ===\n"
                           "Space: Start/Pause simulation\n"
                           "ESC: Exit program\n"
                           "\n=== Display Control ===\n"
                           "T: Toggle coordinate grid display\n"
                           "G: Toggle warehouse display\n"
                           "V: Toggle vehicle display\n"
                           "C: Toggle debug info display\n"
                           "\n=== Vehicle Selection ===\n"
                           "1/2/3: Select vehicle (by number)\n"
                           "Mouse: Click to select vehicle/device\n"
                           "\n=== Current Status ===\n"
                           "• Warehouse: Visible\n"
                           "• Track: Visible\n"
                           "• Vehicles: Running in TASK1 mode\n"
                           "• Mode Switching: Available with time reset\n"
                           "\n=== GUI Components ===\n"
                           "• Toolbar: Time display (HH:MM:SS.mmm) & Mode switching\n"
                           "• Status Panel: Simulation statistics\n"
                           "• Vehicle Info Panel: Vehicle acceleration events\n"
                           "• Simulation View: Track & warehouse rendering\n"
                           "• Backend Communication: Integrated";
        instructions.setString(text);

        window.draw(instructions);
    }
    void run()
    {
        sf::Clock clock;

        std::cout << "GUI Frontend Started - Warehouse Visualization" << std::endl;
        std::cout << "Features:" << std::endl;
        std::cout << "1. Toolbar - Time format display (HH:MM:SS.mmm)" << std::endl;
        std::cout << "2. StatusPanel - 25:35:40 ratio layout" << std::endl;
        std::cout << "3. VehicleInfoPanel - Vehicle acceleration info recording" << std::endl;
        std::cout << "4. SimulationView - Track and warehouse rendering" << std::endl;
        std::cout << "5. Backend communication interface ready" << std::endl;
        std::cout << "\nFrontend ready for backend vehicle data..." << std::endl;
        std::cout << "\nInstructions: Press Space to start simulation when vehicles are loaded from backend" << std::endl;

        while (window.isOpen())
        {
            float deltaTime = clock.restart().asSeconds();
            // printf("Delta Time: %.3f seconds\n", deltaTime); // 输出每帧的时间间隔
            handleEvents();
            update(deltaTime);
            render();

            // 只有当有车辆时才执行车辆更新逻辑
            if (isRunning && m_mode == SimulationMode::TASK1 && scheduler_ptr != nullptr)
            {
                auto &vehicles = scheduler_ptr->vehicle_manager_ptr->getAllVehicles();
                if (vehicles.size() >= 3)
                { // 确保有足够的车辆
                    float timeScale = toolbar->getCurrentSpeed();
                    float intPart, fractionalPart;
                    fractionalPart = std::modf(timeScale, &intPart);

                    // 整数部分 - 完整循环
                    for (int i = 0; i < static_cast<int>(intPart); i++)
                    {
                        updateVehicle1(scheduler_ptr->current_time, deltaTime, &vehicles[0], &vehicles[2]);
                        updateVehicle1(scheduler_ptr->current_time, deltaTime, &vehicles[1], &vehicles[0]);
                        updateVehicle1(scheduler_ptr->current_time, deltaTime, &vehicles[2], &vehicles[1]);
                    }

                    // 小数部分 - 通过deltaTime实现
                    if (fractionalPart > 0.001f)
                    {
                        float adjustedDeltaTime = deltaTime * fractionalPart;
                        updateVehicle1(scheduler_ptr->current_time, adjustedDeltaTime, &vehicles[0], &vehicles[2]);
                        updateVehicle1(scheduler_ptr->current_time, adjustedDeltaTime, &vehicles[1], &vehicles[0]);
                        updateVehicle1(scheduler_ptr->current_time, adjustedDeltaTime, &vehicles[2], &vehicles[1]);
                    }
                }
            }
        }
    }
};

int main()
{
    // Scheduler scheduler;
    EventQueue event_queue;         // 事件队列
    TaskManager task_manager;       // 任务管理器
    VehicleManager vehicle_manager; // 车辆管理器
    DeviceManager device_manager;   // 设备管理器
    Logger logger;                  // 日志记录器

    // scheduler.bind(&task_manager, &vehicle_manager, &device_manager, &event_queue, &logger);

    SimpleDemoApp app; // 设置好了，不需要传递scheduler指针
    // app.setScheduler(&scheduler); // 通过setScheduler方法设置scheduler
    app.toolbar->setOnModeChanged([&app, &scheduler](SimulationMode mode)
                                  {
                                      // 切换任务时重置仿真时间并暂停
                                      app.simulationTime = 0.0f;
                                      app.isRunning = false;
                                      scheduler.current_time = 0.0f;
                                      
                                      std::cout << "Mode changed to: ";
                                      switch (mode)
                                      {
                                      case SimulationMode::TASK1:
                                          app.m_mode = SimulationMode::TASK1;
                                          std::cout << "TASK1" << std::endl;
                                          // TASK1模式下初始化3辆车用于演示
                                          app.scheduler.vehicle_manager_ptr->initializeVehicles(3);

                                          // 设置随机目标设备并让车辆开始运动
                                          {
                                              auto &vehicles = scheduler.vehicle_manager_ptr->getAllVehicles();
                                              if (vehicles.size() >= 3)
                                              {
                                                  srand(time(NULL));
                                                  int random0 = 1 + rand() % 18;
                                                  int random1 = 1 + rand() % 18;
                                                  int random2 = 1 + rand() % 18;

                                                  // 确保不选择设备15
                                                  while (random0 == 15)
                                                      random0 = 1 + rand() % 18;
                                                  while (random1 == 15)
                                                      random1 = 1 + rand() % 18;
                                                  while (random2 == 15)
                                                      random2 = 1 + rand() % 18;

                                                  vehicles[0].towards_device = random0;
                                                  vehicles[1].towards_device = random1;
                                                  vehicles[2].towards_device = random2;

                                                  // 让车辆开始运动
                                                  vehicles[0].m_state.motionState = Vehicle::MotionState::Accelerating;
                                                  vehicles[1].m_state.motionState = Vehicle::MotionState::Accelerating;
                                                  vehicles[2].m_state.motionState = Vehicle::MotionState::Accelerating;

                                                  std::cout << "TASK1 vehicles targets: " << random0 << ", " << random1 << ", " << random2 << std::endl;
                                              }
                                          }

                                          app.updateSchedulerDependentComponents(); // 刷新GUI显示
                                          break;
                                      case SimulationMode::TASK2_1:
                                          app.m_mode = SimulationMode::TASK2_1;
                                          std::cout << "TASK2_1" << std::endl;
                                          break;
                                      case SimulationMode::TASK2_2:
                                          app.m_mode = SimulationMode::TASK2_2;
                                          std::cout << "TASK2_2" << std::endl;
                                          break;
                                      case SimulationMode::TASK2_3:
                                          app.m_mode = SimulationMode::TASK2_3;
                                          std::cout << "TASK2_3" << std::endl;
                                          break;
                                      }

                                      // 更新GUI显示重置后的时间
                                      app.toolbar->updateTimeDisplay(app.simulationTime);
                                      app.statusPanel->setSimulationTime(app.simulationTime);
                                      
                                      // 通知后端模式变化
                                      std::cout << "仿真时间已重置，仿真已暂停，前端传入工作已经完成，后端确认" << std::endl; });

    // 默认启动TASK1模式并初始化车辆
    std::cout << "Initializing default TASK1 mode..." << std::endl;
    // scheduler.vehicle_manager_ptr->initializeVehicles(3);

    // 设置车辆的初始目标设备（用于TASK1演示）
    auto &vehicles = scheduler.vehicle_manager_ptr->getAllVehicles();
    if (vehicles.size() >= 3)
    {
        srand(time(NULL));
        int random0 = 1 + rand() % 18;
        int random1 = 1 + rand() % 18;
        int random2 = 1 + rand() % 18;

        // 确保不选择设备15
        while (random0 == 15)
            random0 = 1 + rand() % 18;
        while (random1 == 15)
            random1 = 1 + rand() % 18;
        while (random2 == 15)
            random2 = 1 + rand() % 18;

        vehicles[0].towards_device = random0;
        vehicles[1].towards_device = random1;
        vehicles[2].towards_device = random2;

        // 让车辆开始运动（从Stopped状态改为Accelerating）
        vehicles[0].m_state.motionState = Vehicle::MotionState::Accelerating;
        vehicles[1].m_state.motionState = Vehicle::MotionState::Accelerating;
        vehicles[2].m_state.motionState = Vehicle::MotionState::Accelerating;

        std::cout << "Vehicle targets set: " << random0 << ", " << random1 << ", " << random2 << std::endl;
        std::cout << "Vehicles set to Accelerating state for immediate movement" << std::endl;
    }

    // 确保GUI立即显示车辆
    app.updateSchedulerDependentComponents();
    std::cout << "GUI refreshed to display vehicles" << std::endl;

    // 不再在前端初始化车辆，等待后端通信
    // 注释掉车辆初始化代码以避免前后端冲突：
    // scheduler.vehicle_manager_ptr->initializeVehicles(3);
    // auto &vehicles = scheduler.vehicle_manager_ptr->getAllVehicles();

    std::cout << "前端默认初始为Task1，后端通信已保证" << std::endl;

    app.run();

    return 0;
}
void updateVehicle1(float current_time, float deltaTime, Vehicle *vehicle, Vehicle *leadingVehicle)
{
    // 前车与后车相对距离
    float distance;
    float epsilon = 0.05f; // 防止浮点数误差
    float device_position[19] = {
        -1000.0f,
        85.9209372261538,
        83.5209372261538,
        79.9209372261538,
        77.5209372261538,
        73.9209372261538,
        71.5209372261538,
        67.9209372261538,
        65.5209372261538,
        61.9209372261538,
        59.5209372261538,
        55.9209372261538,
        53.5209372261538,
        32.000,
        29.000,
        26.000,
        14.0,
        11.000,
        8.000,
    };
    float VehiclePosition = std::fmod(vehicle->m_state.position, 99.47787445225672);
    float LeadingVehiclePosition = std::fmod(leadingVehicle->m_state.position, 99.47787445225672);
    float distancetodevice = device_position[vehicle->towards_device] - VehiclePosition;
    if (fabs(distancetodevice) > epsilon && distancetodevice < 0.f)
    {
        distancetodevice += 99.47787445225672;
    }
    // 对接任务的接口
    vehicle->position_m = VehiclePosition;
    vehicle->velocity_mps = vehicle->m_state.currentSpeed;

    // 前车与后车相对距离
    distance = LeadingVehiclePosition - VehiclePosition;
    if (distance < 0.f)
    {
        distance += 99.47787445225672;
    }
    // 防止浮点数误差
    epsilon = 0.55f;
    // std::cout << "Vehicle " << vehicle->id << "distance:"<<fabs(distancetodevice)<<std::endl;
    // if(fabs(distancetodevice) < epsilon)
    // std::cout << "Vehicle " << vehicle->id << "loading "<<vehicle->m_state.operationTimer<<std::endl;
    // 到车库停车的判断和处理
    //  添加调试输出
    //  std::cout << "Vehicle " << vehicle->id << " position: " << VehiclePosition
    //            << ", target: " << device_position[vehicle->towards_device]
    //            << ", distance: " << fabs(VehiclePosition - device_position[vehicle->towards_device]) << std::endl;
    //  std::cout << "Vehicle " << vehicle->id << "speed"<<vehicle->m_state.currentSpeed<<std::endl;
    //  std::cout << "Vehicle " << vehicle->id << " distance: " << distance << std::endl;
    //  if(vehicle->id==0){
    //  std::cout << "Vehicle " << vehicle->id << "speed"<<vehicle->m_state.currentSpeed<<std::endl;
    //  std::cout << "Vehicle " << vehicle->id << " distance: " << distancetodevice << std::endl;
    //  }
    if ((vehicle->m_state.motionState == Vehicle::MotionState::Stopped) && fabs(distancetodevice) < epsilon)
    {
        // std::cout << "Vehicle " << vehicle->id << "loading "<<vehicle->m_state.operationTimer<<std::endl;
        // std::cout << std::endl;
        if (vehicle->m_state.operationTimer >= vehicle->m_loadTime)
        {
            vehicle->m_state.motionState = Vehicle::MotionState::Accelerating;
            vehicle->m_state.operationTimer = 0.0f;
            vehicle->towards_device = 15;
            vehicle->towards_device = rand() % 18 + 1;
        }
        else
        {
            vehicle->m_state.operationTimer += deltaTime;
        }
    }
    // 防碰撞减速
    else if (((vehicle->m_state.currentSpeed) * (vehicle->m_state.currentSpeed) / (2 * vehicle->m_acceleration)) >= (distance - 0.2 - vehicle->m_length))
    {
        // std::cout << "OK"<<std::endl;
        vehicle->m_state.motionState = Vehicle::MotionState::Decelerating;
    }
    // 到车库提前减速
    else if ((vehicle->m_state.currentSpeed) * (vehicle->m_state.currentSpeed) / (2 * vehicle->m_acceleration) >= (distancetodevice))
    {
        // std::cout<<"stop"<<std::endl;
        // std::cout << "Vehicle " << vehicle->id << "target"<<vehicle->towards_device<<std::endl;
        vehicle->m_state.motionState = Vehicle::MotionState::Decelerating;
    }
    // 弯道减速
    // 处理下面那个弯道的减速
    else if ((VehiclePosition >= 0.f) && (VehiclePosition <= 40.0f) && ((40.0f - VehiclePosition) <= (((vehicle->m_state.currentSpeed) * (vehicle->m_state.currentSpeed)) - (vehicle->m_maxCurveSpeed) * (vehicle->m_maxCurveSpeed)) / (2 * vehicle->m_acceleration)))
    {
        // std::cout<<"stop"<<std::endl;
        vehicle->m_state.motionState = Vehicle::MotionState::Decelerating;
        // std::cout<<vehicle->m_state.currentSpeed<<std::endl;
        // std::cout<<vehicle->m_maxCurveSpeed<<std::endl;
        // std::cout<<(((vehicle->m_state.currentSpeed)*(vehicle->m_state.currentSpeed))-(vehicle->m_maxCurveSpeed)*(vehicle->m_maxCurveSpeed))/(2*vehicle->m_acceleration)<<std::endl;
    }
    // 处理上面那个弯道的减速
    else if ((VehiclePosition >= 49.5209372261538) && (VehiclePosition <= 89.5209372261538) && ((89.5209372261538 - VehiclePosition) <= (((vehicle->m_state.currentSpeed) * (vehicle->m_state.currentSpeed)) - (vehicle->m_maxCurveSpeed) * (vehicle->m_maxCurveSpeed)) / (2 * vehicle->m_acceleration)))
    {
        // std::cout<<"stop"<<std::endl;
        vehicle->m_state.motionState = Vehicle::MotionState::Decelerating;
    }
    // 不减速即设定为加速，更快运动
    else
    {
        vehicle->m_state.motionState = Vehicle::MotionState::Accelerating;
        // std::cout<<vehicle->m_maxStraightSpeed<<std::endl;
    }
    // 根据状态确定下一步的操作
    switch (vehicle->m_state.motionState)
    {
    case Vehicle::MotionState::Accelerating:
        // 判断上一辆车的距离
        vehicle->m_state.currentSpeed += vehicle->m_acceleration * deltaTime;
        // 直线上且超过最大速度
        if ((VehiclePosition >= 0.f) && (VehiclePosition <= 40.0f) && (vehicle->m_state.currentSpeed > vehicle->m_maxStraightSpeed))
        {
            vehicle->m_state.currentSpeed = vehicle->m_maxStraightSpeed;
        }
        else if ((VehiclePosition >= 49.5209372261538) && (VehiclePosition <= 89.5209372261538) && (vehicle->m_state.currentSpeed > vehicle->m_maxStraightSpeed))
        {
            vehicle->m_state.currentSpeed = vehicle->m_maxStraightSpeed;
        }
        // 弯道上且超过最大速度
        else if ((VehiclePosition > 40.0f) && (VehiclePosition < 49.5209372261538) && (vehicle->m_state.currentSpeed > vehicle->m_maxCurveSpeed))
        {
            vehicle->m_state.currentSpeed = vehicle->m_maxCurveSpeed;
        }
        else if ((VehiclePosition > 89.5209372261538) && (VehiclePosition < 99.47787445225672) && (vehicle->m_state.currentSpeed > vehicle->m_maxCurveSpeed))
        {
            vehicle->m_state.currentSpeed = vehicle->m_maxCurveSpeed;
        }
        break;
    case Vehicle::MotionState::Decelerating:
        vehicle->m_state.currentSpeed -= vehicle->m_acceleration * deltaTime;
        if (vehicle->m_state.currentSpeed <= 0.0f)
        {
            vehicle->m_state.currentSpeed = 0.0f;
            vehicle->m_state.motionState = Vehicle::MotionState::Stopped;
        }
        break;
    case Vehicle::MotionState::Cruising:
        break;
    case Vehicle::MotionState::Stopped:
        break;
    default:
        // 处理未知状态
        break;
    }
    // 位置的更新
    vehicle->m_state.position += vehicle->m_state.currentSpeed * deltaTime;
}