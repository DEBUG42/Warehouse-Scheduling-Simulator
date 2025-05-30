#include "gui/ObjectInspector.hpp"
#include <iomanip> // For std::fixed, std::setprecision
#include <sstream> // For std::ostringstream
#include <iostream>

/**
 * @brief 构造函数
 * @param font 字体引用
 * @param width 检视器宽度
 */
ObjectInspector::ObjectInspector(sf::Font &font, float width)
    : m_font(font), m_width(width)
{
    m_titleText.setFont(m_font);
    m_titleText.setCharacterSize(m_characterSize + 2); // 标题稍大
    m_titleText.setFillColor(sf::Color(50, 50, 50));   // 深灰色标题
    m_titleText.setStyle(sf::Text::Bold);
    rebuildDisplay(); // 初始显示 "未选中对象"
}

/**
 * @brief 更新当前检视的对象
 * @param selectedObject 要显示的对象，如果为nullptr则清空显示
 * @param objectType 对象类型字符串
 */
void ObjectInspector::updateObject(const void *selectedObject, const std::string &objectType)
{
    m_currentObject = selectedObject;
    m_objectType = objectType;
    rebuildDisplay();
}

/**
 * @brief 内部方法，用于根据对象类型构建显示内容
 */
void ObjectInspector::rebuildDisplay()
{
    m_detailLines.clear();
    float currentY = m_padding + m_lineSpacing; // 标题下方开始

    if (!m_currentObject)
    {
        m_titleText.setString("No Object Selected");
        m_titleText.setPosition(m_padding, m_padding);
        return;
    }

    m_titleText.setString("Object Details");
    m_titleText.setPosition(m_padding, m_padding);

    if (m_objectType == "Vehicle")
    {
        const Vehicle *vehicle = static_cast<const Vehicle *>(m_currentObject); // Changed core::Vehicle to Vehicle
        if (vehicle)
        {
            addDetailLine("Type: ", "Vehicle", currentY);
            addDetailLine("ID: ", std::to_string(vehicle->id), currentY);

            std::ostringstream speedStream;
            speedStream << std::fixed << std::setprecision(2) << vehicle->m_state.currentSpeed << " m/s";
            addDetailLine("Speed: ", speedStream.str(), currentY);

            addDetailLine("State: ", vehicleStatusToString(static_cast<int>(vehicle->m_state.motionState)), currentY);

            std::ostringstream posStream;
            posStream << std::fixed << std::setprecision(2) << vehicle->m_state.position << " m";
            addDetailLine("Position: ", posStream.str(), currentY);
            addDetailLine("Loaded: ", vehicle->is_loaded ? "Yes" : "No", currentY);
        }
    }
    else if (m_objectType == "Device")
    {
        // Assuming Device is similar or you have a way to cast and get its details
        // const core::Device* device = static_cast<const core::Device*>(m_currentObject);
        // if (device) { ... }
        addDetailLine("Type: ", "Device", currentY);
        addDetailLine("Status: ", "Active", currentY);       // Placeholder
        addDetailLine("Device Type: ", "Storage", currentY); // Placeholder
        addDetailLine("Capacity: ", "100 units", currentY);  // Placeholder
    }
    else
    {
        addDetailLine("Type: ", m_objectType, currentY);
        addDetailLine("Status: ", "Unknown", currentY);
    }
}

/**
 * @brief 绘制对象检视器
 * @param target 渲染目标
 * @param states 渲染状态
 */
void ObjectInspector::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    states.transform *= getTransform(); // 应用 ObjectInspector 自身的位置变换

    // 背景 (可选，如果需要与StatusPanel背景区分或有边框)
    // sf::RectangleShape background(sf::Vector2f(m_width, m_height)); // m_height需要有效
    // background.setFillColor(sf::Color(220, 220, 220)); // 比StatusPanel稍深一点的背景？
    // target.draw(background, states);

    target.draw(m_titleText, states);

    sf::RenderStates detailStates = states; // 复制一份用于绘制详情
    // 详情文本的位置已经在 rebuildDisplay 中相对于 ObjectInspector 的 (0,0) 设置好了
    // 因此，它们会随着 states.transform 一起变换

    for (const auto &line : m_detailLines)
    {
        target.draw(line, detailStates);
    }
}

// 简化的辅助方法
std::string ObjectInspector::vehicleStatusToString(int status) const
{
    // Assuming Vehicle::MotionState is the enum used
    switch (static_cast<Vehicle::MotionState>(status)) // Changed core::Vehicle to Vehicle
    {
    case Vehicle::MotionState::Stopped:
        return "Stopped";
    case Vehicle::MotionState::Accelerating:
        return "Accelerating";
    case Vehicle::MotionState::Decelerating:
        return "Decelerating";
    case Vehicle::MotionState::Cruising:
        return "Cruising";
    // case Vehicle::MotionState::EmergencyStop: // EmergencyStop is not in the provided Vehicle.hpp
    //     return "EmergencyStop";
    default:
        return "Unknown";
    }
}

std::string ObjectInspector::deviceTypeToString(int type) const
{
    return "Storage"; // 简化版本
}

std::string ObjectInspector::deviceStatusToString(int status) const
{
    return "Active"; // 简化版本
}

void ObjectInspector::addDetailLine(const std::string &label, const std::string &value, float &currentY)
{
    sf::Text lineText;
    lineText.setFont(m_font);
    lineText.setCharacterSize(m_characterSize);
    lineText.setFillColor(sf::Color(70, 70, 70)); // 深灰色细节文本 (原为200,200,200)
    lineText.setString(label + value);
    lineText.setPosition(m_padding, currentY);
    m_detailLines.push_back(lineText);
    currentY += m_lineSpacing;
}

/**
 * @brief 处理对象检视器范围内的输入事件
 * @param event SFML事件对象
 * @param localMousePos 相对检视器左上角的鼠标位置
 * @return 如果事件被消耗则返回 true，否则 false
 */
bool ObjectInspector::handleEvent(const sf::Event &event, const sf::Vector2f &localMousePos)
{
    // 当前对象检视器主要用于显示信息，尚无复杂交互
    // 若将来添加可点击元素（例如，按钮跳转到车辆日志），可在此处处理
    if (event.type == sf::Event::MouseButtonPressed)
    {
        if (event.mouseButton.button == sf::Mouse::Left)
        {
            // 示例：检查是否点击了标题区域 (简单示例，实际应用中可能需要更精确的边界框)
            sf::FloatRect titleBounds = m_titleText.getGlobalBounds();
            // 注意：getGlobalBounds() 返回的是变换后的全局坐标，而 localMousePos 是局部的
            // 要正确比较，需要将 m_titleText 的 bounds 转换到局部坐标系，或者将 localMousePos 转换到全局
            // 简单起见，如果 m_titleText 的位置是 (m_padding, m_padding)，可以这样构造局部边界：
            sf::FloatRect localTitleBounds(m_padding, m_padding, m_titleText.getLocalBounds().width, m_titleText.getLocalBounds().height);

            if (localTitleBounds.contains(localMousePos))
            {
                std::cout << "[调试] ObjectInspector: 标题被点击! (位置: " << localMousePos.x << ", " << localMousePos.y << ")" << std::endl;
                // return true; // 如果消耗了事件
            }
        }
    }
    return false; // 默认不消耗事件
}

void ObjectInspector::setSize(float width, float height)
{
    m_width = width;
    // m_height = height; // ObjectInspector's height is managed by its content or StatusPanel layout
    // If you need to explicitly set height for background or clipping, uncomment and use m_height
    rebuildDisplay(); // May need to adjust layout if size changes significantly
}