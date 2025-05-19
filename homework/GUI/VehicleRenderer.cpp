#include "VehicleRenderer.hpp"
#include <cmath>
#include <iostream>

/**
 * @brief 构造函数
 *
 * 初始化渲染器，尝试加载字体
 */
VehicleRenderer::VehicleRenderer()
{
    tryLoadFont();
}

/**
 * @brief 更新车辆状态数据
 * @param vehicles 车辆状态数组
 */
void VehicleRenderer::updateVehicles(const std::vector<VehicleState> &vehicles)
{
    m_vehicles = vehicles;
}

/**
 * @brief 设置字体
 * @param font 字体对象引用
 */
void VehicleRenderer::setFont(const sf::Font &font)
{
    m_font = font;
    m_fontLoaded = true;
}

/**
 * @brief 计算车辆在轨道上的世界坐标和朝向角度
 *
 * 根据车辆在轨道上的位置计算其在世界中的坐标和朝向
 * 考虑直道和弯道的几何形状，确保车辆跟随轨道曲线
 *
 * @param trackPosition 轨道位置（毫米）
 * @param trackLength 轨道总长（毫米）
 * @param straightLength 直道长度（毫米）
 * @param curveRadius 弯道半径（毫米）
 * @param position 输出参数，世界坐标（像素）
 * @param rotation 输出参数，旋转角度（度）
 */
void VehicleRenderer::calculateWorldPositionAndRotation(
    float trackPosition,
    float trackLength,
    float straightLength,
    float curveRadius,
    sf::Vector2f &position,
    float &rotation) const
{
    // 转换为像素单位
    float R_px = mmToPixel(curveRadius);
    float L_px = mmToPixel(straightLength);

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

    if (pos_px < upperStraight)
    {
        // 在上直道
        position.x = pos_px - L_px / 2;
        position.y = -R_px;
        rotation = 0.0f; // 向右
    }
    else if (pos_px < rightCurve)
    {
        // 在右弯道
        float angle = (pos_px - upperStraight) / R_px;
        position.x = L_px / 2 + R_px * sin(angle);
        position.y = -R_px + R_px * cos(angle);
        rotation = angle * 180.0f / M_PI; // 角度转换为度
    }
    else if (pos_px < lowerStraight)
    {
        // 在下直道
        position.x = L_px / 2 - (pos_px - rightCurve);
        position.y = R_px;
        rotation = 180.0f; // 向左
    }
    else
    {
        // 在左弯道
        float angle = (pos_px - lowerStraight) / R_px;
        position.x = -L_px / 2 - R_px * sin(angle);
        position.y = R_px - R_px * cos(angle);
        rotation = (angle * 180.0f / M_PI) + 180.0f; // 角度转换为度
    }
}

/**
 * @brief 重载的绘制方法
 * @param target 渲染目标
 * @param states 渲染状态
 */
void VehicleRenderer::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    if (m_vehicles.empty())
    {
        return;
    }

    // 获取第一个车辆的轨道长度信息（假设所有车辆共用同一轨道）
    // 在实际应用中，这些参数应该从轨道渲染器或仿真引擎获取
    float trackLength = 126000.0f;   // 轨道总长（毫米）
    float straightLength = 40000.0f; // 直道长度（毫米）
    float curveRadius = 2500.0f;     // 弯道半径（毫米）

    // 绘制所有车辆
    for (const auto &vehicle : m_vehicles)
    {
        // 计算世界坐标和旋转角度
        sf::Vector2f position;
        float rotation;
        calculateWorldPositionAndRotation(
            vehicle.trackPosition,
            trackLength,
            straightLength,
            curveRadius,
            position,
            rotation);

        // 绘制单个车辆
        renderVehicle(target, vehicle, position, rotation);
    }
}

/**
 * @brief 绘制单个车辆
 * @param target 渲染目标
 * @param vehicle 车辆数据引用
 * @param position 车辆世界坐标（像素）
 * @param rotation 车辆朝向角度（度）
 */
void VehicleRenderer::renderVehicle(sf::RenderTarget &target,
                                    const VehicleState &vehicle,
                                    const sf::Vector2f &position,
                                    float rotation) const
{
    // 创建车辆底部阴影
    sf::RectangleShape shadow(sf::Vector2f(m_baseSize.x * 1.1f, m_baseSize.y * 1.1f));
    shadow.setOrigin(shadow.getSize().x / 2, shadow.getSize().y / 2);
    shadow.setPosition(position + sf::Vector2f(2.0f, 2.0f)); // 轻微偏移阴影
    shadow.setRotation(rotation);
    shadow.setFillColor(sf::Color(0, 0, 0, 80)); // 半透明黑色阴影
    target.draw(shadow);

    // 创建车辆主体形状 - 尺寸已通过 MM_TO_PIXEL 转换为像素单位
    sf::RectangleShape vehicleBody(m_baseSize);
    vehicleBody.setOrigin(m_baseSize.x / 2, m_baseSize.y / 2); // 设置原点为中心
    vehicleBody.setPosition(position);
    vehicleBody.setRotation(rotation); // 设置旋转角度（度）

    // 根据车辆状态设置颜色
    sf::Color bodyColor;
    if (vehicle.currentTaskId >= 0)
    {
        bodyColor = m_colorAssigned; // 已分配任务状态
    }
    else if (vehicle.isLoaded)
    {
        bodyColor = m_colorLoaded; // 载货状态
    }
    else
    {
        bodyColor = m_colorEmpty; // 空载状态
    }
    vehicleBody.setFillColor(bodyColor);

    // 绘制车辆主体
    target.draw(vehicleBody);

    // 添加3D效果 - 车顶
    float depth = m_baseSize.y * 0.3f; // 3D深度

    sf::ConvexShape top;
    top.setPointCount(4);
    top.setPoint(0, sf::Vector2f(-m_baseSize.x / 2 + depth, -m_baseSize.y / 2 + depth));
    top.setPoint(1, sf::Vector2f(m_baseSize.x / 2 - depth, -m_baseSize.y / 2 + depth));
    top.setPoint(2, sf::Vector2f(m_baseSize.x / 2 - depth, m_baseSize.y / 2 - depth));
    top.setPoint(3, sf::Vector2f(-m_baseSize.x / 2 + depth, m_baseSize.y / 2 - depth));

    // 顶部颜色稍亮
    sf::Color topColor = bodyColor;
    topColor.r = std::min(255, static_cast<int>(bodyColor.r * 1.3f));
    topColor.g = std::min(255, static_cast<int>(bodyColor.g * 1.3f));
    topColor.b = std::min(255, static_cast<int>(bodyColor.b * 1.3f));
    top.setFillColor(topColor);
    top.setPosition(position);
    top.setRotation(rotation);
    top.setOrigin(0, 0);

    target.draw(top);

    // 添加侧面高光
    sf::VertexArray highlight(sf::Lines, 4);

    // 计算边角点坐标（考虑旋转）
    float rad = rotation * M_PI / 180.0f;
    float cosRad = cos(rad);
    float sinRad = sin(rad);

    sf::Vector2f topLeft = position + sf::Vector2f(
                                          (-m_baseSize.x / 2 + depth) * cosRad - (-m_baseSize.y / 2 + depth) * sinRad,
                                          (-m_baseSize.x / 2 + depth) * sinRad + (-m_baseSize.y / 2 + depth) * cosRad);

    sf::Vector2f topRight = position + sf::Vector2f(
                                           (m_baseSize.x / 2 - depth) * cosRad - (-m_baseSize.y / 2 + depth) * sinRad,
                                           (m_baseSize.x / 2 - depth) * sinRad + (-m_baseSize.y / 2 + depth) * cosRad);

    sf::Vector2f bottomRight = position + sf::Vector2f(
                                              (m_baseSize.x / 2) * cosRad - (m_baseSize.y / 2) * sinRad,
                                              (m_baseSize.x / 2) * sinRad + (m_baseSize.y / 2) * cosRad);

    highlight[0] = sf::Vertex(topRight, sf::Color(255, 255, 255, 100));
    highlight[1] = sf::Vertex(bottomRight, sf::Color(255, 255, 255, 100));

    highlight[2] = sf::Vertex(topRight, sf::Color(255, 255, 255, 100));
    highlight[3] = sf::Vertex(topLeft, sf::Color(255, 255, 255, 100));

    target.draw(highlight);

    // 添加方向指示器（小三角形表示车头）
    sf::ConvexShape arrow;
    arrow.setPointCount(3);
    arrow.setPoint(0, sf::Vector2f(m_baseSize.x / 2, 0));
    arrow.setPoint(1, sf::Vector2f(m_baseSize.x / 4, -m_baseSize.y / 4));
    arrow.setPoint(2, sf::Vector2f(m_baseSize.x / 4, m_baseSize.y / 4));

    // 设置箭头变换
    arrow.setOrigin(m_baseSize.x / 4, 0);
    arrow.setPosition(position);
    arrow.setRotation(rotation);
    arrow.setFillColor(sf::Color(220, 220, 220)); // 浅灰色箭头

    // 绘制方向指示器
    target.draw(arrow);

    // 显示车辆ID
    if (m_fontLoaded)
    {
        sf::Text idText;
        idText.setFont(m_font);
        idText.setString(std::to_string(vehicle.id));
        idText.setCharacterSize(12);
        idText.setFillColor(sf::Color::White);

        // 文本居中
        sf::FloatRect textBounds = idText.getLocalBounds();
        idText.setOrigin(textBounds.width / 2, textBounds.height / 2);
        idText.setPosition(position);

        target.draw(idText);
    }

    // 如果车辆正在移动，显示速度指示器
    if (vehicle.speed > 0.1f)
    {
        // 速度指示器（根据速度显示不同数量的条纹）
        float maxSpeed = 10.0f; // 假设的最大速度（米/秒）
        int barCount = static_cast<int>(std::ceil(vehicle.speed / maxSpeed * 3.0f));
        barCount = std::min(barCount, 3); // 最多3条速度条

        for (int i = 0; i < barCount; ++i)
        {
            sf::RectangleShape speedBar(sf::Vector2f(3.0f, 5.0f));
            speedBar.setOrigin(1.5f, 2.5f);

            // 计算速度条位置（在车辆左侧）
            sf::Vector2f barOffset(-m_baseSize.x / 2 - 5.0f - (i * 5.0f), 0.0f);

            // 根据车辆旋转角度调整速度条位置
            float angle = rotation * M_PI / 180.0f; // 角度转弧度
            sf::Vector2f rotatedBarOffset(
                barOffset.x * cos(angle) - barOffset.y * sin(angle),
                barOffset.x * sin(angle) + barOffset.y * cos(angle));

            speedBar.setPosition(position + rotatedBarOffset);
            speedBar.setRotation(rotation);
            speedBar.setFillColor(sf::Color(100, 200, 255)); // 蓝色速度条

            target.draw(speedBar);
        }
    }

    // 如果车辆载货，显示货物图标
    if (vehicle.isLoaded)
    {
        sf::RectangleShape cargo(sf::Vector2f(m_baseSize.x * 0.5f, m_baseSize.y * 0.5f));
        cargo.setOrigin(cargo.getSize().x / 2, cargo.getSize().y / 2);
        cargo.setPosition(position);
        cargo.setRotation(rotation);
        cargo.setFillColor(sf::Color(200, 150, 50)); // 货物颜色
        cargo.setOutlineColor(sf::Color(100, 80, 30));
        cargo.setOutlineThickness(1.0f);

        target.draw(cargo);
    }
}

/**
 * @brief 尝试加载字体
 * @return 是否成功加载
 */
bool VehicleRenderer::tryLoadFont()
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
