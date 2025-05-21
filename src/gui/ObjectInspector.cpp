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
        m_titleText.setString("未选中对象");
        m_titleText.setPosition(m_padding, m_padding);
        return;
    }

    m_titleText.setString("对象详情");
    m_titleText.setPosition(m_padding, m_padding);

    // 使用 SimObject 的 getter 方法
    addDetailLine("ID: ", m_currentObject->getId(), currentY);

    std::ostringstream ossPos;
    ossPos << std::fixed << std::setprecision(1)
           << "(" << m_currentObject->getPosition().x << ", " << m_currentObject->getPosition().y << ")";
    addDetailLine("位置: ", ossPos.str(), currentY);

    // 特定类型信息
    if (m_currentObject->getType() == gui::SimObjectType::Vehicle)
    {
        // 安全地转换为 VehicleState*
        const auto *vehicle = dynamic_cast<const gui::VehicleState *>(m_currentObject);
        if (vehicle)
        {
            addDetailLine("类型: ", "车辆 (AGV)", currentY);
            std::ostringstream ossSpeed;
            ossSpeed << std::fixed << std::setprecision(2) << vehicle->speed << " m/s";
            addDetailLine("速度: ", ossSpeed.str(), currentY);
            addDetailLine("运动状态: ", vehicleStatusToString(vehicle->status), currentY);
            addDetailLine("当前任务ID: ", vehicle->currentTaskId.empty() ? "无" : vehicle->currentTaskId, currentY);
            addDetailLine("载货状态: ", vehicle->isLoaded ? "已载货" : "空闲", currentY);
            if (vehicle->isLoaded)
            {
                addDetailLine("  物料ID: ", std::to_string(vehicle->cargo.materialId), currentY);
                addDetailLine("  数量: ", std::to_string(vehicle->cargo.quantity), currentY);
            }
            std::ostringstream ossBattery;
            ossBattery << std::fixed << std::setprecision(0) << (vehicle->batteryLevel * 100) << "%";
            addDetailLine("电量: ", ossBattery.str(), currentY);
        }
    }
    else if (m_currentObject->getType() == gui::SimObjectType::Device)
    {
        const auto *device = dynamic_cast<const gui::DeviceState *>(m_currentObject);
        if (device)
        {
            addDetailLine("类型: ", deviceTypeToString(device->deviceType), currentY);
            addDetailLine("设备状态: ", deviceStatusToString(device->status), currentY);
            if (!device->boundVehicleId.empty())
            {
                addDetailLine("关联车辆ID: ", device->boundVehicleId, currentY);
            }
            if (device->deviceType == gui::DeviceType::WORK_STATION)
            {
                // addDetailLine("  物料需求: ", "示例需求", currentY);
                // addDetailLine("  生产进度: ", "50%", currentY);
            }
            addDetailLine("  容量: ", std::to_string(device->capacity), currentY);
            addDetailLine("  当前负载: ", std::to_string(device->currentLoad), currentY);
            addDetailLine("  物料ID (处理中): ", device->materialId == -1 ? "无" : std::to_string(device->materialId), currentY);
            std::ostringstream ossProgress;
            ossProgress << std::fixed << std::setprecision(0) << (device->processingProgress * 100) << "%";
            addDetailLine("  处理进度: ", ossProgress.str(), currentY);
            addDetailLine("  排队任务: ", std::to_string(device->queuedTaskCount), currentY);
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
        return "空闲";
    case gui::VehicleStatus::MOVING_TO_LOAD:
        return "前往装货点";
    case gui::VehicleStatus::LOADING:
        return "装货中";
    case gui::VehicleStatus::MOVING_TO_UNLOAD:
        return "前往卸货点";
    case gui::VehicleStatus::UNLOADING:
        return "卸货中";
    case gui::VehicleStatus::CHARGING:
        return "充电中";
    case gui::VehicleStatus::ERROR:
        return "错误";
    case gui::VehicleStatus::UNKNOWN:
        return "未知(移动中)";
    default:
        return "未知车辆状态";
    }
}

// Helper function to convert DeviceType to string
std::string ObjectInspector::deviceTypeToString(gui::DeviceType type) const
{
    switch (type)
    {
    case gui::DeviceType::CHARGER:
        return "充电桩";
    case gui::DeviceType::WORK_STATION:
        return "工作站";
    case gui::DeviceType::INPUT_STATION:
        return "入库口";
    case gui::DeviceType::OUTPUT_STATION:
        return "出库口";
    case gui::DeviceType::STORAGE_STATION:
        return "仓储区";
    case gui::DeviceType::CORE_STORAGE_IN:
        return "入库接口(Core)";
    case gui::DeviceType::CORE_STORAGE_OUT:
        return "出库接口(Core)";
    case gui::DeviceType::CORE_WORKSTATION_IN:
        return "工作站入(Core)";
    case gui::DeviceType::CORE_WORKSTATION_OUT:
        return "工作站出(Core)";
    case gui::DeviceType::UNKNOWN_DEVICE_TYPE:
        return "未知设备类型";
    default:
        return "未知设备类型";
    }
}

// Helper function to convert DeviceStatus to string
std::string ObjectInspector::deviceStatusToString(gui::DeviceStatus status) const
{
    switch (status)
    {
    case gui::DeviceStatus::IDLE:
        return "空闲";
    case gui::DeviceStatus::WORKING:
        return "工作中";
    case gui::DeviceStatus::CHARGING_VEHICLE:
        return "车辆充电中";
    case gui::DeviceStatus::OFFLINE:
        return "离线";
    case gui::DeviceStatus::ERROR_STATUS:
        return "错误";
    case gui::DeviceStatus::BUSY:
        return "忙碌";
    case gui::DeviceStatus::UNKNOWN_DEVICE_STATUS:
        return "未知设备状态";
    default:
        return "未知设备状态";
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