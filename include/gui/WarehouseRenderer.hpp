#ifndef WAREHOUSE_RENDERER_HPP
#define WAREHOUSE_RENDERER_HPP

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <map>
#include "SimObject.hpp"
#include "DeviceState.hpp"
#include "WarehouseUtils.hpp"
#include "WarehouseState.hpp"

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
        int id;                     ///< 接口ID
        InterfaceType type;         ///< 接口类型(入库/出库)
        WarehousePosition position; ///< 所处位置(上/下)
        float centerX;              ///< 中心点X坐标(mm)
        float centerY;              ///< 中心点Y坐标(mm)
        float width;                ///< 宽度(mm)
        float height;               ///< 高度(mm)
        float depth;                ///< 深度(mm)
        gui::DeviceState state;     ///< 设备状态
    };

public:
    /// 默认构造函数
    WarehouseRenderer();

    /**
     * @brief 初始化所有仓库接口设备及图标资源
     * @param trackRadius 轨道半径(mm)
     * @param iconBasePath 图标资源的基础路径
     */
    void initialize(float trackRadius, const std::string &iconBasePath = "resources/icons/");

    /**
     * @brief 更新设备状态
     * @param deviceStates 设备状态数组引用
     */
    void updateDeviceStates(const std::vector<gui::DeviceState> &deviceStates);

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

    /**
     * @brief 兼容旧接口：更新仓库状态（自动转换为设备状态）
     */
    void updateWarehouseStates(const std::vector<WarehouseState> &warehouseStates);

protected:
    /// 绘制接口设备
    void drawInterface(sf::RenderTarget &target, sf::RenderStates states,
                       const WarehouseInterface &interface) const;

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
    std::vector<WarehouseInterface> m_interfaces; ///< 所有接口设备
    sf::Font m_font;                              ///< 标签字体
    sf::Color m_inputColor{60, 180, 75};          ///< 入库口颜色
    sf::Color m_outputColor{230, 85, 40};         ///< 出库口颜色
    sf::Color m_borderColor{100, 100, 100};       ///< 边框颜色
    sf::Color m_shadowColor{50, 50, 50, 150};     ///< 阴影颜色

    // 状态颜色 (从DeviceRenderer借鉴并统一)
    const sf::Color COLOR_IDLE{75, 185, 85, 180};      // 空闲状态 (稍透明)
    const sf::Color COLOR_WORKING{60, 150, 230, 255};  // 工作中状态 (修改了颜色以区分原Input/Output)
    const sf::Color COLOR_FAULT{230, 170, 50, 255};    // 故障状态
    const sf::Color COLOR_OFFLINE{150, 150, 150, 200}; // 离线状态

    // 图标资源
    std::map<gui::DeviceType, sf::Texture> m_iconTextures;

    // 文本标签集合
    std::vector<sf::Text> m_labels; ///< 接口标签
};

#endif // WAREHOUSE_RENDERER_HPP
