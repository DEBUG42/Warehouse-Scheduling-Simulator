#include "MainWindow.hpp"
#include "SimObject.hpp"
#include "TestSimulationEngine.hpp"
#include <iostream>

void MainWindow::initialize(TestSimulationEngine &engine)
{
    // 创建窗口
    create(sf::VideoMode(m_initialSize.x, m_initialSize.y), "仓储穿梭车仿真系统",
           sf::Style::Default);
    setFramerateLimit(60);

    // 加载全局字体
    if (!m_globalFont.loadFromFile("resources/fonts/simhei.ttf"))
    {
        if (!m_globalFont.loadFromFile("resources/fonts/arial.ttf"))
        {
            std::cerr << "无法加载字体文件！" << std::endl;
        }
    }

    // 初始化子组件
    m_simView = std::make_unique<SimulationView>();
    m_simView->initialize(m_globalFont, engine);

    m_statusPanel = std::make_unique<StatusPanel>(m_globalFont);
    m_toolbar = std::make_unique<Toolbar>(m_globalFont, m_toolbarHeight, m_initialSize.x);

    // 设置工具栏回调
    m_toolbar->setTimeScaleCallback([&engine](float scale)
                                    { engine.setTimeScale(scale); });

    m_toolbar->setPlayPauseCallback([&engine]()
                                    {
        // 切换仿真暂停/运行状态
        static bool paused = false;
        paused = !paused;
        engine.setPaused(paused); });

    // 设置视图大小
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
        event.type == sf::Event::MouseButtonReleased)
    {
        mousePos = sf::Vector2f(sf::Mouse::getPosition(*this));
    }

    // 分发事件到子组件
    // 首先是工具栏，因为它在顶层
    if (m_toolbar->handleEvent(event, mousePos))
    {
        return; // 事件被工具栏处理
    }

    // 如果鼠标在状态面板区域，传递给状态面板
    sf::FloatRect statusPanelBounds(m_initialSize.x - m_statusPanel->getPanelWidth(),
                                    m_toolbarHeight,
                                    m_statusPanel->getPanelWidth(),
                                    m_initialSize.y - m_toolbarHeight);
    if (statusPanelBounds.contains(mousePos))
    {
        sf::Vector2f localPos = mousePos - sf::Vector2f(statusPanelBounds.left, statusPanelBounds.top);
        m_statusPanel->handleEvent(event, localPos);
        return;
    }

    // 最后传递给仿真视图
    m_simView->handleViewEvent(event, mousePos);
}

void MainWindow::updateLayout()
{
    // 更新视口
    setView(sf::View(sf::FloatRect(0, 0, m_initialSize.x, m_initialSize.y)));

    // 调整工具栏宽度
    m_toolbar->resize(m_initialSize.x);

    // 更新状态面板位置和高度
    m_statusPanel->resize(m_initialSize.y - m_toolbarHeight);

    // 更新仿真视图尺寸
    sf::FloatRect viewportRect(0,
                               m_toolbarHeight / m_initialSize.y,
                               (m_initialSize.x - m_statusPanel->getPanelWidth()) / m_initialSize.x,
                               (m_initialSize.y - m_toolbarHeight) / m_initialSize.y);
    m_simView->updateViewport(viewportRect);
}