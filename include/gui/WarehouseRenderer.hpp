#ifndef WAREHOUSE_RENDERER_HPP
#define WAREHOUSE_RENDERER_HPP

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <map>
#include "WarehouseUtils.hpp"
#include "TrackRenderer.hpp"
#include "../src/Core/Device.hpp"  // For DeviceState, DeviceType
#include "../src/Core/Vehicle.hpp" // For Vehicle for path rendering (if needed later)

// Bring Core types into the current namespace for easier use

/**
 * @class WarehouseRenderer
 * @brief 仓库/接口设备渲染器，负责绘制仓库和接口设备
 */
class WarehouseRenderer : public sf::Drawable
{
public:
    /// 单位转换系数: 1mm = 0.02px (与TrackRenderer保持一致)
    static constexpr float MM_TO_PIXEL = 0.02f;

    /// 仓库接口类型枚举
    enum class InterfaceType
    {
        INPUT, ///< 入库口
        OUTPUT ///< 出库口
    };

    /// 仓库位置枚举
    enum class WarehousePosition
    {
        TOP,   ///< 上方自动化库仓储区
        BOTTOM ///< 下方出入库作业区
    };

    /// 仓库接口信息结构体
    struct WarehouseInterface
    {
        int id;                             ///< 接口ID
        InterfaceType type;                 ///< 接口类型(入库/出库) - GUI specific enum
        WarehousePosition positionCategory; ///< 所处位置分类(上/下) - GUI specific enum
        DeviceState coreState;              ///< Core device state
        DeviceType coreType;                ///< Core device type

        // New members for precise positioning and dimensions
        sf::Vector2f worldCenterPx; ///< Center of the device in world pixel coordinates
        float worldRotationDegrees; ///< Rotation of the device in world degrees
        sf::FloatRect boundsPx;     ///< Axis-aligned bounding box in world pixels for click detection
        float widthMm;              ///< Physical width of the device (e.g., along the track or perpendicular)
        float depthMm;              ///< Physical depth of the device (perpendicular to width)
    };

public:
    /// 默认构造函数
    WarehouseRenderer();

    /**
     * @brief 初始化所有仓库接口设备及图标资源
     * @param trackRenderer 轨道渲染器引用
     * @param worldOriginOffsetPx 世界坐标原点偏移量
     * @param iconBasePath 图标资源的基础路径
     */
    void initialize(TrackRenderer &trackRenderer, const sf::Vector2f &worldOriginOffsetPx, const std::string &iconBasePath = "resources/icons/"); /**
                                                                                                                                                   * @brief 更新设备状态
                                                                                                                                                   * @param coreDevices Vector of pointers to Core::DeviceBase objects
                                                                                                                                                   */
    void updateDeviceStates(const std::vector<DeviceBase *> &coreDevices);

    /**
     * @brief 更新设备状态（使用状态映射）
     * @param deviceStates Map of device ID to DeviceState
     */
    void updateDeviceStates(const std::map<int, DeviceState> &deviceStates);

    /**
     * @brief 设置入库口颜色
     * @param color 入库口颜色
     */
    void setInputColor(const sf::Color &color);

    /**
     * @brief 设置出库口颜色
     * @param color 出库口颜色
     */
    void setOutputColor(const sf::Color &color);

    /**
     * @brief 获取指定位置的仓库接口
     * @param position 屏幕坐标
     * @return 仓库接口指针，如果没有则返回nullptr
     */
    const WarehouseInterface *getInterfaceAt(const sf::Vector2f &position) const;

protected:
    /// 绘制接口设备
    void drawInterface(sf::RenderTarget &target, sf::RenderStates states,
                       const WarehouseInterface &interface_obj) const;

private:
    /// 从sf::Drawable继承的绘制函数
    virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

    /// 创建上方自动化库仓储区接口
    void createTopInterfaces(float trackRadius);

    /// 创建下方出入库作业区接口
    void createBottomInterfaces(float trackRadius);

    /// 创建接口设备文本标签
    void createLabel(sf::Text &text, const sf::Font &font, const std::string &content,
                     float x, float y, float scale = 1.0f);

    // 成员变量
    sf::Vector2f m_worldOriginOffsetPx; ///< World origin offset in pixels
    TrackRenderer *m_trackRendererRef = nullptr;
    sf::Font m_font; // Add member for font

    std::vector<WarehouseInterface> m_interfaces; ///< List of all warehouse interfaces
    std::vector<sf::Text> m_labels;               ///< Labels for the interfaces

    std::map<DeviceType, sf::Texture> m_iconTextures; // Use DeviceType directly as key

    // Define colors for device states
    const sf::Color COLOR_IDLE = sf::Color::Green;
    const sf::Color COLOR_BUSY = sf::Color::Yellow; // For is_transferring
    const sf::Color COLOR_HAS_GOODS = sf::Color::Blue;
    const sf::Color COLOR_RESERVED = sf::Color(255, 165, 0); // Orange for reserved

    // Predefined layout structure (can be moved to .cpp or a config file)
    struct PredefinedDeviceLayout
    {
        int id;
        float trackDistanceMm;
        InterfaceType type;
        WarehousePosition positionCategory;
        float visualWidthMm;
        float visualDepthMm;
        float offsetFromTrackEdgeMm;
        DeviceType coreDeviceType;
    };
    static const std::vector<PredefinedDeviceLayout> s_deviceLayouts;

    sf::Color m_inputColor = sf::Color(60, 180, 75, 180);  // 入库口默认颜色
    sf::Color m_outputColor = sf::Color(230, 85, 40, 180); // 出库口默认颜色
    sf::Color m_borderColor = sf::Color(100, 100, 100);    // 边框颜色
};

#endif // WAREHOUSE_RENDERER_HPP
