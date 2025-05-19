#pragma once
#include <SFML/Graphics.hpp>
#include "SimObject.hpp"

/**
 * @brief 车辆渲染器类
 *
 * 负责根据车辆状态信息在屏幕上绘制车辆图形
 * 包括车身、方向指示器、ID标签以及速度指示器
 * 根据车辆状态(空载/载货/分配任务)显示不同颜色
 */
class VehicleRenderer
{
private:                                         // 车辆模型参数
    const sf::Vector2f m_baseSize{40.0f, 16.0f}; // 基础尺寸（2000mm车长、800mm车宽对应的像素）

    // 状态样式
    sf::Color m_colorEmpty{80, 130, 200};    // 空载状态
    sf::Color m_colorLoaded{200, 90, 40};    // 载货状态
    sf::Color m_colorAssigned{140, 80, 160}; // 已分配任务状态

public:
    /**
     * @brief 绘制单个车辆
     * @param target 渲染目标
     * @param vehicle 车辆数据引用
     * @param position 车辆世界坐标
     * @param rotation 车辆朝向角度
     */
    void renderVehicle(sf::RenderTarget &target,
                       const VehicleState &vehicle,
                       const sf::Vector2f &position,
                       float rotation);
};