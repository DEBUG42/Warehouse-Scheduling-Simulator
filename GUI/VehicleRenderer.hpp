#pragma once
#include <SFML/Graphics.hpp>
#include "SimObject.hpp"

/**
 * @brief 车辆渲染器类
 *
 * 负责根据车辆状态信息在屏幕上绘制车辆图形
 * 包括车身、方向指示器、ID标签以及速度指示器
 * 根据车辆状态(空载/载货/分配任务)显示不同颜色
 * 支持伪3D效果，增强立体感
 */
class VehicleRenderer
{
private:
    const sf::Vector2f m_baseSize{40.0f, 16.0f}; // 基础尺寸（2000mm车长、800mm车宽对应的像素）
    const float m_height = 10.0f;                // 车辆高度（像素单位）
    const float MM_TO_PIXEL = 0.05f;             // 单位转换系数: 1mm = 0.05px (与TrackRenderer保持一致)

    // 状态样式
    sf::Color m_colorEmpty{80, 130, 200};     // 空载状态
    sf::Color m_colorLoaded{200, 90, 40};     // 载货状态
    sf::Color m_colorAssigned{140, 80, 160};  // 已分配任务状态
    sf::Color m_shadowColor{50, 50, 50, 150}; // 阴影颜色

    // 字体
    sf::Font m_font; // 文本字体

public:
    /**
     * @brief 构造函数，加载资源
     */
    VehicleRenderer();

    /**
     * @brief 更新车辆位置，根据轨道路程计算实际坐标和朝向
     * @param vehicle 车辆状态
     * @param trackLength 轨道总长度(mm)
     * @param curveRadius 弯道半径(mm)
     * @param position 输出参数，返回计算后的位置
     * @param rotation 输出参数，返回计算后的角度
     */
    void calculatePosition(const VehicleState &vehicle,
                           float trackLength,
                           float curveRadius,
                           sf::Vector2f &position,
                           float &rotation);    /**
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
                       
    /**
     * @brief 渲染车辆阴影以增强3D效果
     * @param target 渲染目标
     * @param position 车辆世界坐标
     * @param rotation 车辆朝向角度
     */
    void renderShadow(sf::RenderTarget &target,
                      const sf::Vector2f &position,
                      float rotation);
};