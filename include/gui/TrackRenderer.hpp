#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

/**
 * @brief 轨道渲染器类
 *
 * 负责根据物理参数绘制环形轨道系统，包括直线段和弯道段
 * 继承自sf::Drawable，可直接作为绘制对象传递给SFML渲染目标
 * 轨道采用填充形式，表示轨道的宽度和边界
 */
class TrackRenderer : public sf::Drawable
{
private:
    // 轨道几何数据
    sf::VertexArray m_straightSegments;    // 直轨顶点数组
    sf::VertexArray m_curveSegments;       // 弯轨顶点数组
    sf::VertexArray m_outerBorderSegments; // 外边框顶点数组
    sf::VertexArray m_innerTrackSegments;  // 内轨道顶点数组

    // 样式参数
    float m_trackWidth = 30.0f;                  // 轨道宽度（对应实际1200mm宽度）
    sf::Color m_straightColor{120, 120, 120};    // 直轨颜色
    sf::Color m_curveColor{120, 120, 120};       // 弯轨颜色
    sf::Color m_centerLineColor{200, 200, 200};  // 中心线颜色
    sf::Color m_outerBorderColor{100, 100, 100}; // 外边框颜色（较深）

    // 轨道几何参数
    float m_trackLength = 0.0f; // 轨道直线段长度
    float m_curveRadius = 0.0f; // 弯道半径

    // 缩放参数
    float m_scaleFactor = 0.7f;  // 缩放因子，用于调整轨道在视图中的整体大小
    float m_mmToPxRatio = 0.02f; // 毫米到像素的转换比例 (1mm = 0.02px)

public:
    /**
     * @brief 构造函数
     */
    TrackRenderer();

    /**
     * @brief 根据物理参数生成轨道几何形状
     * @param trackLength 轨道总长（毫米）
     * @param curveRadius 弯道半径（毫米）
     */
    void generateGeometry(float trackLength, float curveRadius);

    // 设置轨道宽度
    void setTrackWidth(float width) { m_trackWidth = width; }

    // 设置缩放因子
    void setScaleFactor(float scale) { m_scaleFactor = scale; }

    // 设置毫米到像素的转换比例
    void setMmToPxRatio(float ratio) { m_mmToPxRatio = ratio; }

    // 获取缩放因子
    float getScaleFactor() const { return m_scaleFactor; }

    // 获取毫米到像素的转换比例
    float getMmToPxRatio() const { return m_mmToPxRatio; }

    // 设置轨道颜色
    void setStraightColor(const sf::Color &color) { m_straightColor = color; }
    void setCurveColor(const sf::Color &color) { m_curveColor = color; }
    void setCenterLineColor(const sf::Color &color) { m_centerLineColor = color; }
    void setOuterBorderColor(const sf::Color &color) { m_outerBorderColor = color; }

    // 渲染轨道（可选位置偏移）
    void render(sf::RenderTarget &target, const sf::Vector2f &position);

    /**
     * @brief 将后端坐标系（左下角弯道与直道交汇点为原点）转换为渲染坐标系（轨道中心为原点）
     * @param backendPoint 后端坐标系中的点
     * @return 渲染坐标系中的点
     */
    sf::Vector2f backendToRenderTransform(const sf::Vector2f &backendPoint) const;

    /**
     * @brief 将渲染坐标系（轨道中心为原点）转换为后端坐标系（左下角弯道与直道交汇点为原点）
     * @param renderPoint 渲染坐标系中的点
     * @return 后端坐标系中的点
     */
    sf::Vector2f renderToBackendTransform(const sf::Vector2f &renderPoint) const;

protected:
    /**
     * @brief 重载的绘制方法
     * @param target 渲染目标
     * @param states 渲染状态
     */
    void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

private:
    /**
     * @brief 添加具有宽度的曲线段（使用三角形组成）
     * @param centerX 圆心X坐标
     * @param centerY 圆心Y坐标
     * @param radius 圆弧半径
     * @param halfWidth 轨道半宽
     * @param startAngle 起始角度(弧度)
     * @param endAngle 结束角度(弧度)
     * @param segments 圆弧分段数
     */
    void addCurveSegment(float centerX, float centerY, float radius, float halfWidth,
                         float startAngle, float endAngle, int segments);

    /**
     * @brief 使用线条绘制曲线轮廓
     * @param centerX 圆心X坐标
     * @param centerY 圆心Y坐标
     * @param radius 圆弧半径
     * @param startAngle 起始角度(弧度)
     * @param endAngle 结束角度(弧度)
     * @param segments 圆弧分段数
     */
    void addCurveOutline(float centerX, float centerY, float radius,
                         float startAngle, float endAngle, int segments);

    /**
     * @brief 绘制多层线条来创建粗边框效果
     * @param centerX 圆心X坐标
     * @param centerY 圆心Y坐标
     * @param radius 圆弧半径
     * @param startAngle 起始角度(弧度)
     * @param endAngle 结束角度(弧度)
     * @param segments 圆弧分段数
     */
    void addCurveBorder(float centerX, float centerY, float radius,
                        float startAngle, float endAngle, int segments);
};