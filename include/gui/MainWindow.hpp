#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include "SimulationView.hpp"
#include "StatusPanel.hpp"
#include "Toolbar.hpp"
#include "SimulationInterface.hpp"
#include "TaskListView.hpp"

/**
 * @class MainWindow
 * @brief 主窗口类，管理整个GUI系统
 *
 * 负责窗口创建、事件处理、布局管理和子组件协调。
 * 作为整个GUI系统的入口点，集成了仿真视图、状态面板和工具栏等组件。
 */
class MainWindow : public sf::RenderWindow
{
private:
    // 窗口布局参数
    sf::Vector2u m_initialSize{1280, 720}; // 初始分辨率
    const float m_toolbarHeight = 30.0f;   // 工具栏高度

    // 子视图组件
    std::unique_ptr<SimulationView> m_simView;        // 仿真视图区域
    std::unique_ptr<StatusPanel> m_statusPanel;       // 右侧状态面板
    std::unique_ptr<Toolbar> m_toolbar;               // 顶部工具栏
    std::unique_ptr<TaskListView> m_taskListViewLeft; // 左侧任务队列

    // 样式资源
    sf::Font m_globalFont;                   // 全局字体
    sf::Color m_backgroundColor{45, 50, 55}; // 背景色

    // 仿真接口
    std::shared_ptr<SimulationInterface> m_simInterface; // 仿真接口
    std::vector<Core::Task> m_pendingTasks;              // 当前任务队列

public:
    /**
     * @brief 构造函数
     */
    MainWindow() = default;

    /**
     * @brief 析构函数
     */
    ~MainWindow() = default;

    /**
     * @brief 初始化窗口布局（使用仿真接口）
     * @param simInterface 仿真接口指针
     */
    void initialize(std::shared_ptr<SimulationInterface> simInterface);

    /**
     * @brief (For test purposes) Processes a single SFML event.
     * @param event The SFML event to process.
     */
    void processEvent(const sf::Event &event);

    /**
     * @brief (For test purposes) Renders a single frame of the GUI.
     * Does not call display().
     */
    void renderFrame();

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

    /**
     * @brief 更新状态面板数据
     */
    void updateStatusPanel();

    /**
     * @brief 处理仿真状态更新回调
     * @param state 仿真状态
     */
    void onSimulationStateUpdate(const SimulationInterface::SimulationState &state);

    /**
     * @brief 处理车辆状态更新回调
     * @param vehicles 车辆状态列表
     */
    void onVehicleUpdate(const std::vector<gui::VehicleState> &vehicles);

    /**
     * @brief 处理设备状态更新回调
     * @param devices 设备状态列表
     */
    void onDeviceUpdate(const std::vector<gui::DeviceState> &devices);
};