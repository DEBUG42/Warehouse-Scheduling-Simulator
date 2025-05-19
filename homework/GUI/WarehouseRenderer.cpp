#include "WarehouseRenderer.hpp"
#include <cmath>
#include <iostream>

/**
 * @brief 构造函数
 *
 * 初始化渲染器，尝试加载字体
 */
WarehouseRenderer::WarehouseRenderer()
{
    tryLoadFont();
}

/**
 * @brief 更新仓库/接口设备状态数据
 * @param warehouses 仓库/接口设备状态数组
 */
void WarehouseRenderer::updateWarehouses(const std::vector<WarehouseState> &warehouses)
{
    m_warehouses = warehouses;
}

/**
 * @brief 设置字体
 * @param font 字体对象引用
 */
void WarehouseRenderer::setFont(const sf::Font &font)
{
    m_font = font;
    m_fontLoaded = true;
}

/**
 * @brief 获取设备渲染尺寸
 * @param isInterface 是否为接口设备
 * @return 设备尺寸（像素）
 */
sf::Vector2f WarehouseRenderer::getSize(bool isInterface) const
{
    if (isInterface)
    {
        return {m_interfaceWidth, m_interfaceHeight};
    }
    else
    {
        return {m_warehouseWidth, m_warehouseHeight};
    }
}

/**
 * @brief 计算世界坐标
 *
 * 根据轨道位置计算仓库/接口设备在世界中的坐标
 * 考虑直道和弯道的几何形状，确保设备正确定位在轨道旁边
 *
 * @param trackPosition 轨道位置（毫米）
 * @param trackLength 轨道总长（毫米）
 * @param straightLength 直道长度（毫米）
 * @param curveRadius 弯道半径（毫米）
 * @return 世界坐标（像素）
 */
sf::Vector2f WarehouseRenderer::calculateWorldPosition(
    float trackPosition,
    float trackLength,
    float straightLength,
    float curveRadius) const
{
    // 转换为像素单位
    float R_px = mmToPixel(curveRadius);
    float L_px = mmToPixel(straightLength);
    float trackOffset_px = m_trackOffset;

    // 确保轨道位置在有效范围内（0 ~ trackLength）
    while (trackPosition < 0)
    {
        trackPosition += trackLength;
    }
    while (trackPosition >= trackLength)
    {
        trackPosition -= trackLength;
    }

    // 转换为轨道相对位置（0~1）
    float relativePos = trackPosition / trackLength;

    // 轨道总长度（像素）= 2 * 直道长度 + 2 * π * 弯道半径
    float totalLength_px = 2 * L_px + 2 * M_PI * R_px;
    float pos_px = relativePos * totalLength_px;

    // 确定在轨道的哪个部分
    float upperStraight = L_px;                     // 上直道终点
    float rightCurve = upperStraight + M_PI * R_px; // 右弯道终点
    float lowerStraight = rightCurve + L_px;        // 下直道终点
    // leftCurve = totalLength_px                    // 左弯道终点（即轨道总长）

    sf::Vector2f position;

    if (pos_px < upperStraight)
    {
        // 在上直道
        float x = pos_px - L_px / 2;
        position = {x, -R_px - trackOffset_px};
    }
    else if (pos_px < rightCurve)
    {
        // 在右弯道
        float angle = (pos_px - upperStraight) / R_px;
        position = {
            L_px / 2 + (R_px + trackOffset_px) * sin(angle),
            -R_px + (R_px + trackOffset_px) * cos(angle)};
    }
    else if (pos_px < lowerStraight)
    {
        // 在下直道
        float x = L_px / 2 - (pos_px - rightCurve);
        position = {x, R_px + trackOffset_px};
    }
    else
    {
        // 在左弯道
        float angle = (pos_px - lowerStraight) / R_px;
        position = {
            -L_px / 2 - (R_px + trackOffset_px) * sin(angle),
            R_px - (R_px + trackOffset_px) * cos(angle)};
    }

    return position;
}

/**
 * @brief 重载的绘制方法
 * @param target 渲染目标
 * @param states 渲染状态
 */
void WarehouseRenderer::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    if (m_warehouses.empty())
    {
        return;
    }

    // 获取第一个仓库的轨道长度信息（假设所有仓库共用同一轨道）
    // 在实际应用中，这些参数应该从轨道渲染器或仿真引擎获取
    float trackLength = 126000.0f;   // 轨道总长（毫米）
    float straightLength = 40000.0f; // 直道长度（毫米）
    float curveRadius = 2500.0f;     // 弯道半径（毫米）

    // 绘制所有仓库/接口设备
    for (const auto &warehouse : m_warehouses)
    {
        // 计算世界坐标
        sf::Vector2f position = calculateWorldPosition(
            warehouse.trackPosition,
            trackLength,
            straightLength,
            curveRadius);

        // 绘制单个仓库/接口设备
        renderWarehouse(target, warehouse, position);
    }
}

/**
 * @brief 绘制单个仓库/接口设备
 * @param target 渲染目标
 * @param warehouse 仓库/接口设备状态数据
 * @param position 世界坐标（像素）
 */
void WarehouseRenderer::renderWarehouse(sf::RenderTarget &target, const WarehouseState &warehouse, const sf::Vector2f &position) const
{
    // 根据类型确定形状和颜色
    sf::Vector2f size = getSize(warehouse.isInterface);
    sf::RectangleShape mainShape(size);
    mainShape.setOrigin(size.x / 2, size.y / 2);
    mainShape.setPosition(position);

    // 设置基础颜色
    sf::Color baseColor = warehouse.isInterface ? m_interfaceColor : m_warehouseColor;

    // 根据负载调整颜色明暗
    float loadRatio = static_cast<float>(warehouse.currentLoad) / warehouse.capacity;
    sf::Color color = baseColor;

    // 负载较高时颜色变深
    if (loadRatio > 0.7f)
    {
        color.r = static_cast<sf::Uint8>(baseColor.r * 0.8f);
        color.g = static_cast<sf::Uint8>(baseColor.g * 0.8f);
        color.b = static_cast<sf::Uint8>(baseColor.b * 0.8f);
    }
    // 负载较低时颜色变亮
    else if (loadRatio < 0.3f)
    {
        color.r = std::min(255, static_cast<int>(baseColor.r * 1.2f));
        color.g = std::min(255, static_cast<int>(baseColor.g * 1.2f));
        color.b = std::min(255, static_cast<int>(baseColor.b * 1.2f));
    }

    mainShape.setFillColor(color);

    // 添加3D效果：顶面和侧面
    sf::ConvexShape top;
    top.setPointCount(4);
    float depthFactor = 0.2f; // 3D深度因子
    float depth = size.y * depthFactor;

    // 设置顶面的四个顶点
    top.setPoint(0, position + sf::Vector2f(-size.x / 2, -size.y / 2));
    top.setPoint(1, position + sf::Vector2f(size.x / 2, -size.y / 2));
    top.setPoint(2, position + sf::Vector2f(size.x / 2 - depth, -size.y / 2 - depth));
    top.setPoint(3, position + sf::Vector2f(-size.x / 2 - depth, -size.y / 2 - depth));

    // 顶面颜色比主体稍亮
    sf::Color topColor = color;
    topColor.r = std::min(255, static_cast<int>(color.r * 1.3f));
    topColor.g = std::min(255, static_cast<int>(color.g * 1.3f));
    topColor.b = std::min(255, static_cast<int>(color.b * 1.3f));
    top.setFillColor(topColor);

    // 侧面
    sf::ConvexShape side;
    side.setPointCount(4);
    side.setPoint(0, position + sf::Vector2f(size.x / 2, -size.y / 2));
    side.setPoint(1, position + sf::Vector2f(size.x / 2, size.y / 2));
    side.setPoint(2, position + sf::Vector2f(size.x / 2 - depth, size.y / 2 - depth));
    side.setPoint(3, position + sf::Vector2f(size.x / 2 - depth, -size.y / 2 - depth));

    // 侧面颜色比主体稍暗
    sf::Color sideColor = color;
    sideColor.r = static_cast<sf::Uint8>(color.r * 0.7f);
    sideColor.g = static_cast<sf::Uint8>(color.g * 0.7f);
    sideColor.b = static_cast<sf::Uint8>(color.b * 0.7f);
    side.setFillColor(sideColor);

    // 绘制主体
    target.draw(mainShape);

    // 绘制3D效果
    target.draw(side);
    target.draw(top);

    // 显示负载指示条
    sf::RectangleShape loadBar({size.x * 0.8f, 5.0f});
    loadBar.setOrigin(loadBar.getSize().x / 2, loadBar.getSize().y / 2);
    loadBar.setPosition(position + sf::Vector2f(0, size.y * 0.4f));
    loadBar.setFillColor(sf::Color(50, 50, 50));
    target.draw(loadBar);

    // 负载进度条
    sf::RectangleShape loadProgress({loadBar.getSize().x * loadRatio, loadBar.getSize().y});
    loadProgress.setOrigin(loadBar.getSize().x / 2, loadProgress.getSize().y / 2);
    loadProgress.setPosition(position + sf::Vector2f(0, size.y * 0.4f));

    // 负载颜色：从绿色(低)到红色(高)
    sf::Color loadColor;
    if (loadRatio < 0.5f)
    {
        loadColor = sf::Color(
            static_cast<sf::Uint8>(255 * loadRatio * 2),
            255,
            0);
    }
    else
    {
        loadColor = sf::Color(
            255,
            static_cast<sf::Uint8>(255 * (1 - (loadRatio - 0.5f) * 2)),
            0);
    }
    loadProgress.setFillColor(loadColor);
    target.draw(loadProgress);

    // 显示ID和类型
    if (m_fontLoaded)
    {
        sf::Text idText;
        idText.setFont(m_font);
        idText.setString(std::to_string(warehouse.id) + (warehouse.isInterface ? "I" : "W"));
        idText.setCharacterSize(12);
        idText.setFillColor(sf::Color::White);

        // 文本居中
        sf::FloatRect textBounds = idText.getLocalBounds();
        idText.setOrigin(textBounds.width / 2, textBounds.height / 2);
        idText.setPosition(position);

        target.draw(idText);
    }
}

/**
 * @brief 尝试加载字体
 * @return 是否成功加载
 */
bool WarehouseRenderer::tryLoadFont()
{
    // 尝试多个可能的路径
    std::vector<std::string> fontPaths = {
        "resources/fonts/arial.ttf",
        "GUI/resources/fonts/arial.ttf",
        "fonts/arial.ttf",
        "C:/Windows/Fonts/arial.ttf"};

    for (const auto &path : fontPaths)
    {
        try
        {
            if (m_font.loadFromFile(path))
            {
                m_fontLoaded = true;
                return true;
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << "尝试加载字体失败: " << path << " - " << e.what() << std::endl;
        }
    }

    std::cerr << "无法加载任何字体，将使用默认显示" << std::endl;
    return false;
}
