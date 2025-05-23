#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>

/**
 * @brief 轨道渲染器类
 *
 * 负责根据物理参数绘制环形轨道系统，包括直线段和弯道段
 * 继承自sf::Drawable，可直接作为绘制对象传递给SFML渲染目标
 * 轨道采用单线表示法，显示轨道的内外边界
 */
class TrackRenderer : public sf::Drawable, public sf::Transformable
{
private:
    sf::VertexArray m_trackShape;
    sf::VertexArray m_centerLineVisual;

    sf::Color m_trackColor = sf::Color(120, 120, 120);
    sf::Color m_borderColor = sf::Color(80, 80, 80);
    sf::Color m_centerLineColor = sf::Color(200, 200, 200, 100);

    float m_mmToPxRatio = 0.02f; // 毫米到像素的转换比例 (1mm = 0.02px)
    float m_straightLengthMm = 40000.0f;
    float m_curveRadiusMm = 2500.0f;
    float m_trackWidthMm = 1200.0f; // Actual physical track width in mm

    struct TrackSegment
    {
        enum Type
        {
            STRAIGHT,
            CURVE
        } type;
        float startDistanceMm;
        float lengthMm;
        sf::Vector2f startCoordMm;
        float startAngleRad;
        sf::Vector2f curveCenterMm;
        float curveStartAngleRad;
        bool clockwise;
    };
    std::vector<TrackSegment> m_trackSegments;
    float m_totalTrackLengthMm = 0.0f;
    bool m_drawCenterLine = true;

public:
    /**
     * @brief 构造函数
     */
    TrackRenderer();

    // Generates or regenerates the track's mathematical and visual geometry
    // based on the provided physical dimensions in millimeters.
    void generateGeometry(float straightLengthMm, float curveRadiusMm);

    // Sets the track's physical width in millimeters.
    // This will trigger a regeneration of the visual geometry.
    void setTrackWidthMm(float widthMm);

    // Sets the conversion ratio from millimeters to pixels.
    // This will trigger a regeneration of the visual geometry.
    void setMmToPxRatio(float ratio);

    // Gets the current conversion ratio from millimeters to pixels.
    float getMmToPxRatio() const;

    // Sets the main color of the track surface.
    void setTrackColor(const sf::Color &color);

    // Sets the color of the track's border lines.
    void setBorderColor(const sf::Color &color);

    // Sets the color of the track's centerline.
    void setCenterLineColor(const sf::Color &color);

    // Enables or disables drawing of the track's centerline.
    void enableCenterLineDrawing(bool enable);

    // Calculates a point and orientation (tangent angle) on the track's centerline
    // for a given distance along the track from its origin.
    // worldOriginOffsetPx is added to the final pointPx.
    bool getPointAndOrientationOnTrack(float distanceMm, sf::Vector2f &pointPx, float &angleRadians, const sf::Vector2f &worldOriginOffsetPx) const;

    // Gets the total length of the track's centerline in millimeters.
    float getTotalTrackLengthMm() const;

    // Gets the length of one straight segment of the track in millimeters.
    float getStraightLengthMm() const;

    // Gets the radius of the curved segments of the track in millimeters.
    float getCurveRadiusMm() const;

    // Gets the physical width of the track in millimeters.
    float getTrackWidthMm() const;

    // Backend to Render transform (Obsolete, or needs re-evaluation)
    // sf::Vector2f backendToRenderTransform(const sf::Vector2f &backendPoint) const;
    // Render to Backend transform (Obsolete, or needs re-evaluation)
    // sf::Vector2f renderToBackendTransform(const sf::Vector2f &renderPoint) const;

protected:
    /**
     * @brief 重载的绘制方法
     * @param target 渲染目标
     * @param states 渲染状态
     */
    void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

    // Calculates the mathematical definition of track segments (centerline path).
    void calculateMathematicalSegments();
    // Updates the visual geometry (vertices for drawing) based on mathematical segments and visual properties.
    void updateVisualTrackGeometry();
};