#include "gui/VehicleRenderer.hpp"
#include "Core/Vehicle.hpp"
#include <iostream>
#include <cmath>
#include <SFML/Graphics/Transformable.hpp>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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
    // }    // 车体尺寸 (转换为像素)
    float bodyWidthPx = BODY_WIDTH_MM * m_mmToPxRatio;   // 使用成员 m_mmToPxRatio
    float bodyHeightPx = BODY_HEIGHT_MM * m_mmToPxRatio; // 使用成员 m_mmToPxRatio
    m_body.setSize(sf::Vector2f(bodyWidthPx, bodyHeightPx));
    m_body.setOrigin(bodyWidthPx / 2.f, bodyHeightPx / 2.f);                        // 中心原点
    m_body.setFillColor(sf::Color(0, 120, 255));                                    // 默认蓝色车体
    m_body.setOutlineThickness(1.f);                                                // 给车体一个细边框
    m_body.setOutlineColor(sf::Color::Black);                                       // 状态框尺寸 (转换为像素)
    float statusWidthPx = (BODY_WIDTH_MM + 2 * STATUS_BORDER_MM) * m_mmToPxRatio;   // 使用成员 m_mmToPxRatio
    float statusHeightPx = (BODY_HEIGHT_MM + 2 * STATUS_BORDER_MM) * m_mmToPxRatio; // 使用成员 m_mmToPxRatio
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
                                        TrackRenderer &trackRenderer,            // 新增参数
                                        const sf::Vector2f &worldOriginOffsetPx, // 新增参数
                                        sf::Vector2f &outRenderPosition,
                                        float &outRenderRotationDeg)
{
    // 使用车辆原始轨道位置
    float currentTrackPosMm = vehicle.rawTrackPositionMm;

    // 从 TrackRenderer 获取点和方向
    // Corrected: vehicle.id to vehicle.getId()
    if (!trackRenderer.getPointAndOrientationOnCenterLine(currentTrackPosMm, outRenderPosition, outRenderRotationDeg, worldOriginOffsetPx))
    {
        // 处理获取失败的情况，例如设置默认位置或打印错误
        std::cerr << "Error: Failed to get point and orientation from TrackRenderer for vehicle " << vehicle.getId() << std::endl;
        outRenderPosition = sf::Vector2f(0, 0); // 默认位置
        outRenderRotationDeg = 0;               // 默认角度
        return;
    }

    // getPointAndOrientationOnCenterLine 返回的是弧度，需要转换为角度
    outRenderRotationDeg = outRenderRotationDeg * 180.f / M_PI;

    // 注意：原始的 calculatePosition 方法中的复杂轨道几何计算现在由 TrackRenderer::getPointAndOrientationOnCenterLine 处理
    // 因此，下方所有关于轨道分段、弯道计算的逻辑都可以移除或注释掉

    /*
    // 轨道几何 (这部分现在由 Track Renderer 处理)
    // float singleStraightLengthMm = totalStraightLengthMm / 2.0f;
    // float curveLengthMm = M_PI * curveRadiusMm;
    // float fullTrackPerimeterMm = totalStraightLengthMm + 2.0f * curveLengthMm;

    // // 确保位置在轨道长度范围内 (0 to fullTrackPerimeterMm)
    // currentTrackPosMm = fmod(currentTrackPosMm, fullTrackPerimeterMm);
    // if (currentTrackPosMm < 0)
    // {
    //     currentTrackPosMm += fullTrackPerimeterMm;
    // }

    // // 段1: 下方直道 (0 ~ singleStraightLengthMm)
    // // 段2: 右侧弯道 (singleStraightLengthMm ~ singleStraightLengthMm + curveLengthMm)
    // // 段3: 上方直道 (singleStraightLengthMm + curveLengthMm ~ 2*singleStraightLengthMm + curveLengthMm)
    // // 段4: 左侧弯道 (2*singleStraightLengthMm + curveLengthMm ~ fullTrackPerimeterMm)

    // float renderX = 0.0f, renderY = 0.0f;
    // float angleDeg = 0.0f;

    // float curveRadiusPx = curveRadiusMm * MM_TO_PIXEL;
    // float singleStraightPx = singleStraightLengthMm * MM_TO_PIXEL;

    // if (currentTrackPosMm < singleStraightLengthMm) // 段1: 下方直道
    // {
    //     renderX = currentTrackPosMm * MM_TO_PIXEL;
    //     renderY = curveRadiusPx; // Y坐标在下方直道时固定为弯道半径处
    //     angleDeg = 0.0f;
    // }
    // else if (currentTrackPosMm < singleStraightLengthMm + curveLengthMm) // 段2: 右侧弯道
    // {
    //     float angleRad = (currentTrackPosMm - singleStraightLengthMm) / curveRadiusMm - (M_PI / 2.0f);
    //     renderX = singleStraightPx + curveRadiusPx * cos(angleRad);
    //     renderY = curveRadiusPx + curveRadiusPx * sin(angleRad);
    //     angleDeg = (angleRad + M_PI / 2.0f) * 180.0f / M_PI;
    // }
    // else if (currentTrackPosMm < 2 * singleStraightLengthMm + curveLengthMm) // 段3: 上方直道
    // {
    //     renderX = singleStraightPx - (currentTrackPosMm - singleStraightLengthMm - curveLengthMm) * MM_TO_PIXEL;
    //     renderY = -curveRadiusPx; // Y坐标在上方直道时固定为负弯道半径处
    //     angleDeg = 180.0f;
    // }
    // else // 段4: 左侧弯道
    // {
    //     float angleRad = (currentTrackPosMm - 2 * singleStraightLengthMm - curveLengthMm) / curveRadiusMm + (M_PI / 2.0f);
    //     renderX = curveRadiusPx * cos(angleRad);
    //     renderY = -curveRadiusPx + curveRadiusPx * sin(angleRad);
    //     angleDeg = (angleRad - M_PI / 2.0f) * 180.0f / M_PI + 180.0f; // 确保角度连续
    // }

    // // 将原点从左弯道中心移到世界坐标系原点 (通常是左上角，但这里我们假设与TrackRenderer一致，原点在 (curveRadiusPx, curveRadiusPx))
    // // TrackRenderer 的原点是 (0,0) 在渲染窗口的中心，但其内部几何计算可能基于不同的参考点。
    // // 这里的转换需要与 TrackRenderer::draw 和 SimulationView::drawGridAndAxes 的坐标系对齐。
    // // 假设 TrackRenderer 的 (0,0) 对应弯道中心，并且它在绘制时已经应用了到屏幕中心的变换。
    // // 车辆位置也需要应用相同的变换。

    // // 之前这里的变换是相对于 (curveRadiusPx, curveRadiusPx) 的，现在由 TrackRenderer 处理，所以直接使用其返回的坐标
    // outRenderPosition = sf::Vector2f(renderX, renderY);
    // outRenderRotationDeg = angleDeg;
    */
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
        idText.setString(vehicle.getId());
        idText.setCharacterSize(12);
        idText.setFillColor(sf::Color::White);
        idText.setOutlineThickness(1.0f);
        idText.setOutlineColor(sf::Color::Black);
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
        return sf::Color(200, 200, 200);
    case gui::VehicleStatus::MOVING_TO_LOAD:
        return sf::Color(100, 200, 100);
    case gui::VehicleStatus::LOADING:
        return sf::Color(100, 100, 200);
    case gui::VehicleStatus::MOVING_TO_UNLOAD:
        return sf::Color(200, 200, 100);
    case gui::VehicleStatus::UNLOADING:
        return sf::Color(200, 100, 100);
    case gui::VehicleStatus::CHARGING:
        return sf::Color(100, 200, 200);
    case gui::VehicleStatus::ERROR:
        return sf::Color(255, 0, 0);
    default:
        return sf::Color(128, 128, 128);
    }
}

void VehicleRenderer::updateState(const gui::VehicleState &state, TrackRenderer &trackRenderer, const sf::Vector2f &worldOriginOffsetPx)
{
    m_currentState = state;
    sf::Vector2f renderPos;
    float renderRotDeg;
    // Updated call to calculatePosition
    calculatePosition(state, trackRenderer, worldOriginOffsetPx, renderPos, renderRotDeg);
    this->setPosition(renderPos);
    this->setRotation(renderRotDeg);
    m_statusBounds.setFillColor(getColorForStatus(state.status));
    if (state.isLoaded)
    {
        m_body.setFillColor(sf::Color(100, 180, 255));
    }
    else
    {
        m_body.setFillColor(sf::Color(0, 120, 255));
    }
    m_idText.setString(state.getId());
    sf::FloatRect textBounds = m_idText.getLocalBounds();
    m_idText.setOrigin(textBounds.left + textBounds.width / 2.0f,
                       textBounds.top + textBounds.height / 2.0f);
    sf::Vector2f localDirPos(m_body.getSize().x / 2.f, 0.f);
    m_directionIndicator.setPosition(localDirPos);
}

void VehicleRenderer::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    states.transform *= getTransform();
    target.draw(m_statusBounds, states);
    target.draw(m_body, states);
    target.draw(m_directionIndicator, states);
    target.draw(m_idText, states);
}

void VehicleRenderer::setMmToPxRatio(float mmToPxRatio)
{
    m_mmToPxRatio = mmToPxRatio;

    // 重新计算车体尺寸
    float bodyWidthPx = BODY_WIDTH_MM * m_mmToPxRatio;
    float bodyHeightPx = BODY_HEIGHT_MM * m_mmToPxRatio;
    m_body.setSize(sf::Vector2f(bodyWidthPx, bodyHeightPx));
    m_body.setOrigin(bodyWidthPx / 2.f, bodyHeightPx / 2.f);

    // 重新计算状态框尺寸
    float statusWidthPx = (BODY_WIDTH_MM + 2 * STATUS_BORDER_MM) * m_mmToPxRatio;
    float statusHeightPx = (BODY_HEIGHT_MM + 2 * STATUS_BORDER_MM) * m_mmToPxRatio;
    m_statusBounds.setSize(sf::Vector2f(statusWidthPx, statusHeightPx));
    m_statusBounds.setOrigin(statusWidthPx / 2.f, statusHeightPx / 2.f);

    // 重新计算方向指示器
    m_directionIndicator.setRadius(std::max(1.f, bodyHeightPx * 0.15f));
    m_directionIndicator.setOrigin(m_directionIndicator.getRadius(), m_directionIndicator.getRadius());

    // 重新计算文字大小
    m_idText.setCharacterSize(static_cast<unsigned int>(std::max(8.f, bodyHeightPx * 0.3f)));
}