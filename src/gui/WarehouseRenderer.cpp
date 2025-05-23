#include "gui/WarehouseRenderer.hpp"
#include <iostream>

/*
每一个仓库对应在弯道上的位置，
根据开发界面信息规定的将整个轨道左下角弯道与直道的交汇点作为原点，
计算出每个仓库接口应该对应的位置

弯道半径=2500mm
pi=3.14159265358979323846
18 8000mm
17 11000mm
16 14000mm
15 26000mm
14 29000mm
13 32000mm
12 40000+2500pi+3800=43800+7500pi=   51635.981634mm
11 40000+2500pi+6200=46200+7500pi=   54035.981634mm
10 40000+2500pi+9800=49800+7500pi=   57635.981634mm
9  40000+2500pi+12200=52200+7500pi=   60035.981634mm
8  40000+2500pi+15800=55800+7500pi=   63635.981634mm
7  40000+2500pi+18200=58200+7500pi=   66035.981634mm
6  40000+2500pi+21800=61800+7500pi=   69635.981634mm
5  40000+2500pi+24200=64200+7500pi=   72035.981634mm
4  40000+2500pi+27800=67800+7500pi=   75635.981634mm
3  40000+2500pi+30200=70200+7500pi=   78035.981634mm
2  40000+2500pi+33800=73800+7500pi=   81635.981634mm
1  40000+2500pi+36200=76200+7500pi=   84035.981634mm
*/

WarehouseRenderer::WarehouseRenderer()
{
    // 加载字体
    if (!m_font.loadFromFile("assets/fonts/Arial.ttf"))
    {
        if (!m_font.loadFromFile("resources/fonts/arial.ttf")) // 备用路径
        {
            std::cerr << "WarehouseRenderer - Font Arial.ttf not found. Interface labels may not display correctly." << std::endl;
        }
    }
}

void WarehouseRenderer::initialize(float trackRadius, const std::string &iconBasePath)
{
    // 清除现有数据
    m_interfaces.clear();
    m_labels.clear();

    // 加载设备图标纹理
    // (逻辑类似旧DeviceRenderer::loadResources)
    sf::Texture storageInTexture, storageOutTexture, workInTexture, workOutTexture, chargeTexture, defaultTexture;
    bool loadSuccess = true;

    auto loadTex = [&](sf::Texture &tex, const std::string &filename)
    {
        if (!tex.loadFromFile(iconBasePath + filename))
        {
            std::cerr << "WarehouseRenderer - Failed to load icon: " << iconBasePath + filename << std::endl;
            return false;
        }
        return true;
    };

    if (!loadTex(storageInTexture, "storage_in.png"))
        loadSuccess = false;
    if (!loadTex(storageOutTexture, "storage_out.png"))
        loadSuccess = false;
    if (!loadTex(workInTexture, "work_in.png"))
        loadSuccess = false;
    if (!loadTex(workOutTexture, "work_out.png"))
        loadSuccess = false;
    if (!loadTex(chargeTexture, "charge_station.png"))
        loadSuccess = false; // 假设有充电桩图标

    if (!loadSuccess)
    {
        std::cout << "WarehouseRenderer - Using default white square for some device icons." << std::endl;
        sf::Image defaultImage;
        defaultImage.create(32, 32, sf::Color::White);
        defaultTexture.loadFromImage(defaultImage);
        if (storageInTexture.getSize().x == 0)
            storageInTexture = defaultTexture;
        if (storageOutTexture.getSize().x == 0)
            storageOutTexture = defaultTexture;
        if (workInTexture.getSize().x == 0)
            workInTexture = defaultTexture;
        if (workOutTexture.getSize().x == 0)
            workOutTexture = defaultTexture;
        if (chargeTexture.getSize().x == 0)
            chargeTexture = defaultTexture;
    }

    m_iconTextures[gui::DeviceType::STORAGE_IN] = storageInTexture;
    m_iconTextures[gui::DeviceType::STORAGE_OUT] = storageOutTexture;
    m_iconTextures[gui::DeviceType::WORKSTATION_IN] = workInTexture;
    m_iconTextures[gui::DeviceType::WORKSTATION_OUT] = workOutTexture;
    m_iconTextures[gui::DeviceType::CHARGING_STATION] = chargeTexture;
    // 可以为更多类型添加图标，或提供一个通用默认图标
    if (defaultTexture.getSize().x > 0)
        m_iconTextures[gui::DeviceType::GENERIC_DEVICE] = defaultTexture;

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
    float visualWidth = interface.width * MM_TO_PIXEL;
    float visualHeight = interface.depth * MM_TO_PIXEL; // 深度在2D视图中表现为Y轴高度

    // 确定主体填充颜色和边框颜色
    sf::Color bodyFillColor;
    sf::Color bodyBorderColor = m_borderColor; // 默认边框

    switch (interface.state.status)
    {
    case gui::DeviceStatus::IDLE:
        bodyFillColor = COLOR_IDLE;
        break;
    case gui::DeviceStatus::WORKING:
    case gui::DeviceStatus::BUSY: // BUSY 映射到 WORKING
        bodyFillColor = COLOR_WORKING;
        break;
    case gui::DeviceStatus::FAULT:
        bodyFillColor = COLOR_FAULT;
        bodyBorderColor = sf::Color::Red; // 故障时边框红色高亮
        break;
    case gui::DeviceStatus::OFFLINE:
        bodyFillColor = COLOR_OFFLINE;
        break;
    default:
        bodyFillColor = sf::Color(100, 100, 100); // 未知状态
        break;
    }

    // 如果是输入/输出类型，可以用旧的颜色方案，否则用状态色
    // if (interface.type == InterfaceType::INPUT) {
    //     bodyFillColor = m_inputColor;
    // } else if (interface.type == InterfaceType::OUTPUT) {
    //     bodyFillColor = m_outputColor;
    // }
    // bodyFillColor.a = (interface.state.status == gui::DeviceStatus::IDLE) ? 180 : 255;

    // 创建伪3D效果
    // 1. 先绘制侧面和顶面（增加立体感）
    // sf::RectangleShape side(sf::Vector2f(visualWidth, interface.height * MM_TO_PIXEL * 0.3f));
    // side.setPosition(x - visualWidth / 2.0f, y - visualHeight / 2.0f);
    // side.setFillColor(sf::Color(bodyFillColor.r / 1.5, bodyFillColor.g / 1.5, bodyFillColor.b / 1.5));
    // target.draw(side, states);

    // 2. 主体矩形
    sf::RectangleShape body(sf::Vector2f(visualWidth, visualHeight));
    body.setPosition(x - visualWidth / 2.0f, y - visualHeight / 2.0f);
    body.setFillColor(bodyFillColor);
    body.setOutlineThickness(1.0f);
    body.setOutlineColor(bodyBorderColor);
    target.draw(body, states);

    // 3. 绘制阴影（增加立体感）
    // sf::RectangleShape shadow(sf::Vector2f(visualWidth, visualHeight / 4.0f));
    // shadow.setPosition(x - visualWidth / 2.0f, y + visualHeight / 2.0f);
    // shadow.setFillColor(m_shadowColor);
    // target.draw(shadow, states);

    // 4. 绘制图标
    sf::Sprite iconSprite;
    bool iconFound = false;
    if (m_iconTextures.count(interface.state.type))
    {
        iconSprite.setTexture(m_iconTextures.at(interface.state.type));
        iconFound = true;
    }
    else if (m_iconTextures.count(gui::DeviceType::GENERIC_DEVICE))
    { // 备用通用图标
        iconSprite.setTexture(m_iconTextures.at(gui::DeviceType::GENERIC_DEVICE));
        iconFound = true;
    }

    if (iconFound)
    {
        sf::FloatRect iconBounds = iconSprite.getLocalBounds();
        iconSprite.setOrigin(iconBounds.width / 2.f, iconBounds.height / 2.f);
        // 图标放在设备中心，可以根据需要调整缩放或位置
        float iconScale = std::min(visualWidth / iconBounds.width, visualHeight / iconBounds.height) * 0.6f;
        iconSprite.setScale(iconScale, iconScale);
        iconSprite.setPosition(x, y);
        target.draw(iconSprite, states);
    }

    // 5. 绘制ID文本 (绘制在设备下方)
    sf::Text idText;
    idText.setFont(m_font);
    idText.setString(interface.state.getId()); // 使用SimObject的ID
    idText.setCharacterSize(10);               // 调小字号
    idText.setFillColor(sf::Color::White);
    sf::FloatRect textBounds = idText.getLocalBounds();
    idText.setOrigin(textBounds.left + textBounds.width / 2.0f, textBounds.top + textBounds.height / 2.0f);
    idText.setPosition(x, y + visualHeight / 2.0f + textBounds.height / 1.5f + 2.f); // 图标下方
    target.draw(idText, states);

    // 6. 绘制任务队列计数 (如果存在，绘制在右上角)
    if (interface.state.queuedTaskCount > 0)
    {
        sf::CircleShape taskCountBg(8.0f); // 调小背景
        taskCountBg.setFillColor(sf::Color(230, 60, 60, 200));
        taskCountBg.setOrigin(taskCountBg.getRadius(), taskCountBg.getRadius());
        taskCountBg.setPosition(x + visualWidth / 2.f - taskCountBg.getRadius(), y - visualHeight / 2.f + taskCountBg.getRadius());

        sf::Text taskCountText;
        taskCountText.setFont(m_font);
        taskCountText.setString(std::to_string(interface.state.queuedTaskCount));
        taskCountText.setCharacterSize(10); // 调小字号
        taskCountText.setFillColor(sf::Color::White);
        sf::FloatRect qTextBounds = taskCountText.getLocalBounds();
        taskCountText.setOrigin(qTextBounds.left + qTextBounds.width / 2.0f, qTextBounds.top + qTextBounds.height / 2.0f);
        taskCountText.setPosition(taskCountBg.getPosition().x, taskCountBg.getPosition().y - 1.f); // 微调y

        target.draw(taskCountBg, states);
        target.draw(taskCountText, states);
    }
}

// 添加辅助函数
// gui::DeviceState warehouseToDeviceState(const WarehouseState &warehouse, bool isInput)
// {
// 不能直接访问id/type，需通过构造
// gui::DeviceState device(std::to_string(warehouse.id), sf::Vector2f(0, 0),
// isInput ? gui::DeviceType::INPUT_STATION : gui::DeviceType::OUTPUT_STATION,
// gui::DeviceStatus::IDLE, "");
// 其它字段可按需赋值
// return device;
// }
// 上面的转换函数不再需要，因为WarehouseInterface直接使用DeviceState

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
        std::string labelText = "ID:" + std::to_string(interface.id) + " " +
                                ((interface.type == InterfaceType::INPUT) ? "Input" : "Output");
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
        std::string labelText = "ID:" + std::to_string(interface.id) + " " +
                                ((interface.type == InterfaceType::INPUT) ? "Input" : "Output");
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
