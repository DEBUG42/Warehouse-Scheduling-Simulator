#include "gui/StatusPanel.hpp"
#include "gui/SimObject.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>
// TaskList.hpp 已经在 StatusPanel.hpp 中包含

// 辅助调试函数：打印sf::FloatRect
void printRect(const std::string &name, const sf::FloatRect &rect)
{
    std::cout << "[调试] " << name << ": "
              << "Pos(" << rect.left << ", " << rect.top << ") "
              << "Size(" << rect.width << ", " << rect.height << ")"
              << std::endl;
}

StatusPanel::StatusPanel(sf::Font &font) : m_font(font)
{
    // m_backgroundColor 已在 .hpp 中初始化为浅绿色
    // m_padding, m_lineSpacing 也已在 .hpp 中定义

    // 1. 初始化状态显示文本对象 (颜色已在上次修改中设为深灰色)
    m_simTimeDisplay.setFont(m_font);
    m_simTimeDisplay.setCharacterSize(12);
    m_simTimeDisplay.setFillColor(sf::Color(50, 50, 50));

    m_vehicleCountDisplay.setFont(m_font);
    m_vehicleCountDisplay.setCharacterSize(12);
    m_vehicleCountDisplay.setFillColor(sf::Color(50, 50, 50));

    m_completedTasksDisplay.setFont(m_font);
    m_completedTasksDisplay.setCharacterSize(12);
    m_completedTasksDisplay.setFillColor(sf::Color(50, 50, 50));

    m_pendingTasksDisplay.setFont(m_font);
    m_pendingTasksDisplay.setCharacterSize(12);
    m_pendingTasksDisplay.setFillColor(sf::Color(50, 50, 50));

    // 初始内容设置
    setSimulationTime(0.0f);
    setVehicleCount(0);
    setCompletedTaskCount(0);
    setPendingTaskCount(0);

    // 2. 初始化对象检视器 (ObjectInspector)
    // 宽度减去左右两边的 padding
    float inspectorWidth = m_panelWidth - 2 * m_padding;
    m_objectInspector = std::make_unique<ObjectInspector>(m_font, inspectorWidth);
    // 注意: ObjectInspector 的背景色等需要其内部自行调整以适应浅色背景，或提供接口进行设置

    // 3. 初始化任务列表视图 (TaskListView)
    // 宽度与检视器相同
    float taskListWidth = m_panelWidth - 2 * m_padding;
    m_taskListView = std::make_unique<TaskListView>(m_font, taskListWidth);
    // 注意: TaskListView 内部的文本颜色等也需要调整以适应浅色背景
    // TaskListView 的高度将会在 render 或 resize 中动态计算并设置

    std::cout << "[调试] StatusPanel构造完成: PanelWidth=" << m_panelWidth << std::endl;
}

void StatusPanel::refreshContent(const gui::SimObject *selectedObject,
                                 const std::vector<Core::Task> &pendingTasks)
{
    // 更新对象检视器
    if (m_objectInspector)
    {
        m_objectInspector->updateObject(selectedObject);
    }

    // 更新待处理任务数 (全局状态)
    setPendingTaskCount(pendingTasks.size());

    // 更新任务列表视图的内容
    if (m_taskListView)
    {
        m_taskListView->updateTasks(pendingTasks); // 假设显示所有待处理任务
                                                   // 如果TaskListView需要显示特定于对象相关的任务，这里逻辑需要调整
    }
    // std::cout << "[调试] StatusPanel::refreshContent - PendingTasks: " << pendingTasks.size() << std::endl;
}

/**
 * @brief 渲染面板界面
 *
 * 绘制状态面板的背景和所有UI组件
 * 包括全局状态区、对象信息区和任务列表区
 *
 * @param target 渲染目标
 * @param panelPosition 面板左上角在窗口中的绝对位置
 */
void StatusPanel::render(sf::RenderTarget &target, const sf::Vector2f &panelPosition)
{
    // 绘制面板背景
    sf::RectangleShape background(sf::Vector2f(m_panelWidth, m_panelHeight));
    background.setPosition(panelPosition);
    background.setFillColor(m_backgroundColor);
    target.draw(background);

    // --- 布局计算 ---
    // 所有子元素的 X 坐标都是相对于 panelPosition 的，并加上内边距
    float currentX = panelPosition.x + m_padding;
    float currentY = panelPosition.y + m_padding;
    float availableWidth = m_panelWidth - 2 * m_padding; // 内部可用宽度

    // 区域1: 全局状态信息区
    // 高度由行数和行间距决定
    float globalStatusHeight = m_lineSpacing * 4; // 4行状态信息
    // (调试输出可以取消注释)
    // printRect("GlobalStatusArea (relative to panel)", sf::FloatRect(m_padding, m_padding, availableWidth, globalStatusHeight));

    sf::Transform globalTransform; // 用于状态文本的变换
    globalTransform.translate(currentX, currentY);
    target.draw(m_simTimeDisplay, globalTransform);
    globalTransform.translate(0, m_lineSpacing);
    target.draw(m_vehicleCountDisplay, globalTransform);
    globalTransform.translate(0, m_lineSpacing);
    target.draw(m_completedTasksDisplay, globalTransform);
    globalTransform.translate(0, m_lineSpacing);
    target.draw(m_pendingTasksDisplay, globalTransform);

    currentY += globalStatusHeight + m_padding; // 更新Y坐标，准备绘制下一个区域 (增加一个padding作为区域间隔)

    // 分隔线1
    sf::RectangleShape divider1(sf::Vector2f(availableWidth, 1.0f));
    divider1.setPosition(currentX, currentY - m_padding / 2.0f); // 分隔线在区域间隔的中间
    divider1.setFillColor(sf::Color(180, 180, 180));             // 浅灰色分隔线
    target.draw(divider1);

    // 区域2: 对象检视器 (ObjectInspector)
    // 预估高度：面板总高度的35% (可调整比例)
    float inspectorHeight = m_panelHeight * 0.35f;
    if (inspectorHeight < 100)
        inspectorHeight = 100; // 最小高度
    // (调试输出可以取消注释)
    // printRect("ObjectInspectorArea (relative to panel)", sf::FloatRect(m_padding, currentY - panelPosition.y, availableWidth, inspectorHeight));

    if (m_objectInspector)
    {
        // ObjectInspector 需要知道自己的位置和大小
        // 通常 ObjectInspector 自身是 sf::Drawable 和 sf::Transformable,
        // 我们设置它的位置，它内部处理绘制。
        // 或者，如果它需要一个明确的渲染区域，我们传递给它。
        // 假设 m_objectInspector 是一个 sf::Drawable
        m_objectInspector->setPosition(currentX, currentY);
        // 如果ObjectInspector需要知道其可用高度进行内部布局：
        // m_objectInspector->setHeight(inspectorHeight); // 假设有此方法
        target.draw(*m_objectInspector);
    }
    currentY += inspectorHeight + m_padding; // 更新Y坐标

    // 分隔线2
    sf::RectangleShape divider2(sf::Vector2f(availableWidth, 1.0f));
    divider2.setPosition(currentX, currentY - m_padding / 2.0f);
    divider2.setFillColor(sf::Color(180, 180, 180));
    target.draw(divider2);

    // 区域3: 任务列表视图 (TaskListView)
    // 高度占据剩余空间
    float taskListY = currentY;
    float taskListHeight = (panelPosition.y + m_panelHeight - m_padding) - taskListY; // 确保不超出底部padding
    if (taskListHeight < 80)
        taskListHeight = 80; // 最小高度
    // (调试输出可以取消注释)
    // printRect("TaskListViewArea (relative to panel)", sf::FloatRect(m_padding, taskListY - panelPosition.y, availableWidth, taskListHeight));

    if (m_taskListView)
    {
        m_taskListView->setPosition(currentX, taskListY);
        m_taskListView->setViewHeight(taskListHeight); // 设置TaskListView的可见高度
        target.draw(*m_taskListView);
    }
}

bool StatusPanel::handleEvent(const sf::Event &event, const sf::Vector2f &panelLocalMousePos)
{
    // panelLocalMousePos 是相对于 StatusPanel 左上角 (0,0) 的坐标

    // --- 布局参数 (与render方法中保持一致，或从成员变量/计算函数获取) ---
    // 这些参数用于确定事件发生在哪个子组件的区域内
    float currentX_relative = m_padding; // 子组件X起始位置 (相对于面板)
    float currentY_relative = m_padding; // 子组件Y起始位置 (相对于面板)
    float availableWidth = m_panelWidth - 2 * m_padding;

    // 区域1: 全局状态区 (通常不可交互)
    float globalStatusHeight = m_lineSpacing * 4;
    currentY_relative += globalStatusHeight + m_padding;

    // 区域2: 对象检视器
    float inspectorHeight = m_panelHeight * 0.35f;
    if (inspectorHeight < 100)
        inspectorHeight = 100;
    sf::FloatRect inspectorBounds(currentX_relative, currentY_relative, availableWidth, inspectorHeight);
    // printRect("EventCheck: InspectorBounds", inspectorBounds); // 调试

    if (m_objectInspector && inspectorBounds.contains(panelLocalMousePos))
    {
        // 将事件坐标转换为 ObjectInspector 的局部坐标
        sf::Vector2f inspectorLocalMousePos = panelLocalMousePos - sf::Vector2f(inspectorBounds.left, inspectorBounds.top);
        if (m_objectInspector->handleEvent(event, inspectorLocalMousePos))
        {
            std::cout << "[调试] 事件被 ObjectInspector 消耗。" << std::endl;
            return true; // 事件被检视器消耗
        }
    }
    currentY_relative += inspectorHeight + m_padding;

    // 区域3: 任务列表视图
    float taskListY_relative = currentY_relative;
    float taskListHeight = (m_panelHeight - m_padding) - taskListY_relative;
    if (taskListHeight < 80)
        taskListHeight = 80;
    sf::FloatRect taskListBounds(currentX_relative, taskListY_relative, availableWidth, taskListHeight);
    // printRect("EventCheck: TaskListBounds", taskListBounds); // 调试

    if (event.type == sf::Event::MouseWheelScrolled)
    {
        if (m_taskListView && taskListBounds.contains(panelLocalMousePos))
        {
            // 将鼠标位置转换为 TaskListView 的局部坐标 (TaskListView::handleScrollEvent可能需要)
            sf::Vector2f listViewLocalMousePos = panelLocalMousePos - sf::Vector2f(taskListBounds.left, taskListBounds.top);
            if (m_taskListView->handleScrollEvent(event.mouseWheelScroll, listViewLocalMousePos))
            {
                // std::cout << "[调试] Scroll event handled by TaskListView." << std::endl;
                return true; // 事件被任务列表消耗
            }
        }
    }

    // 新增：处理鼠标点击事件，并尝试传递给 TaskListView
    if (event.type == sf::Event::MouseButtonPressed)
    {
        if (event.mouseButton.button == sf::Mouse::Left)
        {
            if (m_taskListView && taskListBounds.contains(panelLocalMousePos))
            {
                sf::Vector2f listViewLocalMousePos = panelLocalMousePos - sf::Vector2f(taskListBounds.left, taskListBounds.top);
                // 假设 TaskListView 有一个 handleClick 方法
                if (m_taskListView->handleClick(listViewLocalMousePos))
                {
                    std::cout << "[调试] 点击事件被 TaskListView 消耗。" << std::endl;
                    return true;
                }
            }
        }
    }

    // TODO: 其他事件处理，例如点击检视器中的按钮等
    // std::cout << "[调试] StatusPanel::handleEvent - EventType: " << event.type << " MousePos: (" << panelLocalMousePos.x << "," << panelLocalMousePos.y << ")" << std::endl;
    return false; // 当前没有消耗事件 (除了TaskListView的滚动)
}

float StatusPanel::getPanelWidth() const
{
    // std::cout << "[调试] getPanelWidth called, returning: " << m_panelWidth << std::endl;
    return m_panelWidth;
}

void StatusPanel::resize(float height)
{
    m_panelHeight = height;
    // 当面板高度变化时，可能需要重新计算并设置 TaskListView 和 ObjectInspector 的高度
    // 这个逻辑可以在 render 中处理，或者在这里显式调用一个 updateLayout 之类的私有方法
    // std::cout << "[调试] StatusPanel::resize to Height: " << m_panelHeight << std::endl;
}

void StatusPanel::setSimulationTime(float time)
{
    std::ostringstream oss;
    int hours = static_cast<int>(time / 3600);
    int minutes = static_cast<int>((time - hours * 3600) / 60);
    int seconds = static_cast<int>(time) % 60;
    oss << "仿真时间: " << std::setw(2) << std::setfill('0') << hours << ":"
        << std::setw(2) << std::setfill('0') << minutes << ":"
        << std::setw(2) << std::setfill('0') << seconds;
    m_simTimeDisplay.setString(oss.str());
}

void StatusPanel::setVehicleCount(size_t count)
{
    m_vehicleCountDisplay.setString("车辆总数: " + std::to_string(count));
}

void StatusPanel::setCompletedTaskCount(size_t count)
{
    m_completedTasksDisplay.setString("已完成任务: " + std::to_string(count));
}

void StatusPanel::setPendingTaskCount(size_t count)
{
    m_pendingTasksDisplay.setString("待处理任务: " + std::to_string(count));
}