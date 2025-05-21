#include "gui/VehicleRenderer.hpp"
#include "Core/Vehicle.hpp"
#include <iostream>
#include <cmath>

// 定义车体和状态框的尺寸 (假设是后端坐标系下的毫米)
const float BODY_WIDTH_MM = 2000.0f;
const float BODY_HEIGHT_MM = 1000.0f;
const float STATUS_BORDER_MM = 100.0f; // 状态框比车体每边宽 STATUS_BORDER_MM
// MM_TO_PX 常量已从此移除，将使用头文件中的 MM_TO_PIXEL 成员变量

/**
 * @brief 构造函数，加载资源
 */
VehicleRenderer::VehicleRenderer(const sf::Font &font) : m_font(font)
{
    // 加载字体 (这部分字体加载逻辑在您提供的 VehicleRenderer.cpp 中，但通常字体应由外部资源管理器传入引用)
    // 如果 m_font 是构造函数传入的引用，则不需要在这里 loadFromFile
    // 为保持与您提供的版本一致，暂时保留这里的检查，但理想情况下应移除
    // if (!m_font.getInfo().family.empty()) // 检查字体是否有效，而不是重新加载
    // {
    //     // 字体有效
    // }
    // else
    // {
    //     std::cerr << "警告：传入 VehicleRenderer 的字体无效，车辆标签可能无法正确显示" << std::endl;
    // }

    // 车体尺寸 (转换为像素)
    float bodyWidthPx = BODY_WIDTH_MM * MM_TO_PIXEL;   // 使用成员 MM_TO_PIXEL
    float bodyHeightPx = BODY_HEIGHT_MM * MM_TO_PIXEL; // 使用成员 MM_TO_PIXEL
    m_body.setSize(sf::Vector2f(bodyWidthPx, bodyHeightPx));
    m_body.setOrigin(bodyWidthPx / 2.f, bodyHeightPx / 2.f); // 中心原点
    m_body.setFillColor(sf::Color(0, 120, 255));             // 默认蓝色车体
    m_body.setOutlineThickness(1.f);                         // 给车体一个细边框
    m_body.setOutlineColor(sf::Color::Black);

    // 状态框尺寸 (转换为像素)
    float statusWidthPx = (BODY_WIDTH_MM + 2 * STATUS_BORDER_MM) * MM_TO_PIXEL;   // 使用成员 MM_TO_PIXEL
    float statusHeightPx = (BODY_HEIGHT_MM + 2 * STATUS_BORDER_MM) * MM_TO_PIXEL; // 使用成员 MM_TO_PIXEL
    m_statusBounds.setSize(sf::Vector2f(statusWidthPx, statusHeightPx));
    m_statusBounds.setOrigin(statusWidthPx / 2.f, statusHeightPx / 2.f); // 中心原点

    m_directionIndicator.setRadius(std::max(1.f, bodyHeightPx * 0.15f));
    m_directionIndicator.setOrigin(m_directionIndicator.getRadius(), m_directionIndicator.getRadius());
    m_directionIndicator.setFillColor(sf::Color::White);

    m_idText.setFont(m_font);
    m_idText.setCharacterSize(static_cast<unsigned int>(std::max(8.f, bodyHeightPx * 0.3f)));
    m_idText.setFillColor(sf::Color::Black);
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
 * @param totalStraightLengthMm 总直线长度
 * @param curveRadiusMm 弯道半径
 * @param outRenderPosition 输出参数，返回计算后的位置
 * @param outRenderRotationDeg 输出参数，返回计算后的角度
 */
void VehicleRenderer::calculatePosition(const gui::VehicleState &vehicle,
                                        float totalStraightLengthMm, // 改为总直线长度
                                        float curveRadiusMm,
                                        sf::Vector2f &outRenderPosition, // 改名以更清晰
                                        float &outRenderRotationDeg)     // 改名以更清晰
{
    // 使用车辆原始轨道位置
    float currentTrackPosMm = vehicle.rawTrackPositionMm;

    // 轨道几何
    float singleStraightLengthMm = totalStraightLengthMm / 2.0f; // 假设对称轨道，两段等长直线
    float curveLengthMm = M_PI * curveRadiusMm;
    float fullTrackPerimeterMm = totalStraightLengthMm + 2.0f * curveLengthMm;

    // 确保位置在轨道长度范围内 (0 to fullTrackPerimeterMm)
    currentTrackPosMm = fmod(currentTrackPosMm, fullTrackPerimeterMm);
    if (currentTrackPosMm < 0)
    {
        currentTrackPosMm += fullTrackPerimeterMm;
    }

    // 段1: 下方直道 (0 ~ singleStraightLengthMm)
    // 段2: 右侧弯道 (singleStraightLengthMm ~ singleStraightLengthMm + curveLengthMm)
    // 段3: 上方直道 (singleStraightLengthMm + curveLengthMm ~ 2*singleStraightLengthMm + curveLengthMm)
    // 段4: 左侧弯道 (2*singleStraightLengthMm + curveLengthMm ~ fullTrackPerimeterMm)

    float renderX = 0.0f, renderY = 0.0f;
    float angleDeg = 0.0f;

    float curveRadiusPx = curveRadiusMm * MM_TO_PIXEL;
    float singleStraightPx = singleStraightLengthMm * MM_TO_PIXEL;

    if (currentTrackPosMm < singleStraightLengthMm)
    {
        // 段1: 下方直道 (原点在左弯道中心，轨道从左下角开始向右)
        renderX = (currentTrackPosMm * MM_TO_PIXEL) - singleStraightPx / 2.0f; // 调整，使轨道中心在 (0,0) 附近
        renderY = curveRadiusPx;                                               // Y向下为正，下方直道在 +curveRadiusPx
        angleDeg = 0.0f;                                                       // 朝右
    }
    else if (currentTrackPosMm < singleStraightLengthMm + curveLengthMm)
    {
        // 段2: 右侧弯道
        float angleRad = (currentTrackPosMm - singleStraightLengthMm) / curveRadiusMm;
        // 右侧弯道中心: (singleStraightPx / 2.0f, 0)
        renderX = (singleStraightPx / 2.0f) + curveRadiusPx * sin(angleRad);
        renderY = curveRadiusPx * cos(angleRad); // Y从+curveRadiusPx变到-curveRadiusPx (cos从1到-1)
        angleDeg = angleRad * 180.0f / M_PI;
    }
    else if (currentTrackPosMm < 2 * singleStraightLengthMm + curveLengthMm)
    {
        // 段3: 上方直道
        float posOnSegment = currentTrackPosMm - (singleStraightLengthMm + curveLengthMm);
        renderX = (singleStraightPx / 2.0f) - (posOnSegment * MM_TO_PIXEL); // 从右向左
        renderY = -curveRadiusPx;
        angleDeg = 180.0f; // 朝左
    }
    else
    {
        // 段4: 左侧弯道
        float angleRad = (currentTrackPosMm - (2 * singleStraightLengthMm + curveLengthMm)) / curveRadiusMm;
        // 左侧弯道中心: (-singleStraightPx / 2.0f, 0)
        renderX = (-singleStraightPx / 2.0f) - curveRadiusPx * sin(angleRad);
        renderY = -curveRadiusPx * cos(angleRad); // Y从-curveRadiusPx变到+curveRadiusPx (sin从0到1再到0, cos从-1到1)
        angleDeg = 180.0f + angleRad * 180.0f / M_PI;
    }

    outRenderPosition.x = renderX;
    outRenderPosition.y = renderY;
    outRenderRotationDeg = angleDeg;
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
                                    const gui::VehicleState &vehicle,
                                    const sf::Vector2f &position,
                                    float rotation)
{
    // 根据车辆状态确定颜色
    sf::Color bodyColor;
    if (!vehicle.currentTaskId.empty())
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
        idText.setString(vehicle.id);
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
    if (!vehicle.currentTaskId.empty())
    {
        sf::Text taskText;
        taskText.setFont(m_font);
        taskText.setString("T" + std::to_string(std::stoi(vehicle.currentTaskId)));
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

void VehicleRenderer::updateVehicleStates(const std::vector<gui::VehicleState> &vehicles)
{
    m_vehicles = vehicles;
    // 如果需要计算世界坐标，可在渲染时调用calculatePosition
}

sf::Color VehicleRenderer::getColorForStatus(gui::VehicleStatus status) const
{
    switch (status)
    {
    case gui::VehicleStatus::IDLE:
        return m_statusColors.idle;
    case gui::VehicleStatus::MOVING_TO_LOAD:
        return m_statusColors.movingToLoad;
    case gui::VehicleStatus::LOADING:
        return m_statusColors.loading;
    case gui::VehicleStatus::MOVING_TO_UNLOAD:
        return m_statusColors.movingToUnload;
    case gui::VehicleStatus::UNLOADING:
        return m_statusColors.unloading;
    case gui::VehicleStatus::CHARGING:
        return m_statusColors.charging;
    case gui::VehicleStatus::ERROR:
        return m_statusColors.error;
    default:
        return m_statusColors.unknown;
    }
}

void VehicleRenderer::updateState(const gui::VehicleState &state, float totalStraightLengthMm, float curveRadiusMm)
{
    m_currentState = state; // Store the raw state

    // Calculate render position and rotation based on raw track position and track geometry
    sf::Vector2f renderPos;
    float renderRotDeg;
    calculatePosition(state, totalStraightLengthMm, curveRadiusMm, renderPos, renderRotDeg);

    // Update the VehicleRenderer's own transform (since it's a sf::Transformable)
    // This means the draw() call will draw the vehicle at this calculated position and rotation.
    this->setPosition(renderPos);
    this->setRotation(renderRotDeg);

    // Update visual properties of internal shapes based on the state
    // These internal shapes are drawn relative to the VehicleRenderer's transform.
    // Their own setPosition/setRotation should be (0,0) and 0 if they are meant to align with the VehicleRenderer's origin.

    m_statusBounds.setFillColor(getColorForStatus(state.status));
    // m_statusBounds' position and rotation are relative to the VehicleRenderer's origin (now 0,0)
    // m_statusBounds.setPosition(0,0);
    // m_statusBounds.setRotation(0);

    // Body color based on load
    if (state.isLoaded)
    {
        m_body.setFillColor(sf::Color(100, 180, 255)); // Light blue for loaded
    }
    else
    {
        m_body.setFillColor(sf::Color(0, 120, 255)); // Default blue
    }
    // m_body.setPosition(0,0); // Relative to VehicleRenderer's origin
    // m_body.setRotation(0);   // Relative to VehicleRenderer's origin

    m_idText.setString(state.id);
    sf::FloatRect textBounds = m_idText.getLocalBounds();
    m_idText.setOrigin(textBounds.left + textBounds.width / 2.0f,
                       textBounds.top + textBounds.height / 2.0f);
    // m_idText.setPosition(0,0); // Centered on VehicleRenderer's origin
    // m_idText.setRotation(0);   // No independent rotation for text relative to body

    // Direction indicator position needs to be calculated relative to the body,
    // which is now at (0,0) relative to the VehicleRenderer's transform.
    // The body's origin is its center.
    sf::Vector2f localDirPos(m_body.getSize().x / 2.f, 0.f); // Front-center of the body
    m_directionIndicator.setPosition(localDirPos);           // This is now in local coords of VehicleRenderer
    // m_directionIndicator.setRotation(0); // Indicator itself usually doesn't rotate, its position indicates direction
}

void VehicleRenderer::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    // Apply the VehicleRenderer's own transform (set by setPosition/setRotation in updateState)
    states.transform *= getTransform();

    // Draw components. They are positioned relative to the VehicleRenderer's origin.
    target.draw(m_statusBounds, states);
    target.draw(m_body, states);
    target.draw(m_directionIndicator, states);
    target.draw(m_idText, states);
}