#pragma once
#include <SFML/Graphics.hpp>

/**
 * @brief 修复后的轨道渲染器类
 *
 * 负责根据物理参数绘制环形轨道系统，包括直线段和弯道段
 * 继承自sf::Drawable，可直接作为绘制对象传递给SFML渲染目标
 * 轨道样式采用双线形式，表示轨道的宽度和边界
 * 修复了轨道几何形状和渲染问题，确保正确显示轨道
 */
class TrackRenderer_fixed : public sf::Drawable
{
private:
    // 单位转换常量
    static constexpr float MM_TO_PIXEL = 0.05f; // 毫米到像素的转换因子 (1mm = 0.05px)

    // 轨道几何数据
    sf::VertexArray m_straightSegments; // 直轨顶点数组
    sf::VertexArray m_curveSegments;    // 弯轨顶点数组
    sf::VertexArray m_trackMarkings;    // 轨道标记顶点数组

    // 样式参数
    float m_trackWidth = 60.0f;               // 轨道线宽（对应实际1200mm宽度，即1200 * MM_TO_PIXEL = 60px）
    sf::Color m_straightColor{180, 180, 180}; // 直轨颜色
    sf::Color m_curveColor{160, 160, 160};    // 弯轨颜色
    sf::Color m_markingColor{240, 240, 240};  // 标记颜色

    // 实际物理尺寸缓存（毫米）
    float m_trackLength = 126000.0f;   // 轨道总长度（毫米）
    float m_straightLength = 40000.0f; // 直道长度（毫米）
    float m_curveRadius = 2500.0f;     // 弯道半径（毫米）

public:
    /**
     * @brief 构造函数
     */
    TrackRenderer_fixed();

    /**
     * @brief 根据物理参数生成轨道几何形状
     * @param trackLength 轨道总长（毫米）
     * @param curveRadius 弯道半径（毫米）
     */
    void generateGeometry(float trackLength, float curveRadius);

    /**
     * @brief 获取实际轨道总长度
     * @return 轨道总长度（毫米）
     */
    float getActualTrackLength() const { return m_trackLength; }

    /**
     * @brief 获取直道长度
     * @return 每段直道长度（毫米）
     */
    float getStraightLength() const { return m_straightLength; }

    /**
     * @brief 获取弯道半径
     * @return 弯道半径（毫米）
     */
    float getCurveRadius() const { return m_curveRadius; }

    /**
     * @brief 毫米转换为像素
     * @param mm 毫米值
     * @return 对应的像素值
     */
    static float mmToPixel(float mm) { return mm * MM_TO_PIXEL; }

    /**
     * @brief 设置轨道宽度
     * @param width 轨道宽度（毫米）
     */
    void setTrackWidth(float width)
    {
        m_trackWidth = width * MM_TO_PIXEL; // 转换为像素单位
    }

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
     * @param x1 起点X坐标（像素）
     * @param y1 起点Y坐标（像素）
     * @param x2 终点X坐标（像素）
     * @param y2 终点Y坐标（像素）
     * @param color 线段颜色
     */
    void addLineSegment(float x1, float y1, float x2, float y2, const sf::Color &color);

    /**
     * @brief 添加圆弧段到顶点数组
     * @param centerX 圆心X坐标（像素）
     * @param centerY 圆心Y坐标（像素）
     * @param radius 圆弧半径（像素）
     * @param startAngle 起始角度(弧度)
     * @param endAngle 结束角度(弧度)
     * @param segments 圆弧分段数
     * @param color 圆弧颜色
     */
    void addArcSegment(float centerX, float centerY, float radius,
                       float startAngle, float endAngle, int segments,
                       const sf::Color &color = sf::Color::White);

    /**
     * @brief 添加轨道标记（距离刻度）
     */
    void addTrackMarkings();
};
