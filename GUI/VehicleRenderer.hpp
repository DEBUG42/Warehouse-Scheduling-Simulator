#include <SFML/Graphics.hpp>
#include "Vehicle.hpp"
class VehicleRenderer {
private:
    // 车辆模型参数
    const sf::Vector2f m_baseSize {16.0f, 8.0f}; // 基础尺寸（像素）
    
    // 状态样式
    sf::Color m_colorEmpty {80, 130, 200};       // 空载状态
    sf::Color m_colorLoaded {200, 90, 40};       // 载货状态
    sf::Color m_colorAssigned {140, 80, 160};     // 已分配任务状态

public:
    /**
     * @brief 绘制单个车辆
     * @param target 渲染目标
     * @param vehicle 车辆数据引用
     * @param position 车辆世界坐标
     * @param rotation 车辆朝向角度
     */
    void renderVehicle(sf::RenderTarget& target,
                      const MotionState& vehicle,
                      const sf::Vector2f& position,
                      float rotation);
};