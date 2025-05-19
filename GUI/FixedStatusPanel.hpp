#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>
#include "../Core/Task.hpp"
#include "TaskListHeader.hpp"  // 使用我们刚刚创建的头文件

class SimObject;

class StatusPanel
{
private:
    // 布局参数
    const float m_panelWidth = 300.0f;             // 面板宽度
    float m_panelHeight = 600.0f;                  // 面板高度（可调整）
    const sf::Color m_backgroundColor{35, 40, 45}; // 背景色

    // 字体引用
    sf::Font &m_font;

    // 内容元素
    std::unique_ptr<TaskListView> m_taskList;     // 任务队列视图
    std::unique_ptr<ObjectInspector> m_inspector; // 对象详细信息

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
    void refreshContent(const SimObject *selectedObject,
                        const std::vector<Task> &pendingTasks);

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
     * @brief 设置面板高度
     * @param height 新高度值
     */
    void setHeight(float height) { m_panelHeight = height; }

    /**
     * @brief 获取面板宽度
     */
    float getWidth() const { return m_panelWidth; }
};
