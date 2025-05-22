#include "gui/WarehouseRenderer.hpp"
#include <iostream>

WarehouseRenderer::WarehouseRenderer()
{
    // 加载字体
    if (!m_font.loadFromFile("resources/fonts/Arial.ttf"))
    {
        std::cerr << "警告：无法加载字体文件，接口标签可能无法正确显示" << std::endl;
    }
}

void WarehouseRenderer::initialize(float trackRadius)
{
    // 清除现有数据
    m_interfaces.clear();
    m_labels.clear();

    // 创建上方自动化库仓储区接口
    createTopInterfaces(trackRadius);

    // 创建下方出入库作业区接口
    createBottomInterfaces(trackRadius);
}

void WarehouseRenderer::updateDeviceStates(const std::vector<gui::DeviceState> &deviceStates)
{
    // 更新每个接口设备的状态
    for (const auto &state : deviceStates)
    {
        for (auto &interface : m_interfaces)
        {
            if (std::to_string(interface.id) == state.getId())
            {
                interface.state = state;
                break;
            }
        }
    }
}

void WarehouseRenderer::setInputColor(const sf::Color &color)
{
    m_inputColor = color;
}

void WarehouseRenderer::setOutputColor(const sf::Color &color)
{
    m_outputColor = color;
}

const WarehouseRenderer::WarehouseInterface *WarehouseRenderer::getInterfaceAt(const sf::Vector2f &position) const
{
    // 检查点击位置是否在某个接口设备范围内
    for (const auto &interface : m_interfaces)
    {
        // 转换为像素坐标
        float x = interface.centerX * MM_TO_PIXEL;
        float y = interface.centerY * MM_TO_PIXEL;
        float width = interface.width * MM_TO_PIXEL;
        float height = interface.depth * MM_TO_PIXEL; // 深度在2D视图中是Y方向的高度

        // 创建边界框
        sf::FloatRect bounds(x - width / 2.0f, y - height / 2.0f, width, height);

        // 检查点是否在边界框内
        if (bounds.contains(position))
        {
            return &interface;
        }
    }
    return nullptr;
}

void WarehouseRenderer::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    // 绘制所有接口设备
    for (const auto &interface : m_interfaces)
    {
        drawInterface(target, states, interface);
    }

    // 绘制所有标签
    for (const auto &label : m_labels)
    {
        target.draw(label, states);
    }
}

void WarehouseRenderer::drawInterface(sf::RenderTarget &target, sf::RenderStates states,
                                      const WarehouseInterface &interface) const
{
    // 转换为像素坐标
    float x = interface.centerX * MM_TO_PIXEL;
    float y = interface.centerY * MM_TO_PIXEL;
    float width = interface.width * MM_TO_PIXEL;
    float height = interface.depth * MM_TO_PIXEL; // 深度在2D视图中表现为Y轴高度

    // 确定颜色
    sf::Color fillColor = (interface.type == InterfaceType::INPUT) ? m_inputColor : m_outputColor;

    // 根据设备状态调整颜色亮度
    if (interface.state.status == gui::DeviceStatus::IDLE)
    {
        // 空闲状态，使用淡色
        fillColor.a = 180;
    }
    else if (interface.state.status == gui::DeviceStatus::BUSY)
    {
        // 繁忙状态，使用亮色
        fillColor.a = 255;
    }
    else
    {
        // 故障/离线状态，使用暗色
        fillColor.r = fillColor.r / 2;
        fillColor.g = fillColor.g / 2;
        fillColor.b = fillColor.b / 2;
    }

    // 创建伪3D效果
    // 1. 先绘制侧面和顶面（增加立体感）
    sf::RectangleShape side(sf::Vector2f(width, interface.height * MM_TO_PIXEL * 0.3f));
    side.setPosition(x - width / 2.0f, y - height / 2.0f);
    side.setFillColor(sf::Color(fillColor.r / 1.5, fillColor.g / 1.5, fillColor.b / 1.5));
    target.draw(side, states);

    // 2. 主体矩形
    sf::RectangleShape body(sf::Vector2f(width, height));
    body.setPosition(x - width / 2.0f, y - height / 2.0f);
    body.setFillColor(fillColor);
    body.setOutlineThickness(1.0f);
    body.setOutlineColor(m_borderColor);
    target.draw(body, states);

    // 3. 绘制阴影（增加立体感）
    sf::RectangleShape shadow(sf::Vector2f(width, height / 4.0f));
    shadow.setPosition(x - width / 2.0f, y + height / 2.0f);
    shadow.setFillColor(m_shadowColor);
    target.draw(shadow, states);
}

// 添加辅助函数
gui::DeviceState warehouseToDeviceState(const WarehouseState &warehouse, bool isInput)
{
    // 不能直接访问id/type，需通过构造
    gui::DeviceState device(std::to_string(warehouse.id), sf::Vector2f(0, 0),
                            isInput ? gui::DeviceType::INPUT_STATION : gui::DeviceType::OUTPUT_STATION,
                            gui::DeviceStatus::IDLE, "");
    // 其它字段可按需赋值
    return device;
}

void WarehouseRenderer::createTopInterfaces(float /* trackRadius */)
{
    // 上方自动化库仓储区的12个接口设备
    // 根据开发界面信息计算每个接口的位置

    // 轨道顶部Y坐标 (mm)
    float topY = 2500.0f; // 上方直轨的Y坐标
    // 仓库深度 (mm)
    float warehouseDepth = 1200.0f;
    // 仓库Y坐标 = 轨道Y + 间隙 + 仓库深度/2
    float warehouseY = topY + 500.0f + warehouseDepth / 2.0f;

    // 定义宽度数组 (mm)，按照开发界面信息给定的间距
    float widths[] = {1250.0f, 1250.0f, 2500.0f, 2500.0f, 2500.0f, 2500.0f,
                      2500.0f, 2500.0f, 2500.0f, 2500.0f, 2500.0f, 2500.0f};

    // 计算累积X坐标
    float currentX = 2500.0f; // 从左侧弯道开始

    // 创建1-12号接口
    for (int i = 0; i < 12; ++i)
    {
        WarehouseInterface interface;
        interface.id = i + 1; // ID从1开始
        interface.position = WarehousePosition::TOP;

        // 判断是入库还是出库接口（奇数为入库，偶数为出库）
        interface.type = (i % 2 == 0) ? InterfaceType::INPUT : InterfaceType::OUTPUT;

        // 计算中心X坐标
        currentX += widths[i] / 2.0f;
        interface.centerX = currentX;
        interface.centerY = warehouseY;

        // 设置尺寸
        interface.width = widths[i] * 0.9f; // 稍微小一点，留间隙
        interface.depth = warehouseDepth;
        interface.height = 2000.0f; // 高度假设为2m

        // 初始化设备状态
        interface.state = gui::DeviceState(
            std::to_string(interface.id), sf::Vector2f(0, 0),
            (interface.type == InterfaceType::INPUT) ? gui::DeviceType::INPUT_STATION : gui::DeviceType::OUTPUT_STATION,
            gui::DeviceStatus::IDLE, "");

        // 添加到列表
        m_interfaces.push_back(interface);

        // 创建标签
        sf::Text label;
        std::string labelText = std::to_string(interface.id) + "号" +
                                ((interface.type == InterfaceType::INPUT) ? "入库口" : "出库口");
        createLabel(label, m_font, labelText,
                    interface.centerX * MM_TO_PIXEL,
                    (interface.centerY - interface.depth / 2.0f) * MM_TO_PIXEL - 15.0f);
        m_labels.push_back(label);

        // 更新currentX为下一个接口的起始位置
        currentX += widths[i] / 2.0f;
    }
}

void WarehouseRenderer::createBottomInterfaces(float /* trackRadius */)
{
    // 下方出入库作业区的6个接口设备
    // 根据开发界面信息计算每个接口的位置

    // 轨道底部Y坐标 (mm)
    float bottomY = -2500.0f; // 下方直轨的Y坐标
    // 仓库深度 (mm)
    float warehouseDepth = 1500.0f;
    // 仓库Y坐标 = 轨道Y - 间隙 - 仓库深度/2
    float warehouseY = bottomY - 500.0f - warehouseDepth / 2.0f;

    // 定义宽度和起始X坐标数组 (mm)，按照开发界面信息给定的间距
    struct InterfaceInfo
    {
        float startX;
        float width;
        InterfaceType type;
    };

    InterfaceInfo infos[] = {
        {7450.0f, 1200.0f, InterfaceType::OUTPUT},  // 13号出库口
        {8650.0f, 1200.0f, InterfaceType::OUTPUT},  // 14号出库口
        {9850.0f, 10900.0f, InterfaceType::OUTPUT}, // 15号出库口
        {20750.0f, 10900.0f, InterfaceType::INPUT}, // 16号入库口
        {31650.0f, 1300.0f, InterfaceType::INPUT},  // 17号入库口
        {32950.0f, 1300.0f, InterfaceType::INPUT},  // 18号入库口
    };

    // 创建13-18号接口
    for (int i = 0; i < 6; ++i)
    {
        WarehouseInterface interface;
        interface.id = i + 13; // ID从13开始
        interface.position = WarehousePosition::BOTTOM;
        interface.type = infos[i].type;

        // 计算中心X坐标
        interface.centerX = infos[i].startX + infos[i].width / 2.0f;
        interface.centerY = warehouseY;

        // 设置尺寸
        interface.width = infos[i].width;
        interface.depth = warehouseDepth;
        interface.height = 2500.0f; // 高度假设为2.5m

        // 初始化设备状态
        interface.state = gui::DeviceState(
            std::to_string(interface.id), sf::Vector2f(0, 0),
            (interface.type == InterfaceType::INPUT) ? gui::DeviceType::INPUT_STATION : gui::DeviceType::OUTPUT_STATION,
            gui::DeviceStatus::IDLE, "");

        // 添加到列表
        m_interfaces.push_back(interface);

        // 创建标签
        sf::Text label;
        std::string labelText = std::to_string(interface.id) + "号" +
                                ((interface.type == InterfaceType::INPUT) ? "入库口" : "出库口");
        createLabel(label, m_font, labelText,
                    interface.centerX * MM_TO_PIXEL,
                    (interface.centerY + interface.depth / 2.0f) * MM_TO_PIXEL + 5.0f);
        m_labels.push_back(label);
    }
}

void WarehouseRenderer::updateWarehouseStates(const std::vector<WarehouseState> &warehouseStates)
{
    // 将WarehouseState转换为DeviceState
    std::vector<gui::DeviceState> deviceStates;

    for (const auto &warehouse : warehouseStates)
    {
        // 转换为设备状态 - 默认isInput为true，但实际应根据仓库角色判断
        bool isInput = (warehouse.id % 2 == 0); // 简单划分：偶数ID为输入设备
        gui::DeviceState device = warehouseToDeviceState(warehouse, isInput);
        deviceStates.push_back(device);
    }

    // 调用现有方法更新设备状态
    updateDeviceStates(deviceStates);
}

void WarehouseRenderer::createLabel(sf::Text &text, const sf::Font &font, const std::string &content,
                                    float x, float y, float scale)
{
    text.setFont(font);
    text.setString(content);
    text.setCharacterSize(12 * scale);
    text.setFillColor(sf::Color::Black);

    // 设置阴影
    text.setOutlineColor(sf::Color(255, 255, 255, 180));
    text.setOutlineThickness(0.5f);

    // 居中对齐
    sf::FloatRect bounds = text.getLocalBounds();
    text.setOrigin(bounds.left + bounds.width / 2.0f, bounds.top + bounds.height / 2.0f);
    text.setPosition(x, y);
}
