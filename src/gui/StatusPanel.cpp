#include "gui/StatusPanel.hpp"
#include "gui/SimObject.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>
// TaskList.hpp 已经在 StatusPanel.hpp 中包含

StatusPanel::StatusPanel(sf::Font &font) : m_font(font)
{
    // 初始化对象检视器
    m_objectInspector = std::make_unique<ObjectInspector>(m_font, m_panelWidth - 2 * m_padding);

    // 初始化状态显示文本对象
    m_simTimeDisplay.setFont(m_font);
    m_simTimeDisplay.setCharacterSize(12);
    m_simTimeDisplay.setFillColor(sf::Color::White);

    m_vehicleCountDisplay.setFont(m_font);
    m_vehicleCountDisplay.setCharacterSize(12);
    m_vehicleCountDisplay.setFillColor(sf::Color::White);

    m_completedTasksDisplay.setFont(m_font);
    m_completedTasksDisplay.setCharacterSize(12);
    m_completedTasksDisplay.setFillColor(sf::Color::White);

    m_pendingTasksDisplay.setFont(m_font);
    m_pendingTasksDisplay.setCharacterSize(12);
    m_pendingTasksDisplay.setFillColor(sf::Color::White);

    // 初始内容设置
    setSimulationTime(0.0f);
    setVehicleCount(0);
    setCompletedTaskCount(0);
    setPendingTaskCount(0);
}

void StatusPanel::refreshContent(const gui::SimObject *selectedObject,
                                 const std::vector<Core::Task> &pendingTasks)
{
    // 更新对象检视器
    m_objectInspector->updateObject(selectedObject);
    setPendingTaskCount(pendingTasks.size()); // 顺便更新待处理任务数
}

/**
 * @brief 渲染面板界面
 *
 * 绘制状态面板的背景和所有UI组件
 * 包括对象信息区域和任务列表区域
 *
 * @param target 渲染目标
 * @param position 面板左上角位置
 */
void StatusPanel::render(sf::RenderTarget &target, const sf::Vector2f &position)
{
    // 绘制面板背景
    sf::RectangleShape background(sf::Vector2f(m_panelWidth, m_panelHeight));
    background.setPosition(position);
    background.setFillColor(m_backgroundColor);
    target.draw(background);

    // 定义各区域的高度比例和实际高度
    float globalStatusHeight = m_lineSpacing * 4 + m_padding * 2;      // 全局状态显示区预估高度
    float inspectorHeight = m_panelHeight * 0.3f - globalStatusHeight; // 检视器占30%减去全局状态区
    if (inspectorHeight < 50)
        inspectorHeight = 50;                                                                    // 最小高度
    float taskListHeight = m_panelHeight - globalStatusHeight - inspectorHeight - m_padding * 2; // 剩余给任务列表
    if (taskListHeight < 100)
        taskListHeight = 100; // 任务列表最小高度

    // 确保总高度不超过面板高度，调整inspectorHeight (如果taskList过小)
    if (globalStatusHeight + inspectorHeight + taskListHeight > m_panelHeight - m_padding * 2)
    {
        inspectorHeight = m_panelHeight - m_padding * 2 - globalStatusHeight - taskListHeight;
    }

    // 1. 绘制全局状态信息
    sf::Transform globalTransform;
    globalTransform.translate(position.x + m_padding, position.y + m_padding);
    target.draw(m_simTimeDisplay, globalTransform);
    globalTransform.translate(0, m_lineSpacing);
    target.draw(m_vehicleCountDisplay, globalTransform);
    globalTransform.translate(0, m_lineSpacing);
    target.draw(m_completedTasksDisplay, globalTransform);
    globalTransform.translate(0, m_lineSpacing);
    target.draw(m_pendingTasksDisplay, globalTransform);

    // 2. 绘制检视器
    sf::Transform inspectorTransform;
    inspectorTransform.translate(position.x + m_padding, position.y + m_padding + globalStatusHeight);
    m_objectInspector->setPosition(inspectorTransform.transformPoint(0, 0)); // 设置检视器的位置
    target.draw(*m_objectInspector);                                         // 注意 Inspector 是 sf::Drawable

    // 3. 绘制分隔线
    sf::RectangleShape divider(sf::Vector2f(m_panelWidth - 2 * m_padding, 1.0f));
    divider.setPosition(position.x + m_padding, position.y + m_padding + globalStatusHeight + inspectorHeight);
    divider.setFillColor(sf::Color(70, 70, 70));
    target.draw(divider);
}

bool StatusPanel::handleEvent(const sf::Event &event, const sf::Vector2f &panelLocalMousePos)
{
    // panelLocalMousePos 是相对于 StatusPanel 左上角 (0,0) 的坐标

    // 计算各子组件的相对区域和鼠标位置
    float globalStatusHeight = m_lineSpacing * 4 + m_padding * 2;
    float inspectorHeight = m_panelHeight * 0.3f - globalStatusHeight;
    if (inspectorHeight < 50)
        inspectorHeight = 50;
    // (省略 taskListHeight 和 inspectorHeight 的动态调整逻辑，假设它们已确定)

    // 检视器区域的本地坐标系
    sf::FloatRect inspectorBounds(m_padding, m_padding + globalStatusHeight,
                                  m_panelWidth - 2 * m_padding, inspectorHeight);
    // 任务列表区域的本地坐标系
    sf::FloatRect taskListBounds(m_padding, m_padding + globalStatusHeight + inspectorHeight + 1.0f,
                                 m_panelWidth - 2 * m_padding, m_panelHeight - (m_padding + globalStatusHeight + inspectorHeight + 1.0f) - m_padding);

    // if (m_objectInspector->getGlobalBounds().contains(panelLocalMousePos)) { // 需要 inspector 提供 getGlobalBounds 或手动计算
    //     // TODO: 将事件和相对坐标传递给 m_objectInspector->handleEvent(...)
    // }

    if (event.type == sf::Event::MouseWheelScrolled)
    {
        if (taskListBounds.contains(panelLocalMousePos))
        {
            // 将鼠标位置转换为 TaskListView 的局部坐标
            sf::Vector2f taskListViewLocalMousePos = panelLocalMousePos - sf::Vector2f(taskListBounds.left, taskListBounds.top);
            if (m_taskListView->handleScrollEvent(event.mouseWheelScroll, taskListViewLocalMousePos))
            {
                return true; // 事件被任务列表消耗
            }
        }
    }
    // TODO: 其他事件处理，例如点击检视器中的按钮等
    return false; // 当前没有消耗事件
}

float StatusPanel::getPanelWidth() const
{
    return m_panelWidth;
}

void StatusPanel::resize(float height)
{
    m_panelHeight = height;
}

void StatusPanel::setSimulationTime(float time)
{
    std::ostringstream oss;
    int hours = static_cast<int>(time / 3600);
    int minutes = static_cast<int>((time - hours * 3600) / 60);
    int seconds = static_cast<int>(time) % 60;
    oss << "Simulation Time: " << std::setw(2) << std::setfill('0') << hours << ":"
        << std::setw(2) << std::setfill('0') << minutes << ":"
        << std::setw(2) << std::setfill('0') << seconds;
    m_simTimeDisplay.setString(oss.str());
}

void StatusPanel::setVehicleCount(size_t count)
{
    m_vehicleCountDisplay.setString("Total Vehicles: " + std::to_string(count));
}

void StatusPanel::setCompletedTaskCount(size_t count)
{
    m_completedTasksDisplay.setString("Completed Tasks: " + std::to_string(count));
}

void StatusPanel::setPendingTaskCount(size_t count)
{
    m_pendingTasksDisplay.setString("Pending Tasks: " + std::to_string(count));
}