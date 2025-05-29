#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>
#include "gui/TaskListView.hpp"
#include "gui/ObjectInspector.hpp"

/**
 * @brief Modern Status Panel Class
 *
 * Located on the right side of the interface for displaying detailed information about the currently selected object
 * Includes object type, ID, status properties, and related task lists
 * Dynamically adjusts display content based on different object types (vehicle/device)
 *
 * Modern Design Features:
 * - Dark theme consistent with Toolbar
 * - Improved color contrast
 * - Enhanced visual hierarchy
 * - Better component separation
 */
class StatusPanel
{
private:
    // Layout parameters - Modern design
    const float m_panelWidth = 300.0f;
    float m_panelHeight = 600.0f;
    const sf::Color m_backgroundColor{40, 40, 40}; // Dark gray background
    const sf::Color m_headerColor{50, 50, 50};     // Darker header
    const sf::Color m_separatorColor{70, 70, 70};  // Separator color
    const sf::Color m_textColor{200, 200, 200};    // Light gray text
    const sf::Color m_accentColor{70, 130, 180};   // Steel blue accent
    const float m_padding = 12.0f;                 // Increased padding
    const float m_lineSpacing = 20.0f;             // Increased line spacing
    const float m_sectionSpacing = 15.0f;          // Section spacing

    // Font reference
    sf::Font &m_font;

    // Background shapes for modern design
    sf::RectangleShape m_background;
    sf::RectangleShape m_headerBackground;
    std::vector<sf::RectangleShape> m_sectionSeparators;

    // Content elements
    std::unique_ptr<TaskListView> m_taskListView;
    std::unique_ptr<ObjectInspector> m_objectInspector;

    // Status information displays with improved styling
    sf::Text m_headerText;
    sf::Text m_simTimeDisplay;
    sf::Text m_vehicleCountDisplay;
    sf::Text m_completedTasksDisplay;
    sf::Text m_pendingTasksDisplay;

    // Layout calculation
    void calculateLayout();
    void updateBackgroundShapes();
    sf::Color getOptimalTextColor(const sf::Color &backgroundColor) const;

public:
    /**
     * @brief Constructor
     * @param font Font reference
     */
    StatusPanel(sf::Font &font);

    /**
     * @brief Update panel display content
     * @param selectedObject Currently selected object (can be nullptr)
     * @param objectType Object type string
     * @param pendingTasks Global pending task queue
     */
    void refreshContent(const void *selectedObject,
                        const std::string &objectType,
                        const std::vector<std::string> &pendingTasks);

    /**
     * @brief Render panel interface
     * @param target Render target
     * @param position Panel top-left position
     */
    void render(sf::RenderTarget &target, const sf::Vector2f &position);

    /**
     * @brief Handle input events within panel area
     * @param event SFML event object
     * @param localPos Mouse position relative to panel
     * @return Whether the event was consumed
     */
    bool handleEvent(const sf::Event &event, const sf::Vector2f &localPos);

    /**
     * @brief Get panel width
     * @return Panel width (pixels)
     */
    float getPanelWidth() const;

    /**
     * @brief Adjust panel height
     * @param height New height value
     */
    void resize(float height);

    /**
     * @brief Set simulation time
     * @param time Simulation time (seconds)
     */
    void setSimulationTime(float time);

    /**
     * @brief Set vehicle count
     * @param count Vehicle count
     */
    void setVehicleCount(size_t count);

    /**
     * @brief Set completed task count
     * @param count Completed task count
     */
    void setCompletedTaskCount(size_t count);

    /**
     * @brief Set pending task count
     * @param count Pending task count
     */
    void setPendingTaskCount(size_t count);
};