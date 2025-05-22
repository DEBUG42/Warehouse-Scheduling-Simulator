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
    create(sf::VideoMode(m_initialSize.x, m_initialSize.y), "仓储穿梭车仿真系统",
           sf::Style::Default);
    setFramerateLimit(60);

    // 加载全局字体
    if (!m_globalFont.loadFromFile("resources/fonts/simhei.ttf"))
    {
        if (!m_globalFont.loadFromFile("GUI/resources/fonts/simhei.ttf"))
        {
            if (!m_globalFont.loadFromFile("resources/fonts/arial.ttf"))
            {
                std::cerr << "警告：无法加载字体文件！" << std::endl;
            }
        }
    }

    // 初始化子组件
    m_simView = std::make_unique<SimulationView>(m_globalFont);
    m_simView->initialize(m_globalFont, m_simInterface);

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
        // 将浮点数转换为枚举
        SimulationInterface::SimulationSpeed speedEnum;
        if (scale <= 0.5f) {
            speedEnum = SimulationInterface::SimulationSpeed::SPEED_0_5X;
        } else if (scale <= 1.0f) {
            speedEnum = SimulationInterface::SimulationSpeed::SPEED_1X;
        } else if (scale <= 2.0f) {
            speedEnum = SimulationInterface::SimulationSpeed::SPEED_2X;
        } else if (scale <= 5.0f) {
            speedEnum = SimulationInterface::SimulationSpeed::SPEED_5X;
        } else {
            speedEnum = SimulationInterface::SimulationSpeed::SPEED_10X;
        }
        m_simInterface->setSimulationSpeed(speedEnum); });

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

    float taskListWidth = 250.f;
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
    // 更新状态面板显示
    m_statusPanel->setSimulationTime(state.simulationTime);
    m_statusPanel->setVehicleCount(state.vehicleCount);
    m_statusPanel->setCompletedTaskCount(state.completedTaskCount);
    m_statusPanel->setPendingTaskCount(state.pendingTaskCount);

    // 更新工具栏状态
    float timeScale = 1.0f;
    switch (state.speed)
    {
    case SimulationInterface::SimulationSpeed::SPEED_0_5X:
        timeScale = 0.5f;
        break;
    case SimulationInterface::SimulationSpeed::SPEED_1X:
        timeScale = 1.0f;
        break;
    case SimulationInterface::SimulationSpeed::SPEED_2X:
        timeScale = 2.0f;
        break;
    case SimulationInterface::SimulationSpeed::SPEED_5X:
        timeScale = 5.0f;
        break;
    case SimulationInterface::SimulationSpeed::SPEED_10X:
        timeScale = 10.0f;
        break;
    }
    m_toolbar->updateTimeScale(timeScale);
    m_toolbar->updatePlayPauseState(!state.isPaused);

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