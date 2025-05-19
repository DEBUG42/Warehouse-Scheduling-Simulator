#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include "SimulationView.hpp"
#include "StatusPanel.hpp"
#include "Toolbar.hpp"

class TestSimulationEngine;

class MainWindow : public sf::RenderWindow
{
private:
    // 窗口布局参数
    sf::Vector2u m_initialSize{1280, 720}; // 初始分辨率
    const float m_toolbarHeight = 30.0f;   // 工具栏高度

    // 子视图组件
    std::unique_ptr<SimulationView> m_simView;  // 仿真视图区域
    std::unique_ptr<StatusPanel> m_statusPanel; // 右侧状态面板
    std::unique_ptr<Toolbar> m_toolbar;         // 顶部工具栏

    // 样式资源
    sf::Font m_globalFont;                   // 全局字体
    sf::Color m_backgroundColor{45, 50, 55}; // 背景色

public:    /**
     * @brief 初始化窗口布局
     * @param engine 仿真引擎引用（用于数据绑定）
     */
    void initialize(TestSimulationEngine &engine);

    /**
     * @brief 处理窗口事件循环
     */
    void runEventLoop();

private:
    /**
     * @brief 处理SFML原生事件
     * @param event SFML事件对象
     */
    void handleSystemEvent(const sf::Event &event);

    /**
     * @brief 更新窗口布局
     * 在窗口大小改变时调用
     */
    void updateLayout();
};