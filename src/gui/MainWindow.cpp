#include "gui/MainWindow.hpp"
#include "gui/SimObject.hpp"
#include "gui/MockSimulationInterface.hpp"
#include "gui/Toolbar.hpp"
#include "gui/StatusPanel.hpp"
#include "gui/TaskListView.hpp"
#include <iostream>

/**
 * @brief 初始化窗口（使用仿真接口）
 * 创建窗口，初始化子组件，设置回调函数
 * @param simInterface 仿真接口指针
 */
void MainWindow::initialize(std::shared_ptr<SimulationInterface> simInterface)
{
    // 保存仿真接口
    m_simInterface = simInterface;

    // 创建窗口
    create(sf::VideoMode(m_initialSize.x, m_initialSize.y), "Warehouse Shuttle Simulation System",
           sf::Style::Default);
    setFramerateLimit(60);

    // 加载全局字体
    // Try loading a preferred English font first, then fallback to Arial.
    if (!m_globalFont.loadFromFile("assets/fonts/Roboto-Regular.ttf")) // Assuming this is a desired path
    {
        // If Roboto-Regular is not found, try arial.ttf from a more standard assets path
        if (!m_globalFont.loadFromFile("assets/fonts/arial.ttf"))
        {
            std::cerr << "Warning: Failed to load global font. Some text may not display correctly." << std::endl;
        }
    }

    // 初始化子组件
    m_simView = std::make_unique<SimulationView>(m_globalFont);
    m_simView->initialize(m_globalFont, m_simInterface, sf::Vector2f(static_cast<float>(m_initialSize.x), static_cast<float>(m_initialSize.y)));

    m_statusPanel = std::make_unique<StatusPanel>(m_globalFont);
    m_toolbar = std::make_unique<Toolbar>(m_globalFont, m_toolbarHeight, m_initialSize.x);

    m_taskListViewLeft = std::make_unique<TaskListView>(m_globalFont, 250.f); // 左侧250像素宽

    // 注册回调
    m_simInterface->registerStateUpdateCallback(
        [this](const SimulationInterface::SimulationState &state)
        {
            onSimulationStateUpdate(state);
        });

    m_simInterface->registerVehicleUpdateCallback(
        [this](const std::vector<gui::VehicleState> &vehicles)
        {
            onVehicleUpdate(vehicles);
        });

    m_simInterface->registerDeviceUpdateCallback(
        [this](const std::vector<gui::DeviceState> &devices)
        {
            onDeviceUpdate(devices);
        });

    // 设置工具栏回调
    m_toolbar->setTimeScaleCallback([this](float scale)
                                    {
        // 直接使用 float scale 调用新的接口
        m_simInterface->setSimulationSpeedFactor(scale); });

    m_toolbar->setPlayPauseCallback([this]()
                                    {
        // 切换仿真暂停/运行状态
        auto state = m_simInterface->getSimulationState();
        if (state.isPaused) {
            m_simInterface->resumeSimulation();
        } else {
            m_simInterface->pauseSimulation();
        } });

    // 更新布局
    updateLayout();
}

void MainWindow::runEventLoop()
{
    sf::Clock frameClock;

    while (isOpen())
    {
        // 处理所有事件
        sf::Event event;
        while (pollEvent(event))
        {
            handleSystemEvent(event);
        }

        // 计算帧时间
        float deltaTime = frameClock.restart().asSeconds();

        // 清除屏幕
        clear(m_backgroundColor);

        // 更新视图变换
        m_simView->updateViewTransforms(deltaTime);

        // 绘制仿真视图
        m_simView->renderWorld(*this);

        // 绘制状态面板
        sf::Vector2f statusPanelPos(m_initialSize.x - m_statusPanel->getPanelWidth(), m_toolbarHeight);
        m_statusPanel->render(*this, statusPanelPos);

        // 绘制工具栏
        m_toolbar->render(*this, sf::Vector2f(0, 0));

        // 绘制左侧任务队列
        if (m_taskListViewLeft)
            draw(*m_taskListViewLeft);

        // 显示绘制的内容
        display();
    }
}

void MainWindow::handleSystemEvent(const sf::Event &event)
{
    // 窗口关闭事件
    if (event.type == sf::Event::Closed)
    {
        close();
        return;
    }

    // 窗口大小改变事件
    if (event.type == sf::Event::Resized)
    {
        m_initialSize = sf::Vector2u(event.size.width, event.size.height);
        updateLayout();
        return;
    }

    // 获取当前鼠标位置
    sf::Vector2f mousePos;
    if (event.type == sf::Event::MouseMoved ||
        event.type == sf::Event::MouseButtonPressed ||
        event.type == sf::Event::MouseButtonReleased ||
        event.type == sf::Event::MouseWheelScrolled)
    {
        mousePos = sf::Vector2f(sf::Mouse::getPosition(*this));
    }

    // 处理工具栏事件
    if (mousePos.y < m_toolbarHeight)
    {
        m_toolbar->handleEvent(event, mousePos);
        return;
    }

    // 处理状态面板事件
    if (mousePos.x > m_initialSize.x - m_statusPanel->getPanelWidth())
    {
        m_statusPanel->handleEvent(event, mousePos);
        return;
    }

    // 处理仿真视图事件
    m_simView->handleViewEvent(event, mousePos);
}

