#ifndef TRACK_RENDERER_HPP
#define TRACK_RENDERER_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>

/**
 * @class TrackRenderer
 * @brief 轨道渲染类，用于渲染轨道形状，包括内外轨
 */
class TrackRenderer : public sf::Drawable
{
public:
    /// 单位转换系数: 1mm = 0.05px
    static constexpr float MM_TO_PIXEL = 0.05f;

    /// 默认构造函数
    TrackRenderer();

    /**
     * @brief 生成轨道几何形状
     * @param trackLength 轨道总长度(mm)
     * @param curveRadius 弯道半径(mm)
     */
    void generateGeometry(float trackLength, float curveRadius);

    /**
     * @brief 设置轨道宽度
     * @param width 轨道宽度(mm)
     */
    void setTrackWidth(float width);

    /**
     * @brief 获取轨道实际长度(px)
     * @return 轨道实际长度(像素)
     */
    float getActualTrackLength() const;

    /**
     * @brief 设置中心轨道颜色
     * @param color 中心轨道颜色
     */
    void setCenterLineColor(const sf::Color &color);

    /**
     * @brief 设置直线部分颜色
     * @param color 直线颜色
     */
    void setStraightColor(const sf::Color &color);

    /**
     * @brief 设置弯道部分颜色
     * @param color 弯道颜色
     */
    void setCurveColor(const sf::Color &color);

protected:
    /**
     * @brief 添加直线段
     * @param x1 起点x坐标(px)
     * @param y1 起点y坐标(px)
     * @param x2 终点x坐标(px)
     * @param y2 终点y坐标(px)
     * @param color 线段颜色
     */
    void addLineSegment(float x1, float y1, float x2, float y2, const sf::Color &color);

    /**
     * @brief 添加弧线段
     * @param centerX 圆心x坐标(px)
     * @param centerY 圆心y坐标(px)
     * @param radius 半径(px)
     * @param startAngle 起始角度(弧度)
     * @param endAngle 结束角度(弧度)
     * @param segments 分段数
     * @param color 线段颜色
     */
    void addArcSegment(float centerX, float centerY, float radius,
                       float startAngle, float endAngle, int segments,
                       const sf::Color &color);

private:
    /// @brief 绘制函数，继承自sf::Drawable
    virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

    // 轨道几何数据
    float m_trackLength = 0.0f;    // 轨道总长(mm)
    float m_curveRadius = 0.0f;    // 弯道半径(mm)
    float m_trackWidth = 60.0f;    // 轨道宽度(像素)
    float m_straightLength = 0.0f; // 直道长度(mm)

    // 绘图数据
    sf::VertexArray m_centerLines;     // 中心线
    sf::VertexArray m_innerTrackLines; // 内侧轨道线
    sf::VertexArray m_outerTrackLines; // 外侧轨道线

    // 颜色设置
    sf::Color m_centerLineColor = sf::Color(180, 180, 180);
    sf::Color m_straightColor = sf::Color(160, 160, 160);
    sf::Color m_curveColor = sf::Color(160, 160, 160);
};

#endif // TRACK_RENDERER_HPP
