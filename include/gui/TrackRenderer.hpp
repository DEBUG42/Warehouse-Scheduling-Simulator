#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath> // Added for M_PI if not defined elsewhere, and for std::cos, std::sin

/**
 * @brief 轨道渲染器类
 *
 * 负责根据物理参数绘制环形轨道系统，包括直线段和弯道段
 * 继承自sf::Drawable，可直接作为绘制对象传递给SFML渲染目标
 * 轨道采用单线表示法，显示轨道的内外边界
 */
class TrackRenderer : public sf::Drawable, public sf::Transformable // Added sf::Transformable back
{
private:
    // 轨道几何数据 - Aligned with TrackRenderer.cpp
    sf::VertexArray m_innersTrack;            // 内轨道线条
    sf::VertexArray m_outerTrack;             // 外轨道线条
    std::vector<sf::Vector2f> m_centerPoints; // 中心线点

    // 样式参数
    // float m_trackWidth = 30.0f;                  // Replaced by m_trackWidth (default 1200.0f in .hpp, set via setTrackWidth)
    sf::Color m_trackColor{0, 0, 0};      // Renamed from m_straightColor, used for both inner and outer tracks
    sf::Color m_testColor{255, 255, 255}; // Test color for debugging, can be removed later
    // sf::Color m_curveColor{120, 120, 120};       // Not used in current cpp
    // sf::Color m_centerLineColor{200, 200, 200};  // Not used in current cpp for drawing track lines
    // sf::Color m_outerBorderColor{100, 100, 100}; // Not used in current cpp

    // 轨道几何参数 - Aligned with TrackRenderer.cpp usage
    float m_trackLength = 40000.0f; // 轨道直线段长度 (set in generateGeometry)
    float m_curveRadius = 2500.0f;  // 弯道半径 (set in generateGeometry)
    float m_trackWidth = 1200.0f;   // Actual physical track width in mm, used in cpp, with a default

    // 缩放参数
    float m_scaleFactor = 1.0f;  // Default scale factor, was 0.7f, now 1.0f to match cpp logic better initially
    float m_mmToPxRatio = 0.02f; // 毫米到像素的转换比例 (1mm = 0.02px)

public:
    /**
     * @brief 构造函数
     */
    TrackRenderer();

    /**
     * @brief 根据物理参数生成轨道几何形状
     * @param trackLength 轨道直线段长度（毫米）
     * @param curveRadius 弯道半径（毫米）
     */
    void generateGeometry(float trackLength, float curveRadius);

    // 设置轨道宽度 (mm)
    void setTrackWidth(float width) { m_trackWidth = width; /* Potentially needs regeneration */ }
    // 获取轨道宽度 (mm)
    float getTrackWidth() const { return m_trackWidth; }

    // 设置缩放因子
    void setScaleFactor(float scale) { m_scaleFactor = scale; /* Potentially needs regeneration */ }
    // 获取缩放因子
    float getScaleFactor() const { return m_scaleFactor; }

    // 设置毫米到像素的转换比例
    void setMmToPxRatio(float ratio) { m_mmToPxRatio = ratio; /* Potentially needs regeneration */ }
    // 获取毫米到像素的转换比例
    float getMmToPxRatio() const { return m_mmToPxRatio; }

    // 新增：获取当前缩放级别
    float getCurrentZoomLevel() const { return m_scaleFactor; }

    // 新增：获取世界原点偏移量 (相对于TrackRenderer自身的原点)
    // 如果TrackRenderer被放置在场景中的特定位置，这将是该位置。
    // VehicleRenderer似乎管理自己的全局世界原点，所以这里返回自身位置。
    sf::Vector2f getWorldOriginOffset() const { return getPosition(); }

    // 设置轨道颜色
    void setTrackColor(const sf::Color &color) { m_trackColor = color; }
    // 获取轨道颜色
    const sf::Color &getTrackColor() const { return m_trackColor; }

    // 获取轨道中心线点 (for GUITest)
    const std::vector<sf::Vector2f> &getCenterPoints() const { return m_centerPoints; }

    // 获取轨道的中心线总长度（毫米）
    float getTotalCenterLineLengthMm() const; // 新增方法，用于计算总长度

    // 根据距离计算轨道中心线上的一个点及其方向（切线角度）
    // distanceMm：距离（毫米）
    // pointPx：计算得到的点（像素）
    // angleRadians：计算得到的角度（弧度）
    // worldOriginOffsetPx：最终点的偏移量（像素）
    bool getPointAndOrientationOnCenterLine(float distanceMm, sf::Vector2f &pointPx, float &angleRadians, const sf::Vector2f &worldOriginOffsetPx) const;

    // 获取轨道直线段长度 (mm)
    float getTrackLength() const { return m_trackLength; }
    // 获取弯道半径 (mm)
    float getCurveRadius() const { return m_curveRadius; }

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
    virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

private:
    // Commented out unused private helper methods from the previous .hpp version
    /*
    void addCurveSegment(float centerX, float centerY, float radius, float halfWidth,
                         float startAngle, float endAngle, int segments);
    void addCurveOutline(float centerX, float centerY, float radius,
                         float startAngle, float endAngle, int segments);
    void addCurveBorder(float centerX, float centerY, float radius,
                        float startAngle, float endAngle, int segments);
    */
};