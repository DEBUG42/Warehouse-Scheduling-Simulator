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
    create(sf::VideoMode(m_initialSize.x, m_initialSize.y), "Warehouse Shuttle Simulation System - GUI",
           sf::Style::Default);
    setFramerateLimit(60);

    // 加载全局字体
    if (!m_globalFont.loadFromFile("resources/fonts/simhei.ttf"))
    {
        if (!m_globalFont.loadFromFile("GUI/resources/fonts/simhei.ttf"))
        {
            if (!m_globalFont.loadFromFile("assets/fonts/arial.ttf")) // 备用英文字体
            {
                std::cerr << "警告：MainWindow无法加载字体文件！将使用SFML默认字体。" << std::endl;
            }
        }
    }

    // 初始化子组件
    m_simView = std::make_unique<SimulationView>(); // 默认构造
    m_simView->initialize(m_globalFont, m_simInterface, sf::Vector2f(static_cast<float>(m_initialSize.x), static_cast<float>(m_initialSize.y)));

    m_statusPanel = std::make_unique<StatusPanel>(m_globalFont);
    m_toolbar = std::make_unique<Toolbar>(m_globalFont, m_toolbarHeight, static_cast<float>(m_initialSize.x));

    // m_taskListViewLeft = std::make_unique<TaskListView>(m_globalFont, 250.f); // 不再需要

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
                                    { m_simInterface->setSimulationSpeedFactor(scale); });

    m_toolbar->setPlayPauseCallback([this]()
                                    {
        auto state = m_simInterface->getSimulationState();
        if (state.isPaused) {
            m_simInterface->resumeSimulation();
        } else {
            m_simInterface->pauseSimulation();
        } });

    m_toolbar->setResetViewCallback([this]()
                                    {
        if (m_simView) {
            m_simView->resetDefaultView();
            std::cout << "Toolbar: Reset View button clicked." << std::endl;
        } });

    m_toolbar->setAddTaskCallback([]()
                                  {
        // TODO: 实现添加任务的逻辑，例如弹出一个对话框
        std::cout << "Toolbar: Add Task button clicked. (Not implemented yet)" << std::endl; });

    m_toolbar->setSwitchModeCallback([]()
                                     {
        // TODO: 实现切换模式的逻辑
        std::cout << "Toolbar: Switch Mode button clicked. (Not implemented yet)" << std::endl; });

    // 更新布局
    updateLayout();
}

void MainWindow::runEventLoop()
{
    sf::Clock frameClock;

    while (isOpen())
    {
        sf::Event event;
        while (pollEvent(event))
        {
            handleSystemEvent(event);
        }

        float deltaTime = frameClock.restart().asSeconds();
        if (m_simInterface)
            m_simInterface->update(sf::seconds(deltaTime)); // 更新仿真

        renderFrame(); // 统一调用渲染
    }
}

void MainWindow::renderFrame()
{
    clear(m_backgroundColor);

    // 更新视图变换 (如果 SimulationView 需要平滑过渡或动画)
    // float deltaTime = ... ; // 如果需要，需要获取deltaTime
    // if (m_simView) m_simView->updateViewTransforms(deltaTime); // 暂时不传递deltaTime

    // 绘制工具栏
    if (m_toolbar)
    {
        m_toolbar->render(*this, sf::Vector2f(0, 0));
    }

    // 绘制仿真视图 (在工具栏下方，状态面板左侧)
    if (m_simView)
    {
        // SimulationView 绘制时会使用它自己的视口设置
        m_simView->renderWorld(*this); // 绘制世界内容
        // m_simView->renderUI(*this); // 如果有单独的UI层
    }

    // 绘制状态面板 (在工具栏下方，靠右)
    if (m_statusPanel)
    {
        sf::Vector2f statusPanelPos(getSize().x - m_statusPanel->getPanelWidth(), m_toolbarHeight);
        m_statusPanel->render(*this, statusPanelPos);
    }

    display();
}

