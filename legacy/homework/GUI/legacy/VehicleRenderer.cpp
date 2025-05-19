#include "VehicleRenderer.hpp"
#include "../Core/Vehicle.hpp"
#include <iostream>
#include <cmath>

/**
 * @brief 绘制单个车辆
 *
 * 根据车辆状态、位置和朝向绘制车辆图形。图形包括:
 * - 矩形车身，颜色根据载货状态变化
 * - 三角形方向指示器显示车头朝向
 * - 车辆ID标签
 * - 速度指示条（随速度变化）
 *
 * @param target 渲染目标
 * @param vehicle 车辆数据引用
 * @param position 车辆世界坐标（像素）
 * @param rotation 车辆朝向角度（度）
 */
void VehicleRenderer::renderVehicle(sf::RenderTarget &target,
                                    const VehicleState &vehicle,
                                    const sf::Vector2f &position,
                                    float rotation)
{
    // 创建车辆主体形状 - 尺寸已通过 MM_TO_PIXEL 转换为像素单位
    // m_baseSize = {100px, 40px} 对应 {2000mm, 800mm}
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
    // 注意：在实际应用中，应将字体加载放到构造函数中而不是每次渲染时加载
    static sf::Font font;
    static bool fontLoaded = false;

    if (!fontLoaded)
    {
        try
        {
            // 尝试多个可能的路径
            if (font.loadFromFile("resources/fonts/arial.ttf"))
            {
                fontLoaded = true;
            }
            else if (font.loadFromFile("GUI/resources/fonts/arial.ttf"))
            {
                fontLoaded = true;
            }
            else if (font.loadFromFile("fonts/arial.ttf"))
            {
                fontLoaded = true;
            }
            else if (font.loadFromFile("C:/Windows/Fonts/arial.ttf"))
            {
                fontLoaded = true;
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << "无法加载字体: " << e.what() << std::endl;
        }
    }

    if (fontLoaded)
    {
        sf::Text idText;
        idText.setFont(font);
        idText.setString(std::to_string(vehicle.id));
        idText.setCharacterSize(12);
        idText.setFillColor(sf::Color::White);

        // 计算文本位置（使其跟随车辆旋转）
        sf::FloatRect textBounds = idText.getLocalBounds();
        idText.setOrigin(textBounds.width / 2, textBounds.height / 2);
        idText.setPosition(position);
        idText.setRotation(rotation);

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
}