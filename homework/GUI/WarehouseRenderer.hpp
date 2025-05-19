#pragma once
#include <SFML/Graphics.hpp>
#include "SimObject.hpp"

/**
 * @brief 仓库/接口设备渲染器类
 *
 * 负责根据仓库/接口设备状态信息在屏幕上绘制仓库/接口设备图形
 * 包括建筑图形、状态指示器以及ID标签
 * 根据设备类型(仓库/接口设备)和负载状态显示不同样式
 */
class WarehouseRenderer : public sf::Drawable
{
private:
    // 单位转换常量
    static constexpr float MM_TO_PIXEL = 0.05f; // 毫米到像素的转换因子 (1mm = 0.05px)

    // 仓库/接口设备状态数据
    std::vector<WarehouseState> m_warehouses;

    // 样式参数
    sf::Color m_warehouseColor{100, 150, 100}; // 仓库基础颜色
    sf::Color m_interfaceColor{150, 100, 150}; // 接口设备基础颜色

    // 尺寸参数
    float m_warehouseWidth = 2000.0f * MM_TO_PIXEL;  // 仓库宽度（像素）
    float m_warehouseHeight = 3000.0f * MM_TO_PIXEL; // 仓库高度（像素）
    float m_interfaceWidth = 1500.0f * MM_TO_PIXEL;  // 接口设备宽度（像素）
    float m_interfaceHeight = 1500.0f * MM_TO_PIXEL; // 接口设备高度（像素）
    float m_trackOffset = 2000.0f * MM_TO_PIXEL;     // 距轨道中心的距离（像素）

    // 字体
    sf::Font m_font;
    bool m_fontLoaded = false;

public:
    /**
     * @brief 构造函数
     */
    WarehouseRenderer();

    /**
     * @brief 更新仓库/接口设备状态数据
     * @param warehouses 仓库/接口设备状态数组
     */
    void updateWarehouses(const std::vector<WarehouseState> &warehouses);

    /**
     * @brief 设置字体
     * @param font 字体对象引用
     */
    void setFont(const sf::Font &font);

    /**
     * @brief 获取设备渲染尺寸
     * @param isInterface 是否为接口设备
     * @return 设备尺寸（像素）
     */
    sf::Vector2f getSize(bool isInterface) const;

    /**
     * @brief 计算世界坐标
     * @param trackPosition 轨道位置（毫米）
     * @param trackLength 轨道总长（毫米）
     * @param straightLength 直道长度（毫米）
     * @param curveRadius 弯道半径（毫米）
     * @return 世界坐标（像素）
     */
    sf::Vector2f calculateWorldPosition(
        float trackPosition,
        float trackLength,
        float straightLength,
        float curveRadius) const;

    /**
     * @brief 毫米转换为像素
     * @param mm 毫米值
     * @return 对应的像素值
     */
    static float mmToPixel(float mm) { return mm * MM_TO_PIXEL; }

protected:
    /**
     * @brief 重载的绘制方法
     * @param target 渲染目标
     * @param states 渲染状态
     */
    void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

private:
    /**
     * @brief 绘制单个仓库/接口设备
     * @param target 渲染目标
     * @param warehouse 仓库/接口设备状态数据
     * @param position 世界坐标（像素）
     */
    void renderWarehouse(sf::RenderTarget &target, const WarehouseState &warehouse, const sf::Vector2f &position) const;

    /**
     * @brief 尝试加载字体
     * @return 是否成功加载
     */
    bool tryLoadFont();
};
