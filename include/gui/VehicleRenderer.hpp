#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include "TrackRenderer.hpp"
#include "../src/Core/Vehicle.hpp"

// Bring Core types into the current namespace for easier use
// using Core::Vehicle;
// using Core::MotionState;
// using Core::VehicleState;

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
    sf::Color m_colorEmpty{80, 130, 200}; // 空载状态
    // sf::Color m_colorLoaded{200, 90, 40};     // 载货状态
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
    sf::Color m_emptyColor{60, 120, 200}; // 空载颜色
    // sf::Color m_loadedColor{230, 85, 40};   // 载货颜色
    sf::Color m_selectedColor{255, 255, 0}; // 选中颜色
    sf::Color m_borderColor{100, 100, 100}; // 边框颜色

    // 车辆状态颜色
    sf::Color m_colorIdleNoCargo{120, 180, 240};    // 无任务无货（浅蓝）
    sf::Color m_colorAssignedNoCargo{255, 200, 60}; // 有任务无货（橙黄）
    sf::Color m_colorLoaded{230, 85, 40};           // 有货（橙红）
    sf::Color m_colorError{220, 50, 50};            // 故障/异常（红色）
    sf::Color m_colorCharging{60, 220, 180};        // 充电中（青色）
    sf::Color m_colorSelected{255, 255, 0};         // 选中（黄色）
    sf::Color m_colorShadow{50, 50, 50, 150};       // 阴影

    // 车辆状态
    std::vector<Vehicle *> m_vehicles; // Store pointers to Vehicle objects

    mutable sf::RectangleShape m_body;
    mutable sf::RectangleShape m_statusBounds;    // 新增：用于显示状态的外部矩形
    mutable sf::CircleShape m_directionIndicator; // 或者其他表示方向的形状
    mutable sf::Text m_idText;
    sf::Texture m_vehicleTexture; // Added for texture-based rendering
    sf::Sprite m_vehicleSprite;   // Added for texture-based rendering
    bool m_initialized = false;   // Flag to check if texture is loaded
    float m_visualScale = 0.1f;   // Default visual scale for the sprite

    TrackRenderer *m_trackRendererRef;                                        // Reference to TrackRenderer for scaling info
    float m_vehicleVisualScale;                                               // Additional visual scale for vehicles    // 新增：获取不同状态对应的颜色
    sf::Color getColorForMotionState(Vehicle::MotionState motionState) const; // Use MotionState directly
    sf::Color getColorForVehicleState(const Vehicle *vehicle) const;

public:
    /**
     * @brief 构造函数，加载资源
     */
    VehicleRenderer(const sf::Font &font, TrackRenderer &trackRenderer);

    /**
     * @brief 初始化车辆纹理和视觉比例
     * @param texturePath 纹理文件路径
     * @param scale 视觉比例
     */
    void initializeTexture(const std::string &texturePath, float scale = 0.1f);

    /**
     * @brief 更新车辆位置，根据轨道路程计算实际坐标和朝向
     * @param vehicle 车辆状态
     * @param trackRenderer 轨道渲染器引用
     * @param worldOriginOffsetPx 世界坐标原点偏移量
     * @param position 输出参数，返回计算后的位置
     * @param rotation 输出参数，返回计算后的角度
     */
    void calculateScreenPositionAndRotation(const Vehicle &vehicle, // Use Vehicle directly
                                            TrackRenderer &trackRenderer,
                                            const sf::Vector2f &worldOriginOffsetPx,
                                            sf::Vector2f &screenPosition,
                                            float &screenRotationDegrees) const; // Made const

    /**
     * @brief 绘制单个车辆
     * @param target 渲染目标
     * @param vehicle 车辆数据引用
     * @param position 车辆世界坐标
     * @param rotation 车辆朝向角度
     */
    void renderSingleVehicle(sf::RenderTarget &target,
                             const Vehicle &vehicle, // Use Vehicle directly
                             const sf::Vector2f &screenPosition,
                             float screenRotationDegrees) const; // Added const

    /**
     * @brief 渲染车辆阴影以增强3D效果
     * @param target 渲染目标
     * @param position 车辆世界坐标
     * @param rotation 车辆朝向角度
     */
    void renderShadow(sf::RenderTarget &target,
                      const sf::Vector2f &screenPosition,
                      float screenRotationDegrees) const; // Added const

    /**
     * @brief 更新车辆状态
     * @param vehicles 车辆状态列表
     */
    void setVehiclesToRender(const std::vector<Vehicle *> &vehicles); // Use Vehicle*

    /**
     * @brief 设置车辆的额外视觉缩放比例
     * @param scale 缩放比例
     */
    void setVehicleVisualScale(float scale);

    // Helper rendering functions - ensure these use `const Vehicle&`
    void drawDirectionIndicator(sf::RenderTarget &target, const sf::Vector2f &position, float rotation, float size) const;
    void drawIdText(sf::RenderTarget &target, int id, const sf::Vector2f &position, float scaleFactor) const;
    void drawBatteryIndicator(sf::RenderTarget &target, const Vehicle &vehicle, const sf::Vector2f &basePosition, float vehicleHeight, float scaleFactor) const;
    void drawCargoIndicator(sf::RenderTarget &target, const Vehicle &vehicle, const sf::Vector2f &basePosition, float vehicleWidth, float vehicleHeight, float scaleFactor) const;
    void drawStatusBounds(sf::RenderTarget &target, const Vehicle &vehicle, const sf::Vector2f &basePosition, float length, float width, float rotation) const;
    void drawVehicleID(sf::RenderTarget &target, const Vehicle &vehicle, const sf::Vector2f &screenPos, float spriteHeight, float zoomLevel) const; // Added declaration

    /**
     * @brief 绘制所有车辆
     * @param target 渲染目标
     * @param states 渲染状态
     */
    void draw(sf::RenderTarget &target, sf::RenderStates states) const;

    // Texture and initialization (if using sprites)
    // sf::Texture m_vehicleTexture;
    // sf::Sprite m_vehicleSprite;
    // bool m_initialized = false;
    // float m_visualScale = 1.0f; // General visual scale for the sprite

    // void initializeTexture(const std::string& texturePath, float scale = 1.0f);
};