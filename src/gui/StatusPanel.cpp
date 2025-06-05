#include "gui/StatusPanel.hpp"
#include "gui/UIControls.hpp" // For UIUtils functions
#include "../Core/Task.hpp"   // For Task class definition
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cmath>

StatusPanel::StatusPanel(sf::Font &font) : m_font(font)
{
    // Initialize background shapes
    m_background.setFillColor(m_backgroundColor);
    m_headerBackground.setFillColor(m_headerColor);

    // Initialize header text
    m_headerText.setFont(m_font);
    m_headerText.setCharacterSize(16);
    m_headerText.setFillColor(m_accentColor);
    m_headerText.setString("System Status");
    m_headerText.setStyle(sf::Text::Bold);

    // Initialize status display texts with modern styling
    m_simTimeDisplay.setFont(m_font);
    m_simTimeDisplay.setCharacterSize(14);
    m_simTimeDisplay.setFillColor(m_textColor);

    m_vehicleCountDisplay.setFont(m_font);
    m_vehicleCountDisplay.setCharacterSize(14);
    m_vehicleCountDisplay.setFillColor(m_textColor);

    m_completedTasksDisplay.setFont(m_font);
    m_completedTasksDisplay.setCharacterSize(14);
    m_completedTasksDisplay.setFillColor(m_textColor);

    m_pendingTasksDisplay.setFont(m_font);
    m_pendingTasksDisplay.setCharacterSize(14);
    m_pendingTasksDisplay.setFillColor(m_textColor);

    // Set initial content
    setSimulationTime(0.0f);
    setVehicleCount(0);
    setCompletedTaskCount(0);
    setPendingTaskCount(0);

    // Initialize ObjectInspector with updated width
    float inspectorWidth = m_panelWidth - 2 * m_padding;
    m_objectInspector = std::make_unique<ObjectInspector>(m_font, inspectorWidth);

    // Initialize TaskListView with updated width
    m_taskListView = std::make_unique<TaskListView>(m_font, inspectorWidth);

    calculateLayout();
    updateBackgroundShapes();

    std::cout << "[Debug] StatusPanel construction completed: PanelWidth=" << m_panelWidth << std::endl;
}

void StatusPanel::calculateLayout()
{
    // Calculate section heights based on 25:35:40 ratio
    float headerHeight = 40.0f;
    float globalStatusHeight = 80.0f;                                                                                        // Space for time, vehicle count, etc.
    float availableContentHeight = m_panelHeight - 3 * m_padding - headerHeight - globalStatusHeight - 2 * m_sectionSpacing; // Account for separators

    float inspectorHeight = availableContentHeight * 0.35f; // 35%
    float taskListHeight = availableContentHeight * 0.25f;  // 25%
    // float vehicleInfoHeight = availableContentHeight * 0.40f; // 40% - This seems to be a misunderstanding, StatusPanel doesn't directly manage a VehicleInfoPanel part like this.

    // Update background shapes
    m_background.setSize(sf::Vector2f(m_panelWidth, m_panelHeight));
    m_headerBackground.setSize(sf::Vector2f(m_panelWidth, headerHeight));

    // Update sizes of sub-components
    if (m_objectInspector)
    {
        m_objectInspector->setSize(m_panelWidth - 2 * m_padding, inspectorHeight);
    }
    if (m_taskListView)
    {
        m_taskListView->setViewHeight(taskListHeight);          // TaskListView uses setViewHeight
        m_taskListView->setWidth(m_panelWidth - 2 * m_padding); // Ensure width is also set
    }
    // Create separators
    m_sectionSeparators.clear();
    for (int i = 0; i < 2; ++i)
    { // Only need 2 separators
        sf::RectangleShape separator;
        separator.setFillColor(m_separatorColor);
        separator.setSize(sf::Vector2f(m_panelWidth - 2 * m_padding, 2.0f));
        m_sectionSeparators.push_back(separator);
    }
}

void StatusPanel::updateBackgroundShapes()
{
    // This will be called when position changes during render
}

sf::Color StatusPanel::getOptimalTextColor(const sf::Color &backgroundColor) const
{
    // Calculate brightness
    float brightness = (backgroundColor.r * 0.299f + backgroundColor.g * 0.587f + backgroundColor.b * 0.114f) / 255.0f;

    // Return light text for dark backgrounds, dark text for light backgrounds
    if (brightness < 0.5f)
    {
        return sf::Color(220, 220, 220); // Light gray
    }
    else
    {
        return sf::Color(30, 30, 30); // Dark gray
    }
}

void StatusPanel::refreshContent(const void *selectedObject, const std::string &objectType, const std::vector<std::string> &pendingTasks)
{
    if (m_objectInspector)
    {
        m_objectInspector->updateObject(selectedObject, objectType);
    }
    if (m_taskListView)
    {
        // This version of refreshContent still uses vector<string> for tasks.
        // If you want to pass vector<Task>, you'll need to overload refreshContent or change its signature.
        m_taskListView->updateTasks(pendingTasks);
    }

    // Update pending tasks count
    setPendingTaskCount(pendingTasks.size());
}

// Overload for std::vector<Task>
void StatusPanel::refreshContent(const void *selectedObject, const std::string &objectType, const std::vector<Task> &pendingTasksData)
{
    if (m_objectInspector)
    {
        m_objectInspector->updateObject(selectedObject, objectType);
    }
    if (m_taskListView)
    {
        m_taskListView->updateTasks(pendingTasksData);
    }
    setPendingTaskCount(pendingTasksData.size());
}

void StatusPanel::render(sf::RenderTarget &target, const sf::Vector2f &position)
{
    // Render main background
    m_background.setPosition(position);
    target.draw(m_background);

    // Render header
    m_headerBackground.setPosition(position);
    target.draw(m_headerBackground);

    // Render header text
    sf::Vector2f headerTextPos = position + sf::Vector2f(m_padding, m_padding);
    m_headerText.setPosition(headerTextPos);
    target.draw(m_headerText); // Calculate section positions
    float headerHeight = 40.0f;
    float globalStatusHeight = 80.0f; // Space for time, vehicle count, etc.
    float currentY = position.y + headerHeight + m_padding;

    // Render global status section
    sf::Vector2f statusStartPos = position + sf::Vector2f(m_padding, headerHeight + m_padding);

    m_simTimeDisplay.setPosition(statusStartPos);
    target.draw(m_simTimeDisplay);

    m_vehicleCountDisplay.setPosition(statusStartPos + sf::Vector2f(0, m_lineSpacing));
    target.draw(m_vehicleCountDisplay);

    m_completedTasksDisplay.setPosition(statusStartPos + sf::Vector2f(0, m_lineSpacing * 2));
    target.draw(m_completedTasksDisplay);

    m_pendingTasksDisplay.setPosition(statusStartPos + sf::Vector2f(0, m_lineSpacing * 3));
    target.draw(m_pendingTasksDisplay);

    // Calculate available content height again for positioning
    float availableContentHeight = m_panelHeight - 3 * m_padding - headerHeight - globalStatusHeight - 2 * m_sectionSpacing;
    float inspectorHeight = availableContentHeight * 0.35f;
    float taskListHeight = availableContentHeight * 0.25f;

    // Render separators and position sub-components
    float currentSectionY = position.y + headerHeight + globalStatusHeight + m_padding;

    if (m_sectionSeparators.size() > 0)
    {
        m_sectionSeparators[0].setPosition(sf::Vector2f(position.x + m_padding, currentSectionY));
        target.draw(m_sectionSeparators[0]);
        currentSectionY += m_sectionSeparators[0].getSize().y + m_sectionSpacing;
    }

    // Render ObjectInspector section
    if (m_objectInspector)
    {
        m_objectInspector->setPosition(position.x + m_padding, currentSectionY);
        target.draw(*m_objectInspector);
        currentSectionY += inspectorHeight + m_sectionSpacing;
    }

    if (m_sectionSeparators.size() > 1)
    {
        m_sectionSeparators[1].setPosition(sf::Vector2f(position.x + m_padding, currentSectionY));
        target.draw(m_sectionSeparators[1]);
        currentSectionY += m_sectionSeparators[1].getSize().y + m_sectionSpacing;
    }

    // Render TaskListView section
    if (m_taskListView)
    {
        m_taskListView->setPosition(position.x + m_padding, currentSectionY);
        target.draw(*m_taskListView);
        // currentSectionY += taskListHeight + m_sectionSpacing; // Update if another section follows
    }

    // Comment out the old separator logic as it's integrated above
    /*
    float globalStatusHeight = 80.0f; // This was already defined
    float separatorY = currentY + globalStatusHeight; // currentY was different here

    for (size_t i = 0; i < m_sectionSeparators.size() && i < 3; ++i) {
        float availableContentHeight = m_panelHeight - 3 * m_padding - headerHeight - globalStatusHeight;
        float taskListHeight = availableContentHeight * 0.25f;
        float inspectorHeight = availableContentHeight * 0.35f;

        sf::Vector2f sepPos;
        if (i == 0) {
            sepPos = sf::Vector2f(position.x + m_padding, separatorY);
        } else if (i == 1) {
            sepPos = sf::Vector2f(position.x + m_padding, separatorY + taskListHeight + m_sectionSpacing);
        } else {
            sepPos = sf::Vector2f(position.x + m_padding, separatorY + taskListHeight + inspectorHeight + 2 * m_sectionSpacing);
        }

        m_sectionSeparators[i].setPosition(sepPos);
        target.draw(m_sectionSeparators[i]);
    }

    // Render TaskListView section
    // Note: TaskListView rendering would go here if implemented    // Render ObjectInspector section
    // Note: ObjectInspector rendering would go here if implemented
    */
}

void StatusPanel::render(sf::RenderTarget &target, const sf::Vector2u &windowSize)
{
    // Calculate right-aligned position
    float rightAlignedX = static_cast<float>(windowSize.x) - m_panelWidth;
    sf::Vector2f rightAlignedPosition(rightAlignedX, 0.0f);

    // Call the existing render method with calculated position
    render(target, rightAlignedPosition);
}

bool StatusPanel::handleEvent(const sf::Event &event, const sf::Vector2f &localPos)
{
    // Handle scroll events for TaskListView
    if (event.type == sf::Event::MouseWheelScrolled)
    {
        if (m_taskListView)
        {
            return m_taskListView->handleScrollEvent(event.mouseWheelScroll, localPos);
        }
    }

    // Handle click events
    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
    {
        if (m_taskListView)
        {
            m_taskListView->handleClick(localPos);
        }
    }

    // Handle ObjectInspector events
    if (m_objectInspector)
    {
        return m_objectInspector->handleEvent(event, localPos);
    }

    return false;
}

float StatusPanel::getPanelWidth() const
{
    return m_panelWidth;
}

void StatusPanel::resize(float height)
{
    m_panelHeight = height;
    calculateLayout();
    updateBackgroundShapes();
}

void StatusPanel::setSimulationTime(float time)
{
    std::string timeStr = UIUtils::formatSimulationTime(time);
    m_simTimeDisplay.setString("Sim Time: " + timeStr);
}

void StatusPanel::setVehicleCount(size_t count)
{
    m_vehicleCountDisplay.setString("Vehicles: " + std::to_string(count));
}

void StatusPanel::setCompletedTaskCount(size_t count)
{
    m_completedTasksDisplay.setString("Completed: " + std::to_string(count));
}

void StatusPanel::setPendingTaskCount(size_t count)
{
    m_pendingTasksDisplay.setString("Pending: " + std::to_string(count));
}
