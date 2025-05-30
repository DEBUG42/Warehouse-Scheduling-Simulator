#include "gui/TaskListView.hpp"
#include <sstream>  // 用于格式化字符串
#include <iomanip>  // 用于 std::setw, std::setfill
#include <iostream> // 用于 handleClick 调试输出

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

    // m_titleText.setFillColor(sf::Color::White);
    m_titleText.setFillColor(sf::Color(50, 50, 50)); // 确保其背景透明或与 StatusPanel 的背景色协调。

    m_titleText.setString("Task Queue");
    // m_height 将由 StatusPanel 通过 setViewHeight 设置，或者使用默认值
}

/**
 * @brief 更新要显示的任务列表
 * @param tasks 新的任务列表
 */
void TaskListView::updateTasks(const std::vector<std::string> &tasks)
{
    // This version can be kept for compatibility or removed if only std::vector<Task> is used.
    // For now, let's clear m_tasksData if this is called, to avoid confusion.
    m_tasksData.clear();
    // If you want to convert string tasks to Task objects, implement that logic here.
    // For this example, we assume this string version is less detailed.
    // m_tasks = tasks; // If you still need m_tasks for some reason.

    // To make it functional with strings, we can create placeholder Task objects
    for (const auto &s_task : tasks)
    {
        Task t;                 // Default task
        t.id = -1;              // Indicate it's a string-based placeholder
        t.material_id = s_task; // Store the string here for display
        m_tasksData.push_back(t);
    }
    m_totalContentHeight = m_tasksData.size() * m_itemHeight;
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
 * @brief 更新要显示的任务列表 (with actual Task objects)
 * @param tasksData 新的任务列表
 */
void TaskListView::updateTasks(const std::vector<Task> &tasksData)
{
    m_tasksData = tasksData;
    m_totalContentHeight = m_tasksData.size() * m_itemHeight;
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
    states.transform *= getTransform(); // 应用 TaskListView 自身的位置变换    // 绘制标题
    sf::Text currentTitle = m_titleText;
    // currentTitle.setPosition(0, 0); // 假设TaskListView的位置已经通过states.transform设置好
    target.draw(currentTitle, states);

    // 创建一个剪裁区域，只显示视图范围内的任务项
    // (当前实现是只绘制可见项，而不是严格的像素剪裁)
    float titleHeight = 20.0f; // 标题区域的高度，需要与绘制时的 yPos 初始值匹配
    float yPos = titleHeight;  // 任务项从标题下方开始绘制，相对于TaskListView的(0,0)
    int drawnCount = 0;

    for (size_t i = 0; i < m_tasksData.size(); ++i)
    {
        const auto &task = m_tasksData[i];
        float itemTopY_local = yPos - m_scrollOffset;            // 任务项顶部在TaskListView滚动视图内的Y坐标
        float itemBottomY_local = itemTopY_local + m_itemHeight; // 任务项底部

        // 只绘制在可视区域内的任务项
        // 可视区域的顶部是 titleHeight，底部是 m_height
        if (itemBottomY_local > titleHeight && itemTopY_local < m_height)
        {
            std::string taskLine;
            if (task.id != -1)
            { // Full Task object
                taskLine = "ID:" + std::to_string(task.id) +
                           (task.type == TaskType::INBOUND ? " IN " : " OUT ") +
                           "M:" + task.material_id +
                           " S:" + std::to_string(task.start_device_id) +
                           " E:" + std::to_string(task.end_device_id);
                if (task.is_assigned && task.assigned_vehicle_id != -1)
                {
                    taskLine += " (V:" + std::to_string(task.assigned_vehicle_id) + ")";
                }
                else
                {
                    taskLine += " (Unassigned)";
                }
            }
            else
            {                                // Placeholder string task
                taskLine = task.material_id; // The string was stored in material_id
            }

            sf::Text taskText(taskLine, m_font, 12);      // 任务文本字号
            taskText.setFillColor(sf::Color(70, 70, 70)); // 深灰色任务文本
            taskText.setPosition(5.0f, itemTopY_local);

            target.draw(taskText, states);
            drawnCount++;
        }
        yPos += m_itemHeight; // 移动到下一个任务项的绘制基准线 (无滚动时的位置)
    }
    // 如果需要，可以绘制滚动条指示器，这里暂时省略
    // std::cout << "[调试] TaskListView drawn items: " << drawnCount << " / " << m_tasks.size() << std::endl;
}

/**
 * @brief 处理对任务列表的点击事件
 * @param localMousePos 鼠标点击位置 (相对于 TaskListView 的局部坐标)
 * @return 如果点击了某个任务项则返回 true，否则 false
 */
bool TaskListView::handleClick(const sf::Vector2f &localMousePos)
{
    // 遍历所有任务项，检查点击位置是否在某个任务项的边界内
    // 需要考虑滚动偏移 m_scrollOffset 和标题 m_titleText 的高度

    float titleHeight = 20.0f; // 与 draw 方法中使用的标题高度一致
    if (localMousePos.y < titleHeight)
    {
        // 点击在了标题区域，不处理或作其他响应
        // std::cout << "[调试] TaskListView: Clicked on title area." << std::endl;
        return false;
    }
    float currentItemY = titleHeight; // 第一个任务项的顶部Y坐标（无滚动时）
    for (size_t i = 0; i < m_tasksData.size(); ++i)
    {
        const auto &task = m_tasksData[i]; // Use m_tasksData
        // 计算当前任务项在视图中的实际显示边界 (考虑滚动)
        float itemTopInView = currentItemY - m_scrollOffset;
        float itemBottomInView = itemTopInView + m_itemHeight;

        // 构建该任务项的局部边界框 (相对于TaskListView的0,0)
        // X从0到m_width，Y从itemTopInView到itemBottomInView
        sf::FloatRect itemBounds(0.f, itemTopInView, m_width, m_itemHeight);

        // 只检查在可视区域内的项 (itemBottomInView > titleHeight && itemTopInView < m_height)
        // 并且鼠标点击在该项的边界内
        if (itemBottomInView > titleHeight && itemTopInView < m_height && itemBounds.contains(localMousePos))
        {
            std::string taskDesc = (task.id != -1) ? ("Task ID " + std::to_string(task.id)) : task.material_id;
            std::cout << "[调试] TaskListView: Clicked on " << taskDesc
                      << " (Mouse Y: " << localMousePos.y << ", Item Top: " << itemTopInView << ")"
                      << std::endl;
            // TODO: 在这里可以触发更复杂的操作，例如通知外部监听器，或改变任务项的显示状态
            return true; // 事件被消耗
        }
        currentItemY += m_itemHeight; // 移动到下一个任务项的基准Y坐标
    }

    // std::cout << "[调试] TaskListView: Clicked on empty area or outside visible items." << std::endl;
    return false; // 没有点击到任何任务项
}