void MainWindow::handleSystemEvent(const sf::Event &event)
{
    if (event.type == sf::Event::Closed)
    {
        close();
        return;
    }

    if (event.type == sf::Event::Resized)
    {
        m_initialSize = sf::Vector2u(event.size.width, event.size.height);
        setView(sf::View(sf::FloatRect(0.f, 0.f, static_cast<float>(event.size.width), static_cast<float>(event.size.height))));
        updateLayout();
        return;
    }

    // 将窗口鼠标位置转换为全局坐标，供各组件使用
    // sf::Vector2f mousePosWindow = static_cast<sf::Vector2f>(sf::Mouse::getPosition(*this));
    // 使用事件中的鼠标位置，如果可用，否则使用 sf::Mouse::getPosition
    sf::Vector2f mousePosWindow;
    if (event.type == sf::Event::MouseMoved)
    {
        mousePosWindow = sf::Vector2f(static_cast<float>(event.mouseMove.x), static_cast<float>(event.mouseMove.y));
    }
    else if (event.type == sf::Event::MouseButtonPressed || event.type == sf::Event::MouseButtonReleased)
    {
        mousePosWindow = sf::Vector2f(static_cast<float>(event.mouseButton.x), static_cast<float>(event.mouseButton.y));
    }
    else if (event.type == sf::Event::MouseWheelScrolled)
    {
        mousePosWindow = sf::Vector2f(static_cast<float>(event.mouseWheelScroll.x), static_cast<float>(event.mouseWheelScroll.y));
    }
    else
    {
        // 对于其他不含鼠标位置的事件，我们可能不需要mousePosWindow，
        // 或者某些组件可能仍然依赖于全局鼠标状态。为安全起见，可以获取当前位置。
        mousePosWindow = static_cast<sf::Vector2f>(sf::Mouse::getPosition(*this));
    }

    // 1. 工具栏事件处理
    // 工具栏在顶部，区域 y < m_toolbarHeight
    sf::FloatRect toolbarArea(0, 0, static_cast<float>(getSize().x), m_toolbarHeight);
    if (m_toolbar && toolbarArea.contains(mousePosWindow))
    {
        if (m_toolbar->handleEvent(event, mousePosWindow))
            return; // 事件被消耗
    }

    // 2. 状态面板事件处理
    // 状态面板在右侧，区域 x > getSize().x - m_statusPanelWidth 且 y > m_toolbarHeight
    if (m_statusPanel)
    {
        float panelWidth = m_statusPanel->getPanelWidth();
        sf::FloatRect statusPanelArea(getSize().x - panelWidth, m_toolbarHeight,
                                      panelWidth, getSize().y - m_toolbarHeight);

        if (statusPanelArea.contains(mousePosWindow))
        {
            // 将鼠标坐标转换为 StatusPanel 的局部坐标
            sf::Vector2f statusPanelLocalMousePos = mousePosWindow - sf::Vector2f(statusPanelArea.left, statusPanelArea.top);
            if (m_statusPanel->handleEvent(event, statusPanelLocalMousePos))
                return; // 事件被消耗
        }
    }

    // 3. 仿真视图事件处理
    // 仿真视图在工具栏下方，状态面板左侧
    if (m_simView)
    {
        float panelWidth = m_statusPanel ? m_statusPanel->getPanelWidth() : 0.f;
        sf::FloatRect simViewArea(0, m_toolbarHeight,
                                  getSize().x - panelWidth, getSize().y - m_toolbarHeight);
        if (simViewArea.contains(mousePosWindow))
        {
            // SimulationView 的 handleViewEvent 通常期望的是窗口坐标或相对于其视口的坐标。
            // 如果它内部处理坐标转换 (例如使用 mapPixelToCoords)，直接传递 mousePosWindow 即可。
            // 如果它期望的是相对于simViewArea左上角的坐标，需要转换。
            // 当前假设 handleViewEvent 可以处理窗口坐标，并结合自身视口。
            m_simView->handleViewEvent(event, mousePosWindow);
            // 注意：SimView的事件处理通常不应该阻止其他UI组件的事件，除非是模态交互。
            // 因此，这里不 `return`，除非SimView明确表示消耗了事件。
        }
    }
}

void MainWindow::updateLayout()
{
    if (!isOpen())
        return; // 窗口未创建则不进行布局

    sf::Vector2u windowSize = getSize();

    // 更新工具栏宽度
    if (m_toolbar)
    {
        m_toolbar->resize(static_cast<float>(windowSize.x));
    }

    // 更新状态面板高度
    float statusPanelEffectiveWidth = 0.f;
    if (m_statusPanel)
    {
        float statusPanelHeight = static_cast<float>(windowSize.y) - m_toolbarHeight;
        m_statusPanel->resize(statusPanelHeight); // StatusPanel 根据高度和固定宽度调整内部
        statusPanelEffectiveWidth = m_statusPanel->getPanelWidth();
    }

    // 更新仿真视图视口和大小
    if (m_simView)
    {
        float simViewX = 0;
        float simViewY = m_toolbarHeight;
        float simViewWidth = static_cast<float>(windowSize.x) - statusPanelEffectiveWidth;
        float simViewHeight = static_cast<float>(windowSize.y) - m_toolbarHeight;

        if (simViewWidth < 0)
            simViewWidth = 0;
        if (simViewHeight < 0)
            simViewHeight = 0;

        // 视口是相对于窗口的归一化坐标 (0->1)
        sf::FloatRect simViewport(
            simViewX / windowSize.x,
            simViewY / windowSize.y,
            simViewWidth / windowSize.x,
            simViewHeight / windowSize.y);
        m_simView->updateViewport(simViewport);
        // SimulationView 内部的 initialize 或 updateViewport 也应该更新其 m_worldView 的尺寸和中心以匹配新的区域
        // 例如, m_simView->setSize(sf::Vector2f(simViewWidth, simViewHeight));
    }
}

void MainWindow::onSimulationStateUpdate(const SimulationInterface::SimulationState &state)
{
    if (m_statusPanel)
    {
        m_statusPanel->setSimulationTime(state.simulationTime);
        m_statusPanel->setVehicleCount(state.vehicleCount);
        m_statusPanel->setCompletedTaskCount(state.completedTaskCount);
        // PendingTaskCount 将由 refreshContent 中的 getAllTasks 确定

        // 获取选中的对象和所有任务来刷新StatusPanel
        const gui::SimObject *selectedObject = nullptr;
        if (m_simView)
        {
            selectedObject = m_simView->getSelectedObject();
        }
        std::vector<Core::Task> allTasks; // Core::Task
        if (m_simInterface)
        {
            allTasks = m_simInterface->getAllTasks(); // 假设接口提供此方法
        }
        m_statusPanel->refreshContent(selectedObject, allTasks);
        m_statusPanel->setPendingTaskCount(allTasks.size()); // 确保这个也被更新
    }

    if (m_toolbar)
    {
        m_toolbar->updateTimeDisplay(state.simulationTime, 0); // 假设没有真实时间显示
        m_toolbar->updatePlayPauseState(!state.isPaused);
        m_toolbar->updateTimeScale(state.simulationSpeedFactor);
    }
}

void MainWindow::onVehicleUpdate(const std::vector<gui::VehicleState> &vehicles)
{
    if (m_simView)
    {
        m_simView->updateVehicles(vehicles);
    }
}

void MainWindow::onDeviceUpdate(const std::vector<gui::DeviceState> &devices)
{
    if (m_simView)
    {
        m_simView->updateDevices(devices);
    }
}

// 这个processEvent是用于GUITestMain.cpp调用的版本
void MainWindow::processEvent(const sf::Event &event)
{
    handleSystemEvent(event);
}

// GUITestMain.cpp 不再直接调用 display()，由 renderFrame 包含
// display() 方法是 sf::RenderWindow 的一部分，MainWindow 继承了它

// 这个老的 runEventLoop 也可以保留，如果 MainWindow 自身可以独立运行
// 但在 GUITestMain.cpp 的驱动下，主要是 processEvent 和 renderFrame 被调用