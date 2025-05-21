#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <sstream>
#include <iomanip>

// GUI组件头文件
#include "gui/UIControls.hpp"
#include "gui/Toolbar.hpp"
#include "gui/StatusPanel.hpp"
#include "gui/TaskListView.hpp"
#include "gui/ObjectInspector.hpp"
#include "gui/VehicleRenderer.hpp"

// 数据结构头文件
#include "gui/SimObject.hpp"
#include "gui/DeviceState.hpp"
#include "Core/Task.hpp"

// 定义测试窗口大小
const unsigned int WINDOW_WIDTH = 1280;
const unsigned int WINDOW_HEIGHT = 720;

// 参考 TrackRendererGUITest.cpp 中成功使用的字体路径
const std::string FONT_PATH = "../resources/fonts/msyh.ttc"; // 假设这是TrackRendererGUITest中使用的路径

// 辅助函数：为设备绘制简单的图形
void drawSimpleDevice(sf::RenderTarget &target, const gui::DeviceState &device, sf::Font &font, const sf::Color &color)
{
    // 设备的基本形状 (像素单位，位置直接使用 device.position)
    float deviceWidthPx = 60.f;
    float deviceHeightPx = 40.f;
    sf::RectangleShape shape(sf::Vector2f(deviceWidthPx, deviceHeightPx));
    shape.setOrigin(deviceWidthPx / 2.f, deviceHeightPx / 2.f);
    shape.setPosition(device.getPosition()); // 使用 getter
    shape.setFillColor(color);
    shape.setOutlineThickness(2.f);
    shape.setOutlineColor(sf::Color(color.r / 2, color.g / 2, color.b / 2));
    target.draw(shape);

    // 绘制设备ID
    sf::Text idText(device.getId(), font, 10); // 使用 getter
    sf::FloatRect textBounds = idText.getLocalBounds();
    idText.setOrigin(textBounds.left + textBounds.width / 2.0f, textBounds.top + textBounds.height / 2.0f);
    idText.setPosition(device.getPosition()); // 使用 getter
    idText.setFillColor(sf::Color::White);
    target.draw(idText);
}

int main()
{
    // 1. 初始化SFML窗口
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "综合GUI组件测试");
    window.setFramerateLimit(60);

    // 2. 加载字体
    sf::Font font;
    if (!font.loadFromFile(FONT_PATH))
    {
        std::cerr << "错误：无法从路径 '" << FONT_PATH << "' 加载字体。" << std::endl;
        // 可以尝试直接指定一个绝对路径或放在可执行文件同目录下的字体文件作为最后的备选
        if (!font.loadFromFile("msyh.ttc"))
        { // 尝试可执行文件同目录
            std::cerr << "错误：也无法从可执行文件同目录加载 msyh.ttc。请确保字体文件路径正确。" << std::endl;
            return -1;
        }
        std::cout << "字体从可执行文件同目录加载成功。" << std::endl;
    }
    else
    {
        std::cout << "字体 '" << FONT_PATH << "' 加载成功。" << std::endl;
    }

    // 3. 实例化GUI组件
    float toolbarHeight = 30.f;
    Toolbar toolbar(font, toolbarHeight, static_cast<float>(WINDOW_WIDTH));

    StatusPanel statusPanel(font); // 状态面板宽度固定，高度会由resize调整
    statusPanel.resize(static_cast<float>(WINDOW_HEIGHT) - toolbarHeight);

    VehicleRenderer vehicleRenderer(font);

    // 4. 定义静态数据
    // --- 车辆数据 (3个) ---
    std::vector<gui::VehicleState> vehicleData;
    gui::CargoDisplayInfo cargo1_details;
    cargo1_details.materialId = 101;
    cargo1_details.quantity = 10;

    // gui::VehicleState 构造: id, render_pos, raw_track_pos_mm, speed, status, taskId, isLoaded, cargo, battery
    vehicleData.push_back(gui::VehicleState{
        "V_001", sf::Vector2f(0.f, 0.f),            // 初始渲染位置 (会被 VehicleRenderer::updateState 中的 setPosition 覆盖)
        0.0f,                                       // rawTrackPositionMm = 0 (轨道起点)
        10.0f, gui::VehicleStatus::IDLE, "", false, // speed, status, taskId, isLoaded
        gui::CargoDisplayInfo{}, 0.85f              // cargo (empty), battery
    });
    vehicleData.push_back(gui::VehicleState{
        "V_002", sf::Vector2f(0.f, 0.f),
        20000.0f, // rawTrackPositionMm (示例值)
        12.5f, gui::VehicleStatus::MOVING_TO_LOAD, "T_101", false,
        gui::CargoDisplayInfo{}, 0.95f});
    vehicleData.push_back(gui::VehicleState{
        "V_003", sf::Vector2f(0.f, 0.f),
        40000.0f, // rawTrackPositionMm (示例值)
        0.0f, gui::VehicleStatus::ERROR, "T_102", true,
        cargo1_details, 0.30f});

    // --- 设备/仓库数据 (3个) ---
    std::vector<gui::DeviceState> deviceData;
    // 设备1: 工作站 (模拟仓库出入口)
    // DeviceState 构造函数: (id, pos, deviceType, status, boundVehicleId)
    deviceData.push_back(gui::DeviceState{
        "W_A01", sf::Vector2f(100.f, 100.f),
        gui::DeviceType::CORE_WORKSTATION_OUT, gui::DeviceStatus::IDLE, ""}); // 假设是出库工作站
    // 设备2: 充电桩
    deviceData.push_back(gui::DeviceState{
        "C_001", sf::Vector2f(600.f, 100.f),
        gui::DeviceType::CHARGER, gui::DeviceStatus::CHARGING_VEHICLE, "V_002"});
    // 设备3: 工作站 (模拟另一个仓库区域)
    deviceData.push_back(gui::DeviceState{
        "W_B01", sf::Vector2f(600.f, 500.f),
        gui::DeviceType::CORE_WORKSTATION_IN, gui::DeviceStatus::ERROR, ""}); // 假设是入库工作站

    // --- 任务数据 ---
    std::vector<Core::Task> taskData;
    // Core::Task 构造函数: (id_str, type, materialId, startDevId_int, endDevId_int)
    taskData.push_back(Core::Task{"T_101", Core::TaskType::INPUT, 1, 1, 2});  // 假设 startDevId=1, endDevId=2
    taskData.push_back(Core::Task{"T_102", Core::TaskType::OUTPUT, 2, 3, 4}); // 假设 startDevId=3, endDevId=4
    taskData.push_back(Core::Task{"T_103", Core::TaskType::INPUT, 3, 1, 5});  // 假设 startDevId=1, endDevId=5

    // --- 当前选中对象 ---
    const gui::SimObject *selectedObject = &vehicleData[1]; // 默认选中第二辆车

    // 5. GUI组件初始设置和回调
    // Toolbar callbacks (示例：打印到控制台)
    toolbar.setPlayPauseCallback([&]()
                                 { 
        static bool playing = false; playing = !playing; 
        std::cout << (playing ? "Sim Playing" : "Sim Paused") << std::endl; 
        toolbar.updatePlayPauseState(playing); });
    toolbar.setTimeScaleCallback([](float scale)
                                 { std::cout << "Time scale: " << scale << "x" << std::endl; });
    toolbar.setAddTaskCallback([]()
                               { std::cout << "Add Task button clicked" << std::endl; });
    toolbar.setResetViewCallback([]()
                                 { std::cout << "Reset View button clicked" << std::endl; });
    toolbar.setSwitchModeCallback([]()
                                  { std::cout << "Switch Mode button clicked" << std::endl; });

    toolbar.updatePlayPauseState(false); // 初始为暂停状态
    toolbar.updateTimeScale(1.0f);       // 初始时间缩放

    // StatusPanel 初始更新
    statusPanel.setSimulationTime(1234.5f); // 模拟一个仿真时间
    statusPanel.setVehicleCount(vehicleData.size());
    statusPanel.setCompletedTaskCount(5); // 假设已完成5个任务
    statusPanel.setPendingTaskCount(taskData.size());
    statusPanel.refreshContent(selectedObject, taskData);

    // 6. 主循环
    sf::Clock clock; // 用于模拟时间流逝
    float simTime = 1234.5f;

    // 定义轨道几何参数 (示例值，单位mm)
    const float TRACK_TOTAL_STRAIGHT_MM = 80000.0f; // 例如，上下各40000mm
    const float TRACK_CURVE_RADIUS_MM = 15000.0f;   // 例如，弯道半径15000mm

    while (window.isOpen())
    {
        sf::Time elapsed = clock.restart();
        simTime += elapsed.asSeconds() * toolbar.getTimeScaleValue();

        // 事件处理
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            // 将事件传递给Toolbar (鼠标位置相对于窗口)
            sf::Vector2f mousePos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window));
            toolbar.handleEvent(event, mousePos);

            // 将事件传递给StatusPanel (鼠标位置相对于StatusPanel的左上角)
            // StatusPanel区域: x 从 WINDOW_WIDTH - panelWidth 到 WINDOW_WIDTH
            //                  y 从 toolbarHeight 到 WINDOW_HEIGHT
            float panelWidth = statusPanel.getPanelWidth();
            sf::Vector2f statusPanelPos(WINDOW_WIDTH - panelWidth, toolbarHeight);
            if (mousePos.x >= statusPanelPos.x && mousePos.y >= statusPanelPos.y)
            {
                sf::Vector2f localMousePos = mousePos - statusPanelPos;
                statusPanel.handleEvent(event, localMousePos);
            }

            // (可选) 点击切换选中对象 - 简化版
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
            {
                bool found = false;
                for (size_t i = 0; i < vehicleData.size(); ++i)
                {
                    // 简单碰撞检测: 以车辆位置为中心，设定一个点击区域
                    sf::FloatRect clickableArea(vehicleData[i].position.x - 20, vehicleData[i].position.y - 10, 40, 20);
                    if (clickableArea.contains(mousePos))
                    {
                        selectedObject = &vehicleData[i];
                        statusPanel.refreshContent(selectedObject, taskData);
                        found = true;
                        break;
                    }
                }
                if (!found)
                {
                    for (size_t i = 0; i < deviceData.size(); ++i)
                    {
                        sf::FloatRect clickableArea(deviceData[i].position.x - 30, deviceData[i].position.y - 20, 60, 40);
                        if (clickableArea.contains(mousePos))
                        {
                            selectedObject = &deviceData[i];
                            statusPanel.refreshContent(selectedObject, taskData);
                            found = true;
                            break;
                        }
                    }
                }
                if (!found)
                {
                    // selectedObject = nullptr; // 如果点击空白区域则不选中任何对象
                    // statusPanel.refreshContent(selectedObject, taskData);
                }
            }
        }

        // 更新 (非事件驱动)
        toolbar.updateTimeDisplay(simTime, elapsed.asSeconds()); // 更新工具栏时间
        statusPanel.setSimulationTime(simTime);                  // 更新状态面板时间
        // statusPanel.refreshContent(selectedObject, taskData); // 如果选中对象或任务列表动态变化，则需在此更新

        // 渲染
        window.clear(sf::Color(70, 70, 80)); // 深灰色背景

        // --- 绘制主视图区域 (小车和设备) ---
        // 假设主视图区域在工具栏下方，状态面板左方
        // sf::View mainView; // 如果需要独立的视图控制（缩放/平移）
        // mainView.reset(sf::FloatRect(0, toolbarHeight, WINDOW_WIDTH - statusPanel.getPanelWidth(), WINDOW_HEIGHT - toolbarHeight));
        // window.setView(mainView);

        for (const auto &vehicle : vehicleData)
        {
            // 更新VehicleRenderer的状态，这将计算并设置其渲染位置和旋转
            vehicleRenderer.updateState(vehicle, TRACK_TOTAL_STRAIGHT_MM, TRACK_CURVE_RADIUS_MM);
            // vehicleRenderer.setPosition(vehicle.getPosition()); // updateState 内部会调用 this->setPosition
            window.draw(vehicleRenderer); // 绘制小车
        }
        for (const auto &device : deviceData)
        {
            sf::Color devColor = sf::Color::Cyan;
            if (device.deviceType == gui::DeviceType::CHARGER) // Use gui::DeviceType
                devColor = sf::Color::Yellow;
            if (device.status == gui::DeviceStatus::ERROR) // Use gui::DeviceStatus
                devColor = sf::Color(255, 100, 100);
            drawSimpleDevice(window, device, font, devColor);
        }

        // window.setView(window.getDefaultView()); // 恢复默认视图以绘制UI

        // --- 绘制固定UI组件 ---
        toolbar.render(window, sf::Vector2f(0.f, 0.f));
        statusPanel.render(window, sf::Vector2f(WINDOW_WIDTH - statusPanel.getPanelWidth(), toolbarHeight));

        window.display();
    }

    return 0;
}