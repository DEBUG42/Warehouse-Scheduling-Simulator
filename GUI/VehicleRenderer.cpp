#include "VehicleRenderer.hpp"
#include "../Core/Vehicle.hpp"
#include <iostream>
#include <cmath>

/**
 * @brief 构造函数，加载资源
 */
VehicleRenderer::VehicleRenderer()
{
    // 加载字体
    if (!m_font.loadFromFile("resources/fonts/Arial.ttf"))
    {
        // 尝试多个可能的路径
        if (m_font.loadFromFile("GUI/resources/fonts/Arial.ttf"))
        {
            // 成功加载
        }
        else if (m_font.loadFromFile("fonts/Arial.ttf"))
        {
            // 成功加载
        }
        else
        {
            std::cerr << "警告：无法加载字体文件，车辆标签可能无法正确显示" << std::endl;
        }
    }
}

/**
 * @brief 更新车辆位置，根据轨道路程计算实际坐标和朝向
 *
 * 轨道坐标系说明：
 * - 原点(0,0)位于左侧弯道中心
 * - 轨道路程起点为左下角弯道与直道交界处
 * - 逆时针绕轨道行驶
 *
 * @param vehicle 车辆状态
 * @param trackLength 轨道总长度(mm)
 * @param curveRadius 弯道半径(mm)
 * @param position 输出参数，返回计算后的位置
 * @param rotation 输出参数，返回计算后的角度
 */
void VehicleRenderer::calculatePosition(const VehicleState &vehicle,
                                        float trackLength,
                                        float curveRadius,
                                        sf::Vector2f &position,
                                        float &rotation)
{
    // 计算轨道各段长度
    float straightLength = 40000.0f;                                             // 直道长度固定为40000mm
    float leftCurveLength = M_PI * curveRadius;                                  // 左弯道长度
    float rightCurveLength = M_PI * curveRadius;                                 // 右弯道长度
    float totalLength = 2 * straightLength + leftCurveLength + rightCurveLength; // 轨道总长度

    // 标准化位置：确保位置在轨道总长度内
    float position_mm = fmod(vehicle.position, totalLength);
    if (position_mm < 0)
        position_mm += totalLength;

    // 确定车辆在轨道哪一段
    // 段1: 下方直道 (0 ~ straightLength)
    // 段2: 右侧弯道 (straightLength ~ straightLength + rightCurveLength)
    // 段3: 上方直道 (straightLength + rightCurveLength ~ 2*straightLength + rightCurveLength)
    // 段4: 左侧弯道 (2*straightLength + rightCurveLength ~ totalLength)

    // 转换为像素坐标
    float curveRadius_px = curveRadius * MM_TO_PIXEL;
    float pos_x = 0.0f, pos_y = 0.0f;
    float angle_degrees = 0.0f;

    if (position_mm < straightLength)
    {
        // 段1: 下方直道
        pos_x = position_mm * MM_TO_PIXEL;
        pos_y = -curveRadius * MM_TO_PIXEL;
        angle_degrees = 0.0f; // 朝右
    }
    else if (position_mm < straightLength + rightCurveLength)
    {
        // 段2: 右侧弯道
        float angle = (position_mm - straightLength) / curveRadius; // 弧度
        pos_x = straightLength * MM_TO_PIXEL + curveRadius_px * sin(angle);
        pos_y = -curveRadius_px * cos(angle);
        angle_degrees = angle * 180.0f / M_PI; // 转换为角度
    }
    else if (position_mm < 2 * straightLength + rightCurveLength)
    {
        // 段3: 上方直道
        float pos_on_segment = position_mm - (straightLength + rightCurveLength);
        pos_x = (straightLength - pos_on_segment) * MM_TO_PIXEL; // 从右向左
        pos_y = curveRadius * MM_TO_PIXEL;
        angle_degrees = 180.0f; // 朝左
    }
    else
    {
        // 段4: 左侧弯道
        float angle = (position_mm - (2 * straightLength + rightCurveLength)) / curveRadius; // 弧度
        pos_x = -curveRadius_px * sin(angle);
        pos_y = curveRadius_px * cos(angle);
        angle_degrees = 180.0f + angle * 180.0f / M_PI; // 转换为角度
    }

    // 设置输出参数
    position.x = pos_x;
    position.y = pos_y;
    rotation = angle_degrees;
}

/**
 * @brief 绘制单个车辆
 *
 * 根据车辆状态、位置和朝向绘制车辆图形。图形包括:
 * - 矩形车身，颜色根据载货状态变化
 * - 三角形方向指示器显示车头朝向
 * - 车辆ID标签
 * - 速度指示条（随速度变化）
 * - 3D效果，包括阴影和侧面
 *
 * @param target 渲染目标
 * @param vehicle 车辆数据引用
 * @param position 车辆世界坐标
 * @param rotation 车辆朝向角度
 */
void VehicleRenderer::renderVehicle(sf::RenderTarget &target,
                                    const VehicleState &vehicle,
                                    const sf::Vector2f &position,
                                    float rotation)
{
    // 根据车辆状态确定颜色
    sf::Color bodyColor;
    if (vehicle.currentTaskId >= 0)
    {
        bodyColor = m_colorAssigned; // 已分配任务
    }
    else if (vehicle.isLoaded)
    {
        bodyColor = m_colorLoaded; // 载货状态
    }
    else
    {
        bodyColor = m_colorEmpty; // 空载状态
    }

    // 变换矩阵，用于应用旋转和位置
    sf::Transform transform;
    transform.translate(position);
    transform.rotate(rotation);

    // 创建伪3D效果
    // 1. 阴影 - 绘制椭圆形阴影在车辆正下方
    sf::CircleShape shadow(m_baseSize.x / 2.0f);
    shadow.setScale(1.0f, 0.3f); // 扁平的椭圆
    shadow.setOrigin(shadow.getRadius(), shadow.getRadius());
    shadow.setFillColor(m_shadowColor);
    shadow.setPosition(position.x, position.y + 5.0f); // 稍微偏移一点
    target.draw(shadow);

    // 2. 车辆侧面 - 增加立体感
    sf::RectangleShape side(sf::Vector2f(m_baseSize.x, m_height));
    side.setOrigin(m_baseSize.x / 2, m_height / 2);
    side.setFillColor(sf::Color(
        bodyColor.r * 0.7,
        bodyColor.g * 0.7,
        bodyColor.b * 0.7));
    target.draw(side, transform);

    // 3. 车辆顶面 - 主体
    sf::RectangleShape vehicleBody(m_baseSize);
    vehicleBody.setOrigin(m_baseSize.x / 2, m_baseSize.y / 2);
    vehicleBody.setFillColor(bodyColor);
    vehicleBody.setOutlineThickness(1.0f);
    vehicleBody.setOutlineColor(sf::Color(50, 50, 50));
    target.draw(vehicleBody, transform);

    // 4. 方向指示器（小三角形表示车头）
    sf::ConvexShape arrow;
    arrow.setPointCount(3);
    arrow.setPoint(0, sf::Vector2f(m_baseSize.x / 2 + 3, 0));
    arrow.setPoint(1, sf::Vector2f(m_baseSize.x / 3, -m_baseSize.y / 3));
    arrow.setPoint(2, sf::Vector2f(m_baseSize.x / 3, m_baseSize.y / 3));
    arrow.setOrigin(m_baseSize.x / 3, 0);
    arrow.setFillColor(sf::Color(220, 220, 220));
    target.draw(arrow, transform);

    // 5. 显示速度条 - 根据速度值显示在车辆后方
    if (vehicle.speed > 0.1f)
    {                          // 只有速度大于0.1时才显示
        float maxSpeed = 5.0f; // 假设最大速度5m/s
        float speedRatio = std::min(vehicle.speed / maxSpeed, 1.0f);

        // 速度条随速度变化长度和颜色
        sf::RectangleShape speedBar(sf::Vector2f(speedRatio * m_baseSize.x * 0.8f, 2.0f));
        speedBar.setOrigin(-m_baseSize.x / 2, 1.0f);

        // 颜色从绿到红
        sf::Color speedColor(
            static_cast<sf::Uint8>(255 * speedRatio),
            static_cast<sf::Uint8>(255 * (1.0f - speedRatio)),
            0);
        speedBar.setFillColor(speedColor);
        target.draw(speedBar, transform);
    }

    // 6. 显示车辆ID
    if (m_font.getInfo().family != "")
    {
        // 创建文本对象
        sf::Text idText;
        idText.setFont(m_font);
        idText.setString(std::to_string(vehicle.id));
        idText.setCharacterSize(12);
        idText.setFillColor(sf::Color::White);
        idText.setOutlineThickness(1.0f);
        idText.setOutlineColor(sf::Color::Black);

        // 计算文本位置（不随车辆旋转）
        sf::FloatRect textBounds = idText.getLocalBounds();
        idText.setOrigin(textBounds.width / 2, textBounds.height / 2);
        idText.setPosition(position);

        target.draw(idText);
    }

    // 7. 如果有任务，显示任务ID
    if (vehicle.currentTaskId >= 0)
    {
        sf::Text taskText;
        taskText.setFont(m_font);
        taskText.setString("T" + std::to_string(vehicle.currentTaskId));
        taskText.setCharacterSize(10);
        taskText.setFillColor(sf::Color::Yellow);

        // 放在车辆上方
        sf::FloatRect textBounds = taskText.getLocalBounds();
        taskText.setOrigin(textBounds.width / 2, textBounds.height + 5);
        taskText.setPosition(position);

        target.draw(taskText);
    }
}

/**
 * @brief 渲染车辆阴影以增强3D效果
 * @param target 渲染目标
 * @param position 车辆世界坐标
 * @param rotation 车辆朝向角度
 */
void VehicleRenderer::renderShadow(sf::RenderTarget &target,
                                   const sf::Vector2f &position,
                                   float rotation)
{
    // 创建阴影的椭圆形状
    sf::CircleShape shadow(m_baseSize.x / 2.0f);
    shadow.setScale(1.0f, 0.3f); // 扁平的椭圆
    shadow.setOrigin(shadow.getRadius(), shadow.getRadius());
    shadow.setFillColor(sf::Color(20, 20, 20, 60)); // 更淡的阴影

    // 计算阴影偏移（随角度变化）
    float offsetX = 4.0f * std::cos((rotation - 90) * M_PI / 180.0f);
    float offsetY = 4.0f * std::sin((rotation - 90) * M_PI / 180.0f);
    shadow.setPosition(position.x + offsetX, position.y + offsetY + 5.0f);

    // 绘制阴影
    target.draw(shadow);
}