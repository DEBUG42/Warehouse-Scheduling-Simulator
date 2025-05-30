#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <memory>
#include "../../src/Core/Task.hpp" // 使用Core的Task定义

// 不再定义重复的Task结构

/**
 * @brief 任务列表视图类
 *
 * 用于在状态面板中显示任务队列。
 * 提供可滚动的任务列表展示。
 */
class TaskListView : public sf::Drawable, public sf::Transformable
{
private:
    sf::Font &m_font;        // 外部传入的字体引用
    float m_width;           // 视图宽度
    float m_height = 200.0f; // 视图高度 (StatusPanel会设置一个默认值或者通过setViewHeight更新)
    // std::vector<std::string> m_tasks; // 当前要显示的任务列表（简化版）
    std::vector<Task> m_tasksData; // Store actual Task objects

    // 滚动相关
    float m_scrollOffset = 0.0f;       // 当前滚动偏移量（像素）
    float m_totalContentHeight = 0.0f; // 所有任务项的总高度
    float m_itemHeight = 18.0f;        // 单个任务项的估算高度 (增大一点以便显示更多信息)
    const float m_scrollSpeed = 20.0f; // 每次滚动的像素数

    // 用于显示的文本元素
    sf::Text m_titleText;

public:
    /**
     * @brief 构造函数
     * @param font 字体引用
     * @param width 视图宽度
     */
    TaskListView(sf::Font &font, float width); /**
                                                * @brief 更新要显示的任务列表
                                                * @param tasks 新的任务列表 (std::vector<std::string> for now)
                                                */
    void updateTasks(const std::vector<std::string> &tasks);

    /**
     * @brief 更新要显示的任务列表 (with actual Task objects)
     * @param tasksData 新的任务列表
     */
    void updateTasks(const std::vector<Task> &tasksData);

    /**
     * @brief 处理鼠标滚轮事件以实现滚动
     * @param wheelEvent SFML滚轮事件
     * @param localMousePos 鼠标位置（相对于本视图的坐标，如果需要的话）
     * @return 如果事件被处理则返回 true
     */
    bool handleScrollEvent(const sf::Event::MouseWheelScrollEvent &wheelEvent, const sf::Vector2f &localMousePos);

    /**
     * @brief 设置视图的绘制高度
     * @param height 高度值
     */
    void setViewHeight(float height) { m_height = height; }

    /**
     * @brief 设置视图的宽度
     * @param width 宽度值
     */
    void setWidth(float width) { m_width = width; }

    /**
     * @brief 处理对任务列表的点击事件
     * @param localMousePos 鼠标点击位置 (相对于 TaskListView 的局部坐标)
     * @return 如果点击了某个任务项则返回 true，否则 false
     */
    bool handleClick(const sf::Vector2f &localMousePos);

    /**
     * @brief 绘制任务列表视图
     * @param target 渲染目标
     * @param states 渲染状态
     */
    virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const override;
};