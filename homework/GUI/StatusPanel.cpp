#include "StatusPanel.hpp"
#include "SimObject.hpp"
#include <iostream>
// TaskList.hpp 已经在 StatusPanel.hpp 中包含

StatusPanel::StatusPanel(sf::Font &font) : m_font(font)
{
    // 初始化任务列表视图
    m_taskList = std::make_unique<TaskListView>(m_font, m_panelWidth);

    // 初始化对象检视器
    m_inspector = std::make_unique<ObjectInspector>(m_font, m_panelWidth);
}

void StatusPanel::refreshContent(const SimObject *selectedObject,
                                 const std::vector<Task> &pendingTasks)
{
    // 更新对象检视器
    m_inspector->updateObject(selectedObject);

    // 更新任务列表
    m_taskList->updateTasks(pendingTasks);
}

void StatusPanel::render(sf::RenderTarget &target, const sf::Vector2f &position)
{
    // 绘制面板背景
    sf::RectangleShape background(sf::Vector2f(m_panelWidth, m_panelHeight));
    background.setPosition(position);
    background.setFillColor(m_backgroundColor);
    target.draw(background);

    // 绘制分隔线
    float inspectorHeight = m_panelHeight * 0.4f; // 检视器占40%高度
    sf::RectangleShape divider(sf::Vector2f(m_panelWidth, 1.0f));
    divider.setPosition(position.x, position.y + inspectorHeight);
    divider.setFillColor(sf::Color(70, 70, 70));
    target.draw(divider);

    // 绘制检视器
    m_inspector->render(target, position);

    // 绘制任务列表
    sf::Vector2f taskListPos(position.x, position.y + inspectorHeight + 1.0f);
    m_taskList->render(target, taskListPos);
}

bool StatusPanel::handleEvent(const sf::Event &event, const sf::Vector2f &localPos)
{
    // 根据需要实现事件处理
    // 例如：点击任务列表中的任务，检视器中的按钮等

    // 任务列表区域事件处理
    float inspectorHeight = m_panelHeight * 0.4f;
    if (localPos.y > inspectorHeight)
    {
        // TODO: 实现任务列表的事件处理
        return true;
    }

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