#pragma once
#include <SFML/Graphics.hpp>
#include "SimObject.hpp"
#include <vector>

/**
 * @brief 车辆渲染器类
 *
 * 负责根据车辆状态信息在屏幕上绘制车辆图形
 * 包括车身、方向指示器、ID标签以及速度指示器
 * 根据车辆状态(空载/载货/分配任务)显示不同颜色
 * 同时提供3D视觉效果，使车辆看起来更立体
 */
class VehicleRenderer : public sf::Drawable
{
private:
    // 单位转换常量
    static constexpr float MM_TO_PIXEL = 0.05f; // 毫米到像素的转换因子 (1mm = 0.05px)

    // 车辆状态数据
    std::vector<VehicleState> m_vehicles;

    // 车辆模型参数
    const sf::Vector2f m_baseSize{
        2000.0f * MM_TO_PIXEL, // 车长：2000mm = 100像素
        800.0f * MM_TO_PIXEL   // 车宽：800mm = 40像素
    };

    // 状态样式
    sf::Color m_colorEmpty{80, 130, 200};    // 空载状态
    sf::Color m_colorLoaded{200, 90, 40};    // 载货状态
    sf::Color m_colorAssigned{140, 80, 160}; // 已分配任务状态
    
    // 字体
    sf::Font m_font;
    bool m_fontLoaded = false;

public:
    /**
     * @brief 构造函数
     */
    VehicleRenderer();
    
    /**
     * @brief 更新车辆状态数据
     * @param vehicles 车辆状态数组
     */
    void updateVehicles(const std::vector<VehicleState>& vehicles);
    
    /**
     * @brief 设置字体
     * @param font 字体对象引用
     */
    void setFont(const sf::Font& font);
    
    /**
     * @brief 绘制单个车辆
     * @param target 渲染目标
     * @param vehicle 车辆数据引用
     * @param position 车辆世界坐标（像素）
     * @param rotation 车辆朝向角度（度）
     */
    void renderVehicle(sf::RenderTarget &target,
                       const VehicleState &vehicle,
                       const sf::Vector2f &position,
                       float rotation) const;
    
    /**
     * @brief 计算世界坐标和旋转角度
     * @param trackPosition 轨道位置（毫米）
     * @param trackLength 轨道总长（毫米）
     * @param straightLength 直道长度（毫米）
     * @param curveRadius 弯道半径（毫米）
     * @param position 输出参数，世界坐标（像素）
     * @param rotation 输出参数，旋转角度（度）
     */
    void calculateWorldPositionAndRotation(
        float trackPosition, 
        float trackLength, 
        float straightLength, 
        float curveRadius,
        sf::Vector2f& position, 
        float& rotation) const;

    /**
     * @brief 毫米转换为像素
     * @param mm 毫米值
     * @return 对应的像素值
     */
    static float mmToPixel(float mm) { return mm * MM_TO_PIXEL; }

    /**
     * @brief 获取车辆渲染尺寸
     * @return 车辆尺寸（像素）
     */
    sf::Vector2f getVehicleSize() const { return m_baseSize; }
    
protected:
    /**
     * @brief 重载的绘制方法
     * @param target 渲染目标
     * @param states 渲染状态
     */
    void draw(sf::RenderTarget &target, sf::RenderStates states) const override;
    
private:
    /**
     * @brief 尝试加载字体
     * @return 是否成功加载
     */
    bool tryLoadFont();
};