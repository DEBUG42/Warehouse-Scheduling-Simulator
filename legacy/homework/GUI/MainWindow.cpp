#include "MainWindow.hpp"
#include "MockSimulationInterface.hpp"
#include <iostream>

/**
 * @brief 构造函数
 * 
 * 初始化窗口和仿真视图
 */
MainWindow::MainWindow()
{
    createWindow();
    
    // 创建模拟接口并设置到视图
    auto mockInterface = std::make_shared<MockSimulationInterface>(5, 8);
    m_simView.setSimulationInterface(mockInterface);
}

/**
 * @brief 运行窗口主循环
 * 
 * 处理事件、更新逻辑和渲染，直到窗口关闭
 */
void MainWindow::run()
{
    sf::Clock clock;
    
    while (m_isRunning && m_window.isOpen())
    {
        // 计算时间增量
        float deltaTime = clock.restart().asSeconds();
        
        // 处理事件
        processEvents();
        
        // 更新逻辑
        update(deltaTime);
        
        // 渲染
        render();
        
        // 首次运行时居中视图
        if (m_needCenterView) {
            m_simView.centerView();
            m_needCenterView = false;
        }
    }
}

/**
 * @brief 处理事件
 * 
 * 处理窗口和用户输入事件
 */
void MainWindow::processEvents()
{
    sf::Event event;
    while (m_window.pollEvent(event))
    {
        // 处理窗口事件
        switch (event.type)
        {
            case sf::Event::Closed:
                m_window.close();
                m_isRunning = false;
                break;
                
            case sf::Event::Resized:
                // 更新视图大小
                m_simView.resize(event.size.width, event.size.height);
                break;
                
            case sf::Event::MouseButtonPressed:
                m_simView.handleMouseButtonPressed(
                    sf::Vector2i(event.mouseButton.x, event.mouseButton.y),
                    event.mouseButton.button
                );
                break;
                
            case sf::Event::MouseButtonReleased:
                m_simView.handleMouseButtonReleased(
                    sf::Vector2i(event.mouseButton.x, event.mouseButton.y),
                    event.mouseButton.button
                );
                break;
                
            case sf::Event::MouseMoved:
                m_simView.handleMouseMoved(
                    sf::Vector2i(event.mouseMove.x, event.mouseMove.y)
                );
                break;
                
            case sf::Event::MouseWheelScrolled:
                m_simView.handleMouseWheelScrolled(
                    event.mouseWheelScroll.delta,
                    sf::Vector2i(event.mouseWheelScroll.x, event.mouseWheelScroll.y)
                );
                break;
                
            case sf::Event::KeyPressed:
                m_simView.handleKeyPressed(event.key.code);
                
                // 特殊键处理
                if (event.key.code == sf::Keyboard::Escape) {
                    m_window.close();
                    m_isRunning = false;
                }
                break;
                
            default:
                break;
        }
    }
}

/**
 * @brief 更新逻辑
 * @param deltaTime 时间增量（秒）
 * 
 * 更新仿真视图和其他组件
 */
void MainWindow::update(float deltaTime)
{
    m_simView.update(deltaTime);
}

/**
 * @brief 渲染窗口内容
 * 
 * 清除窗口、绘制组件并显示
 */
void MainWindow::render()
{
    m_window.clear(sf::Color(30, 30, 30));
    
    // 设置视图
    m_window.setView(m_simView.getView());
    
    // 绘制仿真视图
    m_window.draw(m_simView);
    
    // 显示窗口内容
    m_window.display();
}

/**
 * @brief 创建并设置窗口
 * 
 * 设置窗口属性和样式
 */
void MainWindow::createWindow()
{
    // 创建窗口
    m_window.create(
        sf::VideoMode(m_width, m_height),
        m_title,
        sf::Style::Default
    );
    
    // 设置窗口图标（如果有）
    // sf::Image icon;
    // if (icon.loadFromFile("resources/icon.png")) {
    //     m_window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
    // }
    
    // 设置垂直同步
    m_window.setVerticalSyncEnabled(true);
    
    // 设置窗口位置居中
    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    m_window.setPosition(sf::Vector2i(
        (desktop.width - m_width) / 2,
        (desktop.height - m_height) / 2
    ));
    
    std::cout << "窗口创建成功: " << m_width << "x" << m_height << std::endl;
}
