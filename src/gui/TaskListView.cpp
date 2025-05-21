#include "gui/TaskListView.hpp"
#include <sstream> // 用于格式化字符串
#include <iomanip> // 用于 std::setw, std::setfill

/**
 * @brief 构造函数
 * @param font 字体引用
 * @param width 视图宽度
 */
TaskListView::TaskListView(sf::Font &font, float width)
    : m_font(font), m_width(width)
{
    m_titleText.setFont(m_font);
    m_titleText.setCharacterSize(16); // Title font size
    m_titleText.setFillColor(sf::Color::White);
    m_titleText.setString("Task Queue");
    // m_height 将由 StatusPanel 通过 setViewHeight 设置，或者使用默认值
}

/**
 * @brief 更新要显示的任务列表
 * @param tasks 新的任务列表
 */
void TaskListView::updateTasks(const std::vector<Task> &tasks)
{
    m_tasks = tasks;
    m_totalContentHeight = m_tasks.size() * m_itemHeight;
    // 重置滚动条位置，如果内容变少
    if (m_scrollOffset > m_totalContentHeight - m_height && m_totalContentHeight > m_height)
    {
        m_scrollOffset = m_totalContentHeight - m_height;
    }
    if (m_scrollOffset < 0 || m_totalContentHeight <= m_height)
    {
        m_scrollOffset = 0;
    }
}

/**
 * @brief 处理鼠标滚轮事件以实现滚动
 * @param wheelEvent SFML滚轮事件
 * @param localMousePos 鼠标位置（相对于本视图的坐标，如果需要的话）
 * @return 如果事件被处理则返回 true
 */
bool TaskListView::handleScrollEvent(const sf::Event::MouseWheelScrollEvent &wheelEvent, const sf::Vector2f &localMousePos)
{
    // 检查鼠标是否在此视图的区域内 (需要视图的全局位置和尺寸)
    // 假设 StatusPanel 会做这个检查，这里直接处理滚动逻辑
    if (m_totalContentHeight > m_height)
    { // 只有内容超出视图高度时才允许滚动
        if (wheelEvent.delta < 0)
        { // 向下滚动 (通常是滚轮向下，delta < 0)
            m_scrollOffset += m_scrollSpeed;
            if (m_scrollOffset > m_totalContentHeight - m_height)
            {
                m_scrollOffset = m_totalContentHeight - m_height;
            }
        }
        else if (wheelEvent.delta > 0)
        { // 向上滚动
            m_scrollOffset -= m_scrollSpeed;
            if (m_scrollOffset < 0)
            {
                m_scrollOffset = 0;
            }
        }
        return true; // 消耗了滚动事件
    }
    return false; // 未处理事件
}

/**
 * @brief 绘制任务列表视图
 * @param target 渲染目标
 * @param states 渲染状态
 */
void TaskListView::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    states.transform *= getTransform(); // 应用 TaskListView 自身的位置变换

    // 绘制标题
    sf::Text currentTitle = m_titleText;
    // currentTitle.setPosition(0, 0); // 假设TaskListView的位置已经通过states.transform设置好
    target.draw(currentTitle, states);

    // 创建一个剪裁区域，只显示视图范围内的任务项
    // 这需要知道 TaskListView 在窗口中的绝对位置和尺寸
    // 简单的实现方式是使用 sf::View，但这里我们手动计算和剪裁
    // 或者，更简单的方式是不做像素级剪裁，而是只绘制可见范围内的项

    float yPos = 20.0f; // 标题下方的起始Y偏移，相对于TaskListView的 (0,0)
    int drawnCount = 0;

    for (const auto &task : m_tasks)
    {
        float itemTopY = yPos - m_scrollOffset;
        float itemBottomY = itemTopY + m_itemHeight;

        // 只绘制在可视区域内的任务项 (m_height 是可视区域高度，顶部从 yPos 开始算，但要考虑标题高度)
        // 简单的可见性判断：项的顶部在视图底部之上，项的底部在视图顶部之下
        // 这里的视图顶部是标题下方，视图底部是 m_height - 标题高度区域
        if (itemBottomY > 20.0f && itemTopY < m_height)
        {
            std::ostringstream taskLine;
            taskLine << "ID: " << task.id
                     << " Type: " << (task.type == Core::TaskType::INPUT ? "Input" : (task.type == Core::TaskType::OUTPUT ? "Output" : "Move"))
                     << " Material: " << task.materialId
                     << " Start: " << task.startDeviceId
                     << " -> End: " << task.endDeviceId;

            sf::Text taskText(taskLine.str(), m_font, 12); // 任务文本字号
            taskText.setFillColor(sf::Color::White);

            sf::RenderStates itemStates = states;
            itemStates.transform.translate(5.0f, itemTopY); // 5px的左边距
            target.draw(taskText, itemStates);
            drawnCount++;
        }
        yPos += m_itemHeight; // 移动到下一个任务项的位置
    }
    // 如果需要，可以绘制滚动条指示器，这里暂时省略
}