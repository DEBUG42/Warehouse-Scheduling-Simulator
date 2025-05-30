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
#include "../src/Core/Task.hpp" // Include Task.hpp for Task structure

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
    sf::Font font; // GUI组件
    std::unique_ptr<Toolbar> toolbar;
    std::unique_ptr<StatusPanel> statusPanel;
    std::unique_ptr<VehicleInfoPanel> vehicleInfoPanel;
    std::unique_ptr<SimulationView> simulationView;

    // 仿真数据 - 多个车辆实现
    std::vector<std::unique_ptr<Vehicle>> vehicles;
    std::vector<std::unique_ptr<DeviceBase>> devices;

    // 仿真状态
    float simulationTime = 0.0f;
    bool isRunning = false;
    int selectedVehicleId = -1; // 布局参数
    static constexpr float TOOLBAR_HEIGHT = 50.0f;
    static constexpr float STATUS_PANEL_WIDTH = 290.0f;
    static constexpr float VEHICLE_INFO_PANEL_WIDTH = 300.0f;
    static constexpr float VEHICLE_INFO_PANEL_HEIGHT = 250.0f;
    static constexpr float SIMULATION_VIEW_MARGIN = 10.0f;

    // Sample Tasks for TaskListView
    std::vector<Task> sampleTasks;
    // std::vector<std::string> sampleTaskStrings; // No longer primary, but can be kept for compatibility or removed

public:
    SimpleDemoApp() : window(sf::VideoMode(1800, 630), "GUI Phase 1 - Enhanced Demo with Warehouse & Vehicles")
    {
        loadFont();
        initializeSimulationData();
        initializeComponents();
        createSampleTasks(); // Create sample tasks
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

    void initializeSimulationData()
    {
        // 按照VehiclePathPositionTest.cpp和SimulationView_Test.cpp的方式创建3辆车辆，展示完整的轨道和车辆渲染
        const int vehicleCount = 3;
        vehicles.clear();

        for (int i = 0; i < vehicleCount; ++i)
        {
            auto vehicle = std::make_unique<Vehicle>();
            vehicle->id = i + 1;                                                // ID从1开始
            vehicle->position_m = (32.000 - 0.002 * i - vehicle->m_length * i); // 每辆车相距间距，按VehicleManager::initializeVehicles的方法
            vehicle->velocity_mps = 0.0f;
            vehicle->max_speed = 8.0f / 3.0f; // 使用VehicleManager的默认最大速度
            vehicle->is_loaded = false;
            vehicle->is_executing = false;
            vehicle->towards_device = 0;
            vehicle->next_available_time = 0.0f;

            // 初始化状态
            vehicle->m_state.position = vehicle->position_m;
            vehicle->m_state.currentSpeed = 0.0f;
            vehicle->m_state.motionState = Vehicle::MotionState::Stopped;
            vehicle->m_state.currentTask = nullptr;
            vehicle->m_state.operationTimer = 0.0f;

            vehicles.push_back(std::move(vehicle));
        }

        // 设置不同状态（与SimulationView_Test.cpp一致）
        if (vehicles.size() >= 2)
        {
            vehicles[1]->is_loaded = false;                                    // 车辆2：空载状态
            vehicles[1]->m_state.motionState = Vehicle::MotionState::Cruising; // 巡航状态
        }
        if (vehicles.size() >= 3)
        {
            vehicles[2]->is_loaded = true;                                         // 车辆3：载货状态
            vehicles[2]->m_state.motionState = Vehicle::MotionState::Accelerating; // 加速状态
        }

        std::cout << "Initialized " << vehicles.size() << " vehicles with track and multiple vehicle rendering" << std::endl;
    }
    void initializeComponents()
    {
        // 创建工具栏 - 修正参数顺序：(font, width, height)
        toolbar = std::make_unique<Toolbar>(font, window.getSize().x, TOOLBAR_HEIGHT); // 创建状态面板
        statusPanel = std::make_unique<StatusPanel>(font);
        statusPanel->resize(window.getSize().y - 10.0f);

        // 创建车辆信息面板
        vehicleInfoPanel = std::make_unique<VehicleInfoPanel>(font, VEHICLE_INFO_PANEL_WIDTH, VEHICLE_INFO_PANEL_HEIGHT);

        // 创建仿真视图 - 使用正确的构造函数
        simulationView = std::make_unique<SimulationView>(font);

        // 初始化仿真视图 - 使用参考实现的参数设置
        float simulationViewWidth = window.getSize().x - STATUS_PANEL_WIDTH - SIMULATION_VIEW_MARGIN * 2; // Adjusted width
        float simulationViewHeight = window.getSize().y - TOOLBAR_HEIGHT - SIMULATION_VIEW_MARGIN * 2;
        sf::Vector2f simulationViewSize(simulationViewWidth, simulationViewHeight);

        // 使用与VehiclePathPositionTest.cpp相同的初始化方法
        simulationView->initialize(font, nullptr, simulationViewSize); // 设置显示选项（与测试文件一致）
        simulationView->setShowGrid(false);                            // 默认关闭网格
        simulationView->setShowWarehouses(true);                       // 显示仓库
        simulationView->setShowVehicles(true);                         // 显示车辆
        simulationView->setShowDebugInfo(false);                       // 默认关闭调试信息        // 正确的做法：通过 SimulationView 的 updateVehicles 方法更新车辆数据
        std::vector<Vehicle *> vehiclePtrs;
        for (auto &vehicle : vehicles)
        {
            vehiclePtrs.push_back(vehicle.get());
        }
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
                                       { std::cout << "Speed adjusted to: " << speed << "x" << std::endl; }); // 设置初始状态
        statusPanel->setSimulationTime(0.0f);
        statusPanel->setVehicleCount(vehicles.size());
        statusPanel->setCompletedTaskCount(0);
        statusPanel->setPendingTaskCount(5); // Initial placeholder, will be updated by sample tasks
    }

    void createSampleTasks()
    {
        sampleTasks.clear();
        // sampleTaskStrings.clear(); // If removing

        for (int i = 0; i < 5; ++i)
        {
            Task task;
            task.id = 100 + i;
            task.material_id = "MAT_" + std::to_string(i);
            task.type = (i % 2 == 0) ? TaskType::INBOUND : TaskType::OUTBOUND;
            task.start_device_id = 1 + i;
            task.end_device_id = 6 + i;
            task.is_assigned = (i % 3 == 0);
            task.assigned_vehicle_id = task.is_assigned ? ((i / 3) % vehicles.size()) + 1 : -1;
            task.ready_time = simulationTime + i * 10.0; // Stagger ready times
            sampleTasks.push_back(task);

            // Create string representation for current TaskListView (if still needed for compatibility)
            /* std::string taskStr = "Task " + std::to_string(task.id) + ": " +
                                (task.type == TaskType::INBOUND ? "IN" : "OUT") +
                                " M" + task.material_id +
                                " D" + std::to_string(task.start_device_id) + "->D" + std::to_string(task.end_device_id) +
                                (task.is_assigned ? " (V" + std::to_string(task.assigned_vehicle_id) + ")" : " (Unassigned)");
            sampleTaskStrings.push_back(taskStr); */
        }
        // Update status panel with these tasks
        if (statusPanel)
        {
            // Pass the actual Task objects now
            statusPanel->refreshContent(nullptr, "", sampleTasks);
        }
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
                case sf::Keyboard::R:
                    simulationTime = 0.0f;
                    isRunning = false;
                    toolbar->setPlaying(false);
                    resetSimulation();
                    std::cout << "Simulation reset" << std::endl;
                    continue;
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
                    continue;

                // 车辆控制键
                case sf::Keyboard::Num1:
                case sf::Keyboard::Num2:
                case sf::Keyboard::Num3:
                {
                    int vehicleId = event.key.code - sf::Keyboard::Num0;
                    if (vehicleId > 0 && vehicleId <= vehicles.size())
                    {
                        selectedVehicleId = vehicleId;
                        std::cout << "Selected vehicle " << vehicleId << std::endl;
                        // Update panels with selected vehicle info
                        if (statusPanel)
                        {
                            statusPanel->refreshContent(vehicles[selectedVehicleId - 1].get(), "Vehicle", sampleTasks);
                        }
                        if (vehicleInfoPanel)
                        {
                            vehicleInfoPanel->setVehicle(vehicles[selectedVehicleId - 1].get());
                        }
                    }
                }
                    continue;
                case sf::Keyboard::Up:
                case sf::Keyboard::Down:
                    if (selectedVehicleId > 0 && selectedVehicleId <= vehicles.size())
                    {
                        moveSelectedVehicle(event.key.code);
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
                float simulationViewWidth = window.getSize().x - STATUS_PANEL_WIDTH - SIMULATION_VIEW_MARGIN * 2;
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
                                std::cout << "Mouse selected vehicle ID: " << selectedVehicleId << std::endl; // Update panels with selected vehicle info
                                if (statusPanel && selectedVehicleId > 0 && selectedVehicleId <= vehicles.size())
                                {
                                    statusPanel->refreshContent(vehicles[selectedVehicleId - 1].get(), "Vehicle", sampleTasks);
                                }
                                if (vehicleInfoPanel && selectedVehicleId > 0 && selectedVehicleId <= vehicles.size())
                                {
                                    vehicleInfoPanel->setVehicle(vehicles[selectedVehicleId - 1].get());
                                }
                            }
                        }
                        else if (selectedVehicleId != -1)
                        {
                            selectedVehicleId = -1;
                            std::cout << "Mouse deselected vehicle" << std::endl; // Clear panels or set to no selection
                            if (statusPanel)
                            {
                                statusPanel->refreshContent(nullptr, "", sampleTasks);
                            }
                            if (vehicleInfoPanel)
                            {
                                vehicleInfoPanel->setVehicle(nullptr);
                            }
                        }
                    }
                    // Forward scroll events to StatusPanel if mouse is over it
                    float statusPanelX = window.getSize().x - STATUS_PANEL_WIDTH;
                    if (event.type == sf::Event::MouseWheelScrolled &&
                        mousePixelPos.x >= statusPanelX)
                    {
                        sf::Vector2f localPosToStatusPanel(mousePixelPos.x - statusPanelX, mousePixelPos.y);
                        if (statusPanel)
                        {
                            statusPanel->handleEvent(event, localPosToStatusPanel);
                        }
                    }
                    // Forward click events to StatusPanel if mouse is over it
                    if (event.type == sf::Event::MouseButtonPressed &&
                        mousePixelPos.x >= statusPanelX)
                    {
                        sf::Vector2f localPosToStatusPanel(mousePixelPos.x - statusPanelX, mousePixelPos.y);
                        if (statusPanel)
                        {
                            statusPanel->handleEvent(event, localPosToStatusPanel);
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

            // 更新工具栏时间显示
            toolbar->updateTimeDisplay(simulationTime); // 更新状态面板
            statusPanel->setSimulationTime(simulationTime);

            // 更新车辆信息面板
            if (vehicleInfoPanel)
            {
                vehicleInfoPanel->updateInfo(simulationTime);
            }

            // 更新仿真视图
            if (simulationView)
            {
                simulationView->updateViewTransforms(deltaTime * timeScale);
            }
        }
    }

    void moveSelectedVehicle(sf::Keyboard::Key key)
    {
        // 检查是否有有效的选中车辆
        if (selectedVehicleId <= 0 || selectedVehicleId > vehicles.size())
        {
            return;
        }

        auto &vehicle = vehicles[selectedVehicleId - 1]; // ID从1开始，数组从0开始
        float currentPos = vehicle->position_m;
        float newPos = currentPos;

        const float moveSpeed = 2.0f; // 移动速度（米）- 轨道距离步长        // 只支持前后移动，基于轨道距离
        switch (key)
        {
        case sf::Keyboard::Up:
            newPos += moveSpeed; // 沿轨道向前移动（增加距离）
            break;
        case sf::Keyboard::Down:
            newPos -= moveSpeed; // 沿轨道向后移动（减少距离）
            break;
        default:
            return;
        } // 获取轨道总长度（使用 TrackRenderer 的实际数据）
        float maxTrackLengthMm = simulationView->getTrackRenderer().getTotalCenterLineLengthMm();
        float maxTrackLength = maxTrackLengthMm / 1000.0f; // 转换为米

        // 实现绕圈移动（超过总长度时回到起点，小于0时到终点）
        if (newPos >= maxTrackLength)
        {
            newPos = std::fmod(newPos, maxTrackLength);
        }
        else if (newPos < 0)
        {
            newPos = maxTrackLength + std::fmod(newPos, maxTrackLength);
        }

        // 更新车辆位置
        vehicle->position_m = newPos;
        vehicle->m_state.position = newPos;
        vehicle->m_state.motionState = Vehicle::MotionState::Cruising;

        std::cout << "Vehicle " << selectedVehicleId << " moved along track to position: " << newPos << "m (Track length: " << maxTrackLength << "m)" << std::endl;

        // 更新仿真视图中的车辆数据
        if (simulationView)
        {
            std::vector<Vehicle *> vehiclePtrs;
            for (auto &v : vehicles)
            {
                vehiclePtrs.push_back(v.get());
            }
            simulationView->updateVehicles(vehiclePtrs);
        }
    }
    void resetSimulation()
    {
        // 重置所有车辆位置和状态
        for (int i = 0; i < vehicles.size(); ++i)
        {
            auto &vehicle = vehicles[i];
            vehicle->position_m = (32.000 - 0.002 * i - vehicle->m_length * i); // 使用VehicleManager的初始位置方法
            vehicle->velocity_mps = 0.0f;
            vehicle->is_loaded = false;
            vehicle->m_state.position = vehicle->position_m;
            vehicle->m_state.currentSpeed = 0.0f;
            vehicle->m_state.motionState = Vehicle::MotionState::Stopped;
        }

        // 重新设置不同状态
        if (vehicles.size() >= 2)
        {
            vehicles[1]->is_loaded = false;                                    // 车辆2：空载状态
            vehicles[1]->m_state.motionState = Vehicle::MotionState::Cruising; // 巡航状态
        }
        if (vehicles.size() >= 3)
        {
            vehicles[2]->is_loaded = true;                                         // 车辆3：载货状态
            vehicles[2]->m_state.motionState = Vehicle::MotionState::Accelerating; // 加速状态
        }

        // 清除选择
        selectedVehicleId = -1;

        // 更新仿真视图数据
        if (simulationView)
        {
            std::vector<Vehicle *> vehiclePtrs;
            for (auto &vehicle : vehicles)
            {
                vehiclePtrs.push_back(vehicle.get());
            }
            simulationView->updateVehicles(vehiclePtrs);

            // 设备重置暂时跳过，因为我们没有创建设备
            std::vector<DeviceBase *> devicePtrs;
            simulationView->updateDevices(devicePtrs);
        }

        // 重置统计
        statusPanel->setCompletedTaskCount(0);
        statusPanel->setPendingTaskCount(5);
        std::cout << "Simulation reset - " << vehicles.size() << " vehicles positioned" << std::endl;
    }
    void render()
    {
        window.clear(sf::Color(245, 245, 245)); // 浅灰色背景        // 首先渲染仓库仿真视图（背景层）
        if (simulationView)
        {
            // 设置仿真视图的视口（排除工具栏、状态面板和车辆信息面板区域）
            // Adjust SimulationView position and width calculation
            float simulationViewLeft = SIMULATION_VIEW_MARGIN; // Starts from left margin
            float simulationViewTop = TOOLBAR_HEIGHT + SIMULATION_VIEW_MARGIN;
            float simulationViewWidth = window.getSize().x - STATUS_PANEL_WIDTH - SIMULATION_VIEW_MARGIN * 2; // Adjusted width
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
        toolbar->render(window, sf::Vector2f(0, 0));   // 渲染状态面板（右侧，自动贴在右边缘）
        statusPanel->render(window, window.getSize()); // 渲染车辆信息面板（左上角）
        if (vehicleInfoPanel)
        {
            float vehicleInfoX = SIMULATION_VIEW_MARGIN;
            float vehicleInfoY = TOOLBAR_HEIGHT + SIMULATION_VIEW_MARGIN; // 在工具栏下方
            vehicleInfoPanel->setPosition(vehicleInfoX, vehicleInfoY);
            window.draw(*vehicleInfoPanel);
        }

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
        std::string text = "GUI Phase 1 Enhanced Demo - Track & 3 Vehicles\n"
                           "=== Simulation Control ===\n"
                           "Space: Start/Pause simulation\n"
                           "R: Reset simulation\n"
                           "ESC: Exit program\n"
                           "\n=== Display Control ===\n"
                           "T: Toggle coordinate grid display\n"
                           "G: Toggle warehouse display\n"
                           "V: Toggle vehicle display\n"
                           "C: Toggle debug info display\n"
                           "\n=== Vehicle Control ===\n"
                           "1/2/3: Select vehicle (by number)\n"
                           "W/S: Move selected vehicle forward/backward along track\n"
                           "Mouse: Click to select vehicle/device\n"
                           "\n=== Vehicle Status ===\n"
                           "• Vehicle 1: Empty Stopped (Blue)\n"
                           "• Vehicle 2: Empty Cruising (Blue Moving)\n"
                           "• Vehicle 3: Loaded Accelerating (Orange Moving)\n"
                           "\n=== GUI Components ===\n"
                           "• Toolbar: Time display (HH:MM:SS.mmm)\n"
                           "• Status Panel: Simulation statistics\n"
                           "• Simulation View: Track & multi-vehicle rendering";
        instructions.setString(text);

        window.draw(instructions);
    }

    void run()
    {
        sf::Clock clock;
        std::cout << "GUI Phase 1 Enhanced Demo - Track & 3 Vehicles Started" << std::endl;
        std::cout << "Features:" << std::endl;
        std::cout << "1. Toolbar - Time format display (HH:MM:SS.mmm)" << std::endl;
        std::cout << "2. StatusPanel - 25:35:40 ratio layout" << std::endl;
        std::cout << "3. VehicleInfoPanel - Vehicle acceleration info recording" << std::endl;
        std::cout << "4. SimulationView - Track rendering with 3 vehicles" << std::endl;
        std::cout << "5. Encapsulated rendering logic from VehiclePathPositionTest.cpp" << std::endl;
        std::cout << "\nVehicles initialized:" << std::endl;
        std::cout << "• Vehicle 1: Empty, Stopped (Blue)" << std::endl;
        std::cout << "• Vehicle 2: Empty, Cruising (Blue, moving)" << std::endl;
        std::cout << "• Vehicle 3: Loaded, Accelerating (Orange, moving)" << std::endl;
        std::cout << "\nInstructions: Press Space to start simulation, 1/2/3 to select vehicles, WASD to move" << std::endl;

        while (window.isOpen())
        {
            float deltaTime = clock.restart().asSeconds();
            printf("Delta Time: %.3f seconds\n", deltaTime); // 输出每帧的时间间隔
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