void MainWindow::updateLayout()
{
    // 更新视图
    sf::View view;
    view.setSize(static_cast<float>(m_initialSize.x), static_cast<float>(m_initialSize.y));
    view.setCenter(static_cast<float>(m_initialSize.x / 2), static_cast<float>(m_initialSize.y / 2));
    setView(view);

    // 更新工具栏
    m_toolbar->resize(m_initialSize.x);

    // 更新状态面板
    float statusPanelHeight = m_initialSize.y - m_toolbarHeight;
    m_statusPanel->resize(statusPanelHeight);

    // 更新仿真视图
    float simViewWidth = m_initialSize.x - m_statusPanel->getPanelWidth();
    float simViewHeight = m_initialSize.y - m_toolbarHeight;
    sf::FloatRect simViewport(0.0f, m_toolbarHeight / m_initialSize.y,
                              simViewWidth / m_initialSize.x,
                              simViewHeight / m_initialSize.y);
    m_simView->updateViewport(simViewport);

    // 移除未使用的变量 taskListWidth
    if (m_taskListViewLeft)
    {
        m_taskListViewLeft->setViewHeight(m_initialSize.y - m_toolbarHeight);
        m_taskListViewLeft->setPosition(sf::Vector2f(0, m_toolbarHeight));
    }
}

/**
 * @brief 处理仿真状态更新回调
 * @param state 仿真状态
 */
void MainWindow::onSimulationStateUpdate(const SimulationInterface::SimulationState &state)
{
    // 更新状态面板
    m_statusPanel->setSimulationTime(state.simulationTime);
    m_statusPanel->setVehicleCount(state.vehicleCount);
    m_statusPanel->setCompletedTaskCount(state.completedTaskCount);
    m_statusPanel->setPendingTaskCount(state.pendingTaskCount);

    // 更新工具栏状态
    m_toolbar->updateTimeDisplay(state.simulationTime, 0);   // 假设没有真实时间显示，或者需要从别处获取
    m_toolbar->updatePlayPauseState(!state.isPaused);        // isPaused=true -> 按钮显示Play (即非isPlaying)
    m_toolbar->updateTimeScale(state.simulationSpeedFactor); // 使用新的 float speed factor

    if (m_taskListViewLeft)
        m_taskListViewLeft->updateTasks(m_pendingTasks);
}

/**
 * @brief 处理车辆状态更新回调
 * @param vehicles 车辆状态列表
 */
void MainWindow::onVehicleUpdate(const std::vector<gui::VehicleState> &vehicles)
{
    // 通知仿真视图更新车辆状态
    m_simView->updateVehicles(vehicles);
}

/**
 * @brief 处理设备状态更新回调
 * @param devices 设备状态列表
 */
void MainWindow::onDeviceUpdate(const std::vector<gui::DeviceState> &devices)
{
    // 通知仿真视图更新设备状态
    m_simView->updateDevices(devices);
}

// New methods for testing
void MainWindow::processEvent(const sf::Event &event)
{
    // Call the existing comprehensive event handler
    handleSystemEvent(event);

    // Explicitly handle TaskListView events if not covered by handleSystemEvent
    // or if it needs direct calls. Assuming handleSystemEvent is sufficient for now
    // or that TaskListView handles its own events when its region is interacted with.
    // Example if TaskListView needed explicit event forwarding for specific types:
    // if (m_taskListViewLeft) {
    //     // Convert window mouse coordinates to be relative to TaskListView if its handleEvent expects that
    //     // sf::Vector2f mousePosView = mapPixelToCoords(sf::Mouse::getPosition(*this), getView());
    //     // if (m_taskListViewLeft->getGlobalBounds().contains(mousePosView)) {
    //     //     m_taskListViewLeft->handleEvent(event, *this); // Or appropriate arguments
    //     // }
    // }
}

void MainWindow::renderFrame()
{
    clear(m_backgroundColor); // Use internal background color

    // Draw components in a plausible order
    // This order should ideally match or be consistent with runEventLoop if it was decomposed

    if (m_toolbar)
    {
        m_toolbar->render(*this, sf::Vector2f(0, 0));
    }

    if (m_taskListViewLeft)
    {
        // TaskListView is drawn directly, implying its position is managed internally or set during layout
        draw(*m_taskListViewLeft);
    }

    if (m_simView)
    {
        // SimulationView rendering as in runEventLoop
        // m_simView->updateViewTransforms(deltaTime); // deltaTime is not available here without passing it
        // or simView managing its own time for smooth transforms.
        // For a test frame, static transforms are often acceptable.
        m_simView->renderWorld(*this);
        // m_simView->renderUI(*this); // If SimulationView has a separate UI layer to render on top
    }

    if (m_statusPanel)
    {
        // StatusPanel render call as in runEventLoop
        // Use getSize() for current window dimensions to correctly position panel
        sf::Vector2f statusPanelPos(static_cast<float>(getSize().x) - m_statusPanel->getPanelWidth(), m_toolbarHeight);
        m_statusPanel->render(*this, statusPanelPos);
    }
    // Note: display() is NOT called here. The external loop (GUITestMain) will call it.
}