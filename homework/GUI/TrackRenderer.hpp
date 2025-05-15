#pragma once
#include <SFML/Graphics.hpp>

/**
 * @brief 轨道渲染器类
 *
 * 负责根据物理参数绘制环形轨道系统，包括直线段和弯道段
 * 继承自sf::Drawable，可直接作为绘制对象传递给SFML渲染目标
 * 轨道样式采用双线形式，表示轨道的宽度和边界
 */
class TrackRenderer : public sf::Drawable
{
private:
    // 轨道几何数据
    sf::VertexArray m_straightSegments;       // 直轨顶点数组
    sf::VertexArray m_curveSegments;          // 弯轨顶点数组    // 样式参数
    const float m_trackWidth = 24.0f;         // 轨道线宽（对应实际1200mm宽度）
    sf::Color m_straightColor{180, 180, 180}; // 直轨颜色
    sf::Color m_curveColor{160, 160, 160};    // 弯轨颜色

public:
    /**
     * @brief 构造函数
     */
    TrackRenderer();

    /**
     * @brief 根据物理参数生成轨道几何形状
     * @param trackLength 轨道总长
     * @param curveRadius 弯道半径
     */
    void generateGeometry(float trackLength, float curveRadius);

protected:
    /**
     * @brief 重载的绘制方法
     * @param target 渲染目标
     * @param states 渲染状态
     */
    void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

private:
    /**
     * @brief 添加直线段到顶点数组
     * @param x1 起点X坐标
     * @param y1 起点Y坐标
     * @param x2 终点X坐标
     * @param y2 终点Y坐标
     * @param color 线段颜色
     */
    void addLineSegment(float x1, float y1, float x2, float y2, const sf::Color &color); /**
                                                                                          * @brief 添加圆弧段到顶点数组
                                                                                          * @param centerX 圆心X坐标
                                                                                          * @param centerY 圆心Y坐标
                                                                                          * @param radius 圆弧半径
                                                                                          * @param startAngle 起始角度(弧度)
                                                                                          * @param endAngle 结束角度(弧度)
                                                                                          * @param segments 圆弧分段数
                                                                                          */
    void addArcSegment(float centerX, float centerY, float radius,
                       float startAngle, float endAngle, int segments);
};