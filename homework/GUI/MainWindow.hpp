#pragma once
#include <SFML/Graphics.hpp>
#include "SimulationView.hpp"

/**
 * @brief 主窗口类
 *
 * 负责创建和管理应用程序的主窗口
 * 处理窗口事件，如调整大小、关闭等
 * 管理模拟视图和UI组件
 */
class MainWindow
{
private:
    // 窗口设置
    sf::RenderWindow m_window;
    const sf::String m_title = L"物流穿梭车系统仿真";
    unsigned int m_width = 1280;
    unsigned int m_height = 800;

    // 视图组件
    SimulationView m_simView;

    // 状态变量
    bool m_isRunning = true;
    bool m_needCenterView = true;

public:
    /**
     * @brief 构造函数
     */
    MainWindow();

    /**
     * @brief 运行窗口主循环
     */
    void run();

    /**
     * @brief 获取是否正在运行
     * @return 窗口是否正在运行
     */
    bool isRunning() const { return m_isRunning; }

    /**
     * @brief 获取渲染窗口引用
     * @return 渲染窗口引用
     */
    sf::RenderWindow &getWindow() { return m_window; }

private:
    /**
     * @brief 处理事件
     */
    void processEvents();

    /**
     * @brief 更新逻辑
     * @param deltaTime 时间增量（秒）
     */
    void update(float deltaTime);

    /**
     * @brief 渲染窗口内容
     */
    void render();

    /**
     * @brief 创建并设置窗口
     */
    void createWindow();
};
