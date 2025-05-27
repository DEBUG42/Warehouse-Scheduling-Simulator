#pragma once
#include <SFML/Graphics.hpp>
#include "SimObject.hpp"
#include <SFML/Graphics/Transformable.hpp>
#include "gui/TrackRenderer.hpp" // Added include for TrackRenderer

/**
 * @brief 车辆渲染器类
 *
 * 负责根据车辆状态信息在屏幕上绘制车辆图形
 * 包括车身、方向指示器、ID标签以及速度指示器
 * 根据车辆状态(空载/载货/分配任务)显示不同颜色
 * 支持伪3D效果，增强立体感
 */
class VehicleRenderer : public sf::Drawable, public sf::Transformable
{
private:
    // const sf::Vector2f m_baseSize{40.0f, 16.0f}; // REMOVED: Base size in pixels, will be dynamic
    // float m_mmToPxRatio = 0.05f;                 // REMOVED: Will use TrackRenderer's ratio
    const float m_height = 10.0f; // 车辆高度（像素单位） - This might need scaling too if it's a fixed pixel height not relative to zoom

    // 状态样式
    sf::Color m_colorEmpty{80, 130, 200};     // 空载状态
    sf::Color m_colorLoaded{200, 90, 40};     // 载货状态
    sf::Color m_colorAssigned{140, 80, 160};  // 已分配任务状态
    sf::Color m_shadowColor{50, 50, 50, 150}; // 阴影颜色

    // 字体
    const sf::Font &m_font; // 文本字体

    // 渲染参数 (Physical dimensions in mm)
    float m_vehicleLength = 2000.0f; // 车辆长度(mm)
    float m_vehicleWidth = 800.0f;   // 车辆宽度(mm)
    // float m_trackWidth = 1200.0f;    // REMOVED: Not directly used for vehicle's own rendering scale
    // float m_curveRadius = 2500.0f;   // REMOVED: Not directly used for vehicle's own rendering scale

    // 颜色设置
    sf::Color m_emptyColor{60, 120, 200};   // 空载颜色
    sf::Color m_loadedColor{230, 85, 40};   // 载货颜色
    sf::Color m_selectedColor{255, 255, 0}; // 选中颜色
    sf::Color m_borderColor{100, 100, 100}; // 边框颜色

    // 车辆状态
    std::vector<gui::VehicleState> m_vehicles; // 车辆状态列表

    mutable sf::RectangleShape m_body;
    mutable sf::RectangleShape m_statusBounds;    // 新增：用于显示状态的外部矩形
    mutable sf::CircleShape m_directionIndicator; // 或者其他表示方向的形状
    mutable sf::Text m_idText;

    gui::VehicleState m_currentState; // 存储当前状态用于绘制

    TrackRenderer *m_trackRendererRef; // Reference to TrackRenderer for scaling info
    float m_vehicleVisualScale;        // Additional visual scale for vehicles

    // 新增：获取不同状态对应的颜色
    sf::Color getColorForStatus(gui::VehicleStatus status) const;

public:
    /**
     * @brief 构造函数，加载资源
     */
    VehicleRenderer(const sf::Font &font, TrackRenderer &trackRenderer); // Added TrackRenderer reference

    /**
     * @brief 更新车辆位置，根据轨道路程计算实际坐标和朝向
     * @param vehicle 车辆状态
     * @param trackRenderer 轨道渲染器引用
     * @param worldOriginOffsetPx 世界坐标原点偏移量
     * @param position 输出参数，返回计算后的位置
     * @param rotation 输出参数，返回计算后的角度
     */
    void calculatePosition(const gui::VehicleState &vehicle,
                           TrackRenderer &trackRenderer, // Kept for now, might be replaceable by m_trackRendererRef
                           const sf::Vector2f &worldOriginOffsetPx,
                           sf::Vector2f &position,
                           float &rotation);

    /**
     * @brief 绘制单个车辆
     * @param target 渲染目标
     * @param vehicle 车辆数据引用
     * @param position 车辆世界坐标
     * @param rotation 车辆朝向角度
     */
    void renderVehicle(sf::RenderTarget &target,
                       const gui::VehicleState &vehicle,
                       const sf::Vector2f &position,
                       float rotation) const; // Added const

    /**
     * @brief 渲染车辆阴影以增强3D效果
     * @param target 渲染目标
     * @param position 车辆世界坐标
     * @param rotation 车辆朝向角度
     */
    void renderShadow(sf::RenderTarget &target,
                      const sf::Vector2f &position,
                      float rotation) const; // Added const

    /**
     * @brief 更新车辆状态
     * @param vehicles 车辆状态列表
     */
    void updateVehicleStates(const std::vector<gui::VehicleState> &vehicles, const sf::Vector2f &worldOriginOffsetPx);

    virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const override;
    // REMOVED: setMmToPxRatio and getMmToPxRatio
    // void setMmToPxRatio(float mmToPxRatio);
    // float getMmToPxRatio() const { return m_mmToPxRatio; }

    /**
     * @brief 更新车辆状态，同时传递 TrackRenderer 和 worldOriginOffsetPx
     * @param state 车辆状态
     * @param trackRenderer 轨道渲染器引用
     * @param worldOriginOffsetPx 世界坐标原点偏移量
     */
    void updateState(const gui::VehicleState &state, TrackRenderer &trackRenderer, const sf::Vector2f &worldOriginOffsetPx);

    /**
     * @brief 设置车辆视觉缩放比例
     * @param scale 缩放比例
     */
    void setVehicleVisualScale(float scale);
    /**
     * @brief 获取车辆视觉缩放比例
     * @return 缩放比例
     */
    float getVehicleVisualScale() const { return m_vehicleVisualScale; }

private:
    // float m_vehicleVisualScale; // Moved up to be with m_trackRendererRef
};