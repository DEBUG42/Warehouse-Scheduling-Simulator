#include "gui/ObjectInspector.hpp"
#include <iomanip> // For std::fixed, std::setprecision
#include <sstream> // For std::ostringstream

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
    m_titleText.setFillColor(sf::Color::White);
    m_titleText.setStyle(sf::Text::Bold);
    rebuildDisplay(); // 初始显示 "未选中对象"
}

/**
 * @brief 更新当前检视的对象
 * @param gameObject 要显示的对象，如果为nullptr则清空显示
 */
void ObjectInspector::updateObject(const gui::SimObject *selectedObject)
{
    m_currentObject = selectedObject;
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

    // 使用 SimObject 的 getter 方法
    addDetailLine("ID: ", m_currentObject->getId(), currentY);

    std::ostringstream ossPos;
    ossPos << std::fixed << std::setprecision(1)
           << "(" << m_currentObject->getPosition().x << ", " << m_currentObject->getPosition().y << ")";
    addDetailLine("Position: ", ossPos.str(), currentY);

    // 特定类型信息
    if (m_currentObject->getType() == gui::SimObjectType::Vehicle)
    {
        // 安全地转换为 VehicleState*
        const auto *vehicle = dynamic_cast<const gui::VehicleState *>(m_currentObject);
        if (vehicle)
        {
            addDetailLine("Type: ", "AGV", currentY);
            std::ostringstream ossSpeed;
            ossSpeed << std::fixed << std::setprecision(2) << vehicle->speed << " m/s";
            addDetailLine("Speed: ", ossSpeed.str(), currentY);
            addDetailLine("Status: ", vehicleStatusToString(vehicle->status), currentY);
            addDetailLine("Task ID: ", vehicle->currentTaskId.empty() ? "None" : vehicle->currentTaskId, currentY);
            addDetailLine("Load Status: ", vehicle->isLoaded ? "Loaded" : "Empty", currentY);
            if (vehicle->isLoaded)
            {
                addDetailLine("  Material ID: ", std::to_string(vehicle->cargo.materialId), currentY);
                addDetailLine("  Quantity: ", std::to_string(vehicle->cargo.quantity), currentY);
            }
            std::ostringstream ossBattery;
            ossBattery << std::fixed << std::setprecision(0) << (vehicle->batteryLevel * 100) << "%";
            addDetailLine("Battery: ", ossBattery.str(), currentY);
        }
    }
    else if (m_currentObject->getType() == gui::SimObjectType::Device)
    {
        const auto *device = dynamic_cast<const gui::DeviceState *>(m_currentObject);
        if (device)
        {
            addDetailLine("Type: ", deviceTypeToString(device->deviceType), currentY);
            addDetailLine("Status: ", deviceStatusToString(device->status), currentY);
            if (!device->boundVehicleId.empty())
            {
                addDetailLine("Bound Vehicle: ", device->boundVehicleId, currentY);
            }
            if (device->deviceType == gui::DeviceType::WORK_STATION)
            {
                // addDetailLine("  物料需求: ", "示例需求", currentY);
                // addDetailLine("  生产进度: ", "50%", currentY);
            }
            addDetailLine("  Capacity: ", std::to_string(device->capacity), currentY);
            addDetailLine("  Current Load: ", std::to_string(device->currentLoad), currentY);
            addDetailLine("  Material ID (Processing): ", device->materialId == -1 ? "None" : std::to_string(device->materialId), currentY);
            std::ostringstream ossProgress;
            ossProgress << std::fixed << std::setprecision(0) << (device->processingProgress * 100) << "%";
            addDetailLine("  Processing Progress: ", ossProgress.str(), currentY);
            addDetailLine("  Queued Tasks: ", std::to_string(device->queuedTaskCount), currentY);
        }
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

    target.draw(m_titleText, states);

    sf::RenderStates detailStates = states; // 复制一份用于绘制详情
    // 详情文本的位置已经在 rebuildDisplay 中相对于 ObjectInspector 的 (0,0) 设置好了
    // 因此，它们会随着 states.transform 一起变换

    for (const auto &line : m_detailLines)
    {
        target.draw(line, detailStates);
    }
}

// Helper function to convert VehicleStatus to string
std::string ObjectInspector::vehicleStatusToString(gui::VehicleStatus status) const
{
    switch (status)
    {
    case gui::VehicleStatus::IDLE:
        return "Idle";
    case gui::VehicleStatus::MOVING_TO_LOAD:
        return "Moving to Load";
    case gui::VehicleStatus::LOADING:
        return "Loading";
    case gui::VehicleStatus::MOVING_TO_UNLOAD:
        return "Moving to Unload";
    case gui::VehicleStatus::UNLOADING:
        return "Unloading";
    case gui::VehicleStatus::CHARGING:
        return "Charging";
    case gui::VehicleStatus::ERROR:
        return "Error";
    default:
        return "Unknown";
    }
}

// Helper function to convert DeviceType to string
std::string ObjectInspector::deviceTypeToString(gui::DeviceType type) const
{
    switch (type)
    {
    case gui::DeviceType::CORE_WORKSTATION_OUT:
        return "Workstation Out";
    case gui::DeviceType::CORE_WORKSTATION_IN:
        return "Workstation In";
    case gui::DeviceType::CHARGER:
        return "Charger";
    default:
        return "Unknown Device";
    }
}

// Helper function to convert DeviceStatus to string
std::string ObjectInspector::deviceStatusToString(gui::DeviceStatus status) const
{
    switch (status)
    {
    case gui::DeviceStatus::IDLE:
        return "Idle";
    case gui::DeviceStatus::BUSY:
        return "Busy";
    case gui::DeviceStatus::WORKING:
        return "Working";
    case gui::DeviceStatus::OFFLINE:
        return "Offline";
    case gui::DeviceStatus::CHARGING_VEHICLE:
        return "Charging Vehicle";
    case gui::DeviceStatus::ERROR:
        return "Error";
    default:
        return "Unknown";
    }
}

void ObjectInspector::addDetailLine(const std::string &label, const std::string &value, float &currentY)
{
    sf::Text lineText;
    lineText.setFont(m_font);
    lineText.setCharacterSize(m_characterSize);
    lineText.setFillColor(sf::Color(200, 200, 200)); // 浅灰色细节
    lineText.setString(label + value);
    lineText.setPosition(m_padding, currentY);
    m_detailLines.push_back(lineText);
    currentY += m_lineSpacing;
}