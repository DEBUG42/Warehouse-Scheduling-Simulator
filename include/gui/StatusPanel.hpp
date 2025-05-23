#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>
#include "../Core/Task.hpp"
#include "gui/TaskListView.hpp"
#include "gui/ObjectInspector.hpp"
#include "gui/SimObject.hpp"
#include "gui/DeviceState.hpp"

/**
 * @brief 状态面板类
 *
 * 位于界面右侧，用于显示当前选中对象的详细信息
 * 包括对象类型、ID、状态属性以及相关任务列表
 * 根据不同对象类型(车辆/设备)动态调整显示内容
 */
class StatusPanel
{
private:
    // 布局参数
    const float m_panelWidth = 300.0f;                // 面板宽度
    float m_panelHeight = 600.0f;                     // 面板高度（可调整）
    const sf::Color m_backgroundColor{230, 240, 230}; // 背景色 - 更改为浅绿色
    const float m_padding = 10.0f;                    // 内部边距
    const float m_lineSpacing = 18.0f;                // 行间距

    // 字体引用
    sf::Font &m_font;

    // 内容元素
    std::unique_ptr<TaskListView> m_taskListView;       // 任务队列视图
    std::unique_ptr<ObjectInspector> m_objectInspector; // 对象详细信息

    // 存储的状态信息文本
    sf::Text m_simTimeDisplay;
    sf::Text m_vehicleCountDisplay;
    sf::Text m_completedTasksDisplay;
    sf::Text m_pendingTasksDisplay;

public:
    /**
     * @brief 构造函数
     * @param font 字体引用
     */
    StatusPanel(sf::Font &font);

    /**
     * @brief 更新面板显示内容
     * @param selectedObject 当前选中的对象（可为nullptr）
     * @param pendingTasks 全局待处理任务队列
     */
    void refreshContent(const gui::SimObject *selectedObject,
                        const std::vector<Core::Task> &pendingTasks);

    /**
     * @brief 渲染面板界面
     * @param target 渲染目标
     * @param position 面板左上角位置
     */
    void render(sf::RenderTarget &target, const sf::Vector2f &position);

    /**
     * @brief 处理面板区域内的输入事件
     * @param event SFML事件对象
     * @param localPos 相对面板的鼠标位置
     * @return 是否消耗了事件
     */
    bool handleEvent(const sf::Event &event, const sf::Vector2f &localPos);

    /**
     * @brief 获取面板宽度
     * @return 面板宽度（像素）
     */
    float getPanelWidth() const;

    /**
     * @brief 调整面板高度
     * @param height 新的高度值
     */
    void resize(float height);

    /**
     * @brief 设置仿真时间
     * @param time 仿真时间（秒）
     */
    void setSimulationTime(float time);

    /**
     * @brief 设置车辆数量
     * @param count 车辆数量
     */
    void setVehicleCount(size_t count);

    /**
     * @brief 设置已完成任务数量
     * @param count 已完成任务数量
     */
    void setCompletedTaskCount(size_t count);

    /**
     * @brief 设置待处理任务数量
     * @param count 待处理任务数量
     */
    void setPendingTaskCount(size_t count);
};