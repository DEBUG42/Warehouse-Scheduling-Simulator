#include "TaskList.hpp"
#include <sstream>
#include <iomanip>

// TaskListView 实现
TaskListView::TaskListView(sf::Font &font, float width)
    : m_font(font), m_width(width)
{
}

void TaskListView::updateTasks(const std::vector<Task> &tasks)
{
    m_tasks = tasks; // 保存任务列表的副本
}

void TaskListView::render(sf::RenderTarget &target, const sf::Vector2f &position)
{
    // 绘制标题背景
    sf::RectangleShape titleBg(sf::Vector2f(m_width, 25));
    titleBg.setPosition(position);
    titleBg.setFillColor(sf::Color(60, 63, 65));
    target.draw(titleBg);

    // 绘制标题文本
    sf::Text titleText;
    titleText.setFont(m_font);
    titleText.setString("任务队列");
    titleText.setCharacterSize(16);
    titleText.setFillColor(m_textColor);
    titleText.setPosition(position.x + 10, position.y + 3);
    target.draw(titleText);

    // 计算可见的任务数量
    size_t visibleTasks = 10; // 假设固定显示10个任务

    // 绘制任务列表
    for (size_t i = 0; i < m_tasks.size() && i < visibleTasks; ++i)
    {
        // 绘制任务项背景
        sf::RectangleShape itemBg(sf::Vector2f(m_width, m_itemHeight));
        itemBg.setPosition(position.x, position.y + 25 + i * m_itemHeight);
        itemBg.setFillColor(i % 2 == 0 ? m_backgroundColor : m_highlightColor);
        target.draw(itemBg);

        // 绘制任务信息
        sf::Text taskIdText, taskTypeText, statusText;
        taskIdText.setFont(m_font);
        taskTypeText.setFont(m_font);
        statusText.setFont(m_font);

        // 设置文本内容
        taskIdText.setString("#" + std::to_string(m_tasks[i].taskId));

        std::string typeStr = (m_tasks[i].type == TaskType::input) ? "入库" : "出库";
        taskTypeText.setString(typeStr);

        std::string statusStr;
        if (m_tasks[i].assignedVehicleId >= 0)
        {
            statusStr = "进行中 (V" + std::to_string(m_tasks[i].assignedVehicleId) + ")";
            statusText.setFillColor(m_activeColor);
        }
        else
        {
            statusStr = "等待中";
            statusText.setFillColor(m_pendingColor);
        }
        statusText.setString(statusStr);

        // 设置文本样式
        taskIdText.setCharacterSize(14);
        taskTypeText.setCharacterSize(14);
        statusText.setCharacterSize(14);

        taskIdText.setFillColor(m_textColor);
        taskTypeText.setFillColor(m_textColor);

        // 设置文本位置
        taskIdText.setPosition(position.x + 10, position.y + 30 + i * m_itemHeight);
        taskTypeText.setPosition(position.x + 60, position.y + 30 + i * m_itemHeight);
        statusText.setPosition(position.x + 120, position.y + 30 + i * m_itemHeight);

        // 绘制文本
        target.draw(taskIdText);
        target.draw(taskTypeText);
        target.draw(statusText);
    }

    // 如果任务数量超过可显示数量，显示省略号
    if (m_tasks.size() > visibleTasks)
    {
        sf::Text moreText;
        moreText.setFont(m_font);
        moreText.setString("... +" + std::to_string(m_tasks.size() - visibleTasks) + " 更多任务");
        moreText.setCharacterSize(14);
        moreText.setFillColor(sf::Color(150, 150, 150));
        moreText.setPosition(position.x + 10, position.y + 25 + visibleTasks * m_itemHeight + 5);
        target.draw(moreText);
    }
}

// ObjectInspector 实现
ObjectInspector::ObjectInspector(sf::Font &font, float width)
    : m_font(font), m_width(width), m_currentType(ObjectType::None)
{
}

void ObjectInspector::updateObject(const SimObject *obj)
{
    if (obj == nullptr)
    {
        m_currentType = ObjectType::None;
        return;
    }

    // 更新对象数据
    switch (obj->getType())
    {
    case SimObject::ObjectType::Vehicle:
        m_currentType = ObjectType::Vehicle;
        m_data.vehicle.id = obj->getId();
        // 其他车辆属性需要从引擎获取
        break;

    case SimObject::ObjectType::Device:
        m_currentType = ObjectType::Device;
        m_data.device.id = obj->getId();
        // 其他设备属性需要从引擎获取
        break;

    default:
        m_currentType = ObjectType::None;
        break;
    }
}

void ObjectInspector::render(sf::RenderTarget &target, const sf::Vector2f &position)
{
    // 绘制标题背景
    sf::RectangleShape titleBg(sf::Vector2f(m_width, 25));
    titleBg.setPosition(position);
    titleBg.setFillColor(sf::Color(60, 63, 65));
    target.draw(titleBg);

    // 绘制标题文本
    sf::Text titleText;
    titleText.setFont(m_font);
    titleText.setCharacterSize(16);
    titleText.setFillColor(m_textColor);

    if (m_currentType == ObjectType::Vehicle)
    {
        titleText.setString("车辆信息 #" + std::to_string(m_data.vehicle.id));
    }
    else if (m_currentType == ObjectType::Device)
    {
        titleText.setString("设备信息 #" + std::to_string(m_data.device.id));
    }
    else
    {
        titleText.setString("未选择对象");
    }

    titleText.setPosition(position.x + 10, position.y + 3);
    target.draw(titleText);

    // 如果未选择对象，显示提示信息
    if (m_currentType == ObjectType::None)
    {
        sf::Text hintText;
        hintText.setFont(m_font);
        hintText.setString("点击车辆或设备查看详细信息");
        hintText.setCharacterSize(14);
        hintText.setFillColor(sf::Color(150, 150, 150));
        hintText.setPosition(position.x + 10, position.y + 40);
        target.draw(hintText);
        return;
    }

    // 根据对象类型绘制不同的信息
    if (m_currentType == ObjectType::Vehicle)
    {
        renderVehicleDetails(target, sf::Vector2f(position.x + 10, position.y + 30));
    }
    else if (m_currentType == ObjectType::Device)
    {
        renderDeviceDetails(target, sf::Vector2f(position.x + 10, position.y + 30));
    }
}

void ObjectInspector::renderVehicleDetails(sf::RenderTarget &target, const sf::Vector2f &position)
{
    sf::Text idText, posText, speedText, taskText, loadedText;

    // 设置字体
    idText.setFont(m_font);
    posText.setFont(m_font);
    speedText.setFont(m_font);
    taskText.setFont(m_font);
    loadedText.setFont(m_font);

    // 设置文本内容（使用示例数据）
    idText.setString("ID: " + std::to_string(m_data.vehicle.id));

    // 格式化位置显示（保留2位小数）
    std::ostringstream posStream;
    posStream << "位置: " << std::fixed << std::setprecision(2) << m_data.vehicle.position;
    posText.setString(posStream.str());

    // 格式化速度显示（保留2位小数）
    std::ostringstream speedStream;
    speedStream << "速度: " << std::fixed << std::setprecision(2) << m_data.vehicle.speed << " m/s";
    speedText.setString(speedStream.str());

    // 任务显示
    if (m_data.vehicle.taskId > 0)
    {
        taskText.setString("当前任务: #" + std::to_string(m_data.vehicle.taskId));
    }
    else
    {
        taskText.setString("当前任务: 无");
    }

    // 载货状态
    loadedText.setString("载货状态: " + std::string(m_data.vehicle.isLoaded ? "已装载" : "空载"));

    // 设置文本样式
    idText.setCharacterSize(14);
    posText.setCharacterSize(14);
    speedText.setCharacterSize(14);
    taskText.setCharacterSize(14);
    loadedText.setCharacterSize(14);

    idText.setFillColor(m_textColor);
    posText.setFillColor(m_textColor);
    speedText.setFillColor(m_textColor);
    taskText.setFillColor(m_textColor);
    loadedText.setFillColor(m_textColor);

    // 设置文本位置
    idText.setPosition(position);
    posText.setPosition(position.x, position.y + 20);
    speedText.setPosition(position.x, position.y + 40);
    taskText.setPosition(position.x, position.y + 60);
    loadedText.setPosition(position.x, position.y + 80);

    // 绘制文本
    target.draw(idText);
    target.draw(posText);
    target.draw(speedText);
    target.draw(taskText);
    target.draw(loadedText);
}

void ObjectInspector::renderDeviceDetails(sf::RenderTarget &target, const sf::Vector2f &position)
{
    sf::Text idText, typeText, statusText, queueText;

    // 设置字体
    idText.setFont(m_font);
    typeText.setFont(m_font);
    statusText.setFont(m_font);
    queueText.setFont(m_font);

    // 设置文本内容
    idText.setString("ID: " + std::to_string(m_data.device.id));

    // 设备类型    std::string typeStr;
    switch (m_data.device.type)
    {
    case DeviceType::StorageIn:
        typeStr = "入库接口";
        break;
    case DeviceType::StorageOut:
        typeStr = "出库接口";
        break;
    case DeviceType::WorkstationIn:
        typeStr = "入库作业口";
        break;
    case DeviceType::WorkstationOut:
        typeStr = "出库作业口";
        break;
    default:
        typeStr = "未知类型";
    }
    typeText.setString("类型: " + typeStr);

    // 设备状态
    std::string statusStr = (m_data.device.status == DeviceStatus::idle) ? "空闲" : "工作中";
    statusText.setString("状态: " + statusStr);    // 队列任务数量
    queueText.setString("队列任务数: " + std::to_string(m_data.device.queueCount));

    // 设置文本样式
    idText.setCharacterSize(14);
    typeText.setCharacterSize(14);
    statusText.setCharacterSize(14);
    queueText.setCharacterSize(14);

    idText.setFillColor(m_textColor);
    typeText.setFillColor(m_textColor);
    statusText.setFillColor(m_textColor);
    queueText.setFillColor(m_textColor);

    // 设置文本位置
    idText.setPosition(position);
    typeText.setPosition(position.x, position.y + 20);
    statusText.setPosition(position.x, position.y + 40);
    queueText.setPosition(position.x, position.y + 60);

    // 绘制文本
    target.draw(idText);
    target.draw(typeText);
    target.draw(statusText);
    target.draw(queueText);
}