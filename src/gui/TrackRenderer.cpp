#include "gui/TrackRenderer.hpp"
#include <SFML/Graphics.hpp> // Redundant if already in hpp, but safe
#include <cmath>             // For M_PI, std::cos, std::sin, std::fmod
#include <iostream>          // For debug output
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

TrackRenderer::TrackRenderer()
// : m_innersTrack(sf::LineStrip), // OLD - REMOVE
//   m_outerTrack(sf::LineStrip)   // OLD - REMOVE
{
    // Initial setup: Recalculate based on default member values
    calculateMathematicalSegments();
    updateVisualTrackGeometry();
}

void TrackRenderer::setMmToPxRatio(float ratio)
{
    if (ratio > 0 && m_mmToPxRatio != ratio)
    {
        m_mmToPxRatio = ratio;
        updateVisualTrackGeometry(); // Visuals depend on this ratio
    }
}

void TrackRenderer::setTrackWidthMm(float widthMm)
{
    if (widthMm > 0 && m_trackWidthMm != widthMm)
    {
        m_trackWidthMm = widthMm;
        updateVisualTrackGeometry(); // Visuals (track edges) depend on this width
    }
}

void TrackRenderer::generateGeometry(float straightLengthMm, float curveRadiusMm)
{
    bool changed = false;
    if (straightLengthMm > 0 && m_straightLengthMm != straightLengthMm)
    {
        m_straightLengthMm = straightLengthMm;
        changed = true;
    }
    if (curveRadiusMm > 0 && m_curveRadiusMm != curveRadiusMm)
    {
        m_curveRadiusMm = curveRadiusMm;
        changed = true;
    }

    if (changed)
    {
        calculateMathematicalSegments();
        updateVisualTrackGeometry();
    }
}

void TrackRenderer::setTrackColor(const sf::Color &color)
{
    if (m_trackColor != color)
    {
        m_trackColor = color;
        updateVisualTrackGeometry(); // Update colors of the vertices
    }
}

void TrackRenderer::setBorderColor(const sf::Color &color)
{
    if (m_borderColor != color)
    {
        m_borderColor = color;
        updateVisualTrackGeometry();
    }
}

void TrackRenderer::setCenterLineColor(const sf::Color &color)
{
    if (m_centerLineColor != color)
    {
        m_centerLineColor = color;
        if (m_drawCenterLine)
        {
            updateVisualTrackGeometry();
        }
    }
}

void TrackRenderer::enableCenterLineDrawing(bool enable)
{
    if (m_drawCenterLine != enable)
    {
        m_drawCenterLine = enable;
        updateVisualTrackGeometry(); // Need to update visuals if this changes
    }
}

void TrackRenderer::calculateMathematicalSegments()
{
    m_trackSegments.clear();
    m_totalTrackLengthMm = 0.0f;

    if (m_straightLengthMm <= 0 || m_curveRadiusMm <= 0)
    {
        return; // Invalid parameters
    }

    float currentDistanceMm = 0.0f;
    // Doc Origin: Inner track, bottom-left, where curve meets straight.
    // Track path proceeds CCW. Start point (0,0) is this doc origin for the centerline.
    sf::Vector2f currentCoordMm(0.0f, 0.0f);
    float currentAngleRad = 0.0f; // Initial direction: along +X axis

    // Segment 1: Bottom straight track (length = m_straightLengthMm)
    m_trackSegments.push_back({
        TrackSegment::STRAIGHT,
        currentDistanceMm, m_straightLengthMm,
        currentCoordMm, currentAngleRad,
        sf::Vector2f(), 0.0f, false // Unused for straight
    });
    currentDistanceMm += m_straightLengthMm;
    currentCoordMm.x += m_straightLengthMm * std::cos(currentAngleRad); // Should be +m_straightLengthMm
    currentCoordMm.y += m_straightLengthMm * std::sin(currentAngleRad); // Should be 0

    // Segment 2: Right curve (CCW, 180 degrees / PI radians)
    // Center of this curve: (L, R) relative to doc origin (0,0)
    float curveLength = M_PI * m_curveRadiusMm;
    sf::Vector2f curveCenterRight(m_straightLengthMm, m_curveRadiusMm);
    m_trackSegments.push_back({
        TrackSegment::CURVE,
        currentDistanceMm, curveLength,
        currentCoordMm, currentAngleRad,
        curveCenterRight, 0.0f, false // Curve starts at angle 0 relative to its center's positive X-axis
    });
    currentDistanceMm += curveLength;
    // End of this curve: (L, 2*R). Angle is now PI (pointing left).
    currentCoordMm = sf::Vector2f(m_straightLengthMm, 2.0f * m_curveRadiusMm);
    currentAngleRad = M_PI;

    // Segment 3: Top straight track (length = m_straightLengthMm)
    // Starts at (L, 2*R), moves left by L.
    m_trackSegments.push_back({TrackSegment::STRAIGHT,
                               currentDistanceMm, m_straightLengthMm,
                               currentCoordMm, currentAngleRad,
                               sf::Vector2f(), 0.0f, false});
    currentDistanceMm += m_straightLengthMm;
    // End of this straight: (0, 2*R). Angle is still PI.
    currentCoordMm = sf::Vector2f(0.0f, 2.0f * m_curveRadiusMm);

    // Segment 4: Left curve (CCW, 180 degrees / PI radians)
    // Center of this curve: (0, R) relative to doc origin (0,0)
    // curveLength is the same
    sf::Vector2f curveCenterLeft(0.0f, m_curveRadiusMm);
    m_trackSegments.push_back({TrackSegment::CURVE,
                               currentDistanceMm, curveLength,
                               currentCoordMm, currentAngleRad, // Starts at angle PI relative to its center's positive X-axis
                               curveCenterLeft, M_PI, false});
    currentDistanceMm += curveLength;
    // End of this curve should be back at (0,0), angle 2*PI (or 0).
    // currentCoordMm = sf::Vector2f(0.0f, 0.0f); // Reset for clarity, though not strictly needed for next iter
    // currentAngleRad = 0.0f; // Or 2.0f * M_PI

    m_totalTrackLengthMm = currentDistanceMm;
}

void TrackRenderer::updateVisualTrackGeometry()
{
    m_trackShape.clear();
    m_centerLineVisual.clear();

    if (m_trackSegments.empty() || m_mmToPxRatio <= 0.0f)
    {
        return;
    }

    m_trackShape.setPrimitiveType(sf::TriangleStrip); // Or LineStrip for inner/outer separately then combine
                                                      // For a filled track, TriangleStrip is better.

    // Define number of points for curves to make them smooth
    const int curvePoints = 32; // Number of segments to approximate a 180-degree curve

    std::vector<sf::Vector2f> centerLinePointsPx;
    std::vector<sf::Vector2f> innerEdgePointsPx;
    std::vector<sf::Vector2f> outerEdgePointsPx;

    float halfTrackWidthPx = (m_trackWidthMm / 2.0f) * m_mmToPxRatio;

    for (const auto &segment : m_trackSegments)
    {
        if (segment.type == TrackSegment::STRAIGHT)
        {
            sf::Vector2f p1_center_mm = segment.startCoordMm;
            sf::Vector2f p2_center_mm = sf::Vector2f(
                segment.startCoordMm.x + segment.lengthMm * std::cos(segment.startAngleRad),
                segment.startCoordMm.y + segment.lengthMm * std::sin(segment.startAngleRad));
            sf::Vector2f p1_center_px = sf::Vector2f(p1_center_mm.x * m_mmToPxRatio, p1_center_mm.y * m_mmToPxRatio);
            sf::Vector2f p2_center_px = sf::Vector2f(p2_center_mm.x * m_mmToPxRatio, p2_center_mm.y * m_mmToPxRatio);

            centerLinePointsPx.push_back(p1_center_px);
            // Note: Don't add p2_center_px here if the next segment starts from it, to avoid duplicate points.
            // However, for distinct segments in loop, last point of straight is needed before curve starts unless curve calc reuses.

            // Normal vector (points "left" if looking along track dir, for CCW track this is outwards for bottom, inwards for top)
            sf::Vector2f normal(std::sin(segment.startAngleRad), -std::cos(segment.startAngleRad)); // perpendicular to tangent

            innerEdgePointsPx.push_back(p1_center_px - normal * halfTrackWidthPx);
            outerEdgePointsPx.push_back(p1_center_px + normal * halfTrackWidthPx);
            innerEdgePointsPx.push_back(p2_center_px - normal * halfTrackWidthPx);
            outerEdgePointsPx.push_back(p2_center_px + normal * halfTrackWidthPx);

            if (segment.startDistanceMm + segment.lengthMm >= m_totalTrackLengthMm - 0.1f)
            {                                               // If it's the last point of track
                centerLinePointsPx.push_back(p2_center_px); // Add final point for centerline
            }
        }
        else
        { // CURVE
            for (int i = 0; i <= curvePoints; ++i)
            {
                float fraction = static_cast<float>(i) / curvePoints;
                float angleOnCurve = segment.curveStartAngleRad + (segment.clockwise ? -1 : 1) * fraction * M_PI;

                sf::Vector2f center_pt_mm = sf::Vector2f(
                    segment.curveCenterMm.x + m_curveRadiusMm * std::cos(angleOnCurve),
                    segment.curveCenterMm.y + m_curveRadiusMm * std::sin(angleOnCurve));
                sf::Vector2f center_pt_px = sf::Vector2f(center_pt_mm.x * m_mmToPxRatio, center_pt_mm.y * m_mmToPxRatio);

                if (i == 0 && !centerLinePointsPx.empty() &&
                    std::abs(centerLinePointsPx.back().x - center_pt_px.x) < 0.01f &&
                    std::abs(centerLinePointsPx.back().y - center_pt_px.y) < 0.01f)
                {
                    // Skip if this point is identical to the last point (e.g. end of straight = start of curve)
                }
                else
                {
                    centerLinePointsPx.push_back(center_pt_px);
                }

                // Normal vector from curve center to point on curve (points outwards)
                sf::Vector2f normal = center_pt_mm - segment.curveCenterMm;
                float len = std::sqrt(normal.x * normal.x + normal.y * normal.y);
                if (len > 0)
                    normal /= len;

                innerEdgePointsPx.push_back(center_pt_px - normal * halfTrackWidthPx);
                outerEdgePointsPx.push_back(center_pt_px + normal * halfTrackWidthPx);
            }
        }
    }

    // Ensure the loop is closed for edge points if it isn't perfectly by calculation
    if (!innerEdgePointsPx.empty() && !outerEdgePointsPx.empty())
    {
        if (std::hypot(innerEdgePointsPx.front().x - innerEdgePointsPx.back().x, innerEdgePointsPx.front().y - innerEdgePointsPx.back().y) > 0.1f * m_mmToPxRatio)
        {
            innerEdgePointsPx.push_back(innerEdgePointsPx.front());
            outerEdgePointsPx.push_back(outerEdgePointsPx.front());
        }
        if (m_drawCenterLine && !centerLinePointsPx.empty() &&
            std::hypot(centerLinePointsPx.front().x - centerLinePointsPx.back().x, centerLinePointsPx.front().y - centerLinePointsPx.back().y) > 0.1f * m_mmToPxRatio)
        {
            centerLinePointsPx.push_back(centerLinePointsPx.front());
        }
    }

    // Build m_trackShape using a triangle strip from inner and outer edge points
    // The order is important: OuterP1, InnerP1, OuterP2, InnerP2, ...
    for (size_t i = 0; i < outerEdgePointsPx.size(); ++i)
    {
        m_trackShape.append(sf::Vertex(outerEdgePointsPx[i], m_trackColor, sf::Vector2f())); // UVs not used
        m_trackShape.append(sf::Vertex(innerEdgePointsPx[i], m_trackColor, sf::Vector2f()));
    }

    // Build m_centerLineVisual if enabled
    if (m_drawCenterLine)
    {
        m_centerLineVisual.setPrimitiveType(sf::LineStrip);
        for (const auto &pt : centerLinePointsPx)
        {
            m_centerLineVisual.append(sf::Vertex(pt, m_centerLineColor));
        }
    }
    // TODO: Add border drawing if desired (e.g., two more LineStrips for inner/outer edges with m_borderColor)
}

bool TrackRenderer::getPointAndOrientationOnTrack(float distanceMm, sf::Vector2f &pointPx, float &angleRadians, const sf::Vector2f &worldOriginOffsetPx) const
{
    if (m_trackSegments.empty() || m_totalTrackLengthMm <= 0)
        return false;

    // Normalize distance to be within one lap of the track
    distanceMm = std::fmod(distanceMm, m_totalTrackLengthMm);
    if (distanceMm < 0)
        distanceMm += m_totalTrackLengthMm;

    for (const auto &segment : m_trackSegments)
    {
        if (distanceMm >= segment.startDistanceMm && distanceMm <= segment.startDistanceMm + segment.lengthMm + 1e-3f /*epsilon for float inaccuracies at segment ends*/)
        {
            float distIntoSegment = distanceMm - segment.startDistanceMm;
            sf::Vector2f localPointMm; // Point relative to track's own (0,0) doc origin

            if (segment.type == TrackSegment::STRAIGHT)
            {
                localPointMm.x = segment.startCoordMm.x + distIntoSegment * std::cos(segment.startAngleRad);
                localPointMm.y = segment.startCoordMm.y + distIntoSegment * std::sin(segment.startAngleRad);
                angleRadians = segment.startAngleRad;
            }
            else
            { // CURVE
                // distIntoSegment is arc length. Angle turned along the curve = arc length / radius.
                float angleTurnedAlongCurve = distIntoSegment / m_curveRadiusMm;

                // Calculate the actual angle of the point on the curve relative to the curve's center's coordinate system.
                // segment.curveStartAngleRad is the angle from the curve's center to the start of this curve segment.
                float currentPointAbsoluteAngle;
                if (segment.clockwise)
                {
                    currentPointAbsoluteAngle = segment.curveStartAngleRad - angleTurnedAlongCurve;
                }
                else
                {
                    currentPointAbsoluteAngle = segment.curveStartAngleRad + angleTurnedAlongCurve;
                }

                localPointMm.x = segment.curveCenterMm.x + m_curveRadiusMm * std::cos(currentPointAbsoluteAngle);
                localPointMm.y = segment.curveCenterMm.y + m_curveRadiusMm * std::sin(currentPointAbsoluteAngle);

                // The tangent of the track path at this point.
                // segment.startAngleRad is the tangent of the track WHEN IT ENTERS this curve segment.
                // angleTurnedAlongCurve is how much the tangent has rotated since entering the curve.
                if (segment.clockwise)
                {
                    angleRadians = segment.startAngleRad - angleTurnedAlongCurve;
                }
                else
                {
                    angleRadians = segment.startAngleRad + angleTurnedAlongCurve;
                }
            }

            pointPx.x = localPointMm.x * m_mmToPxRatio + worldOriginOffsetPx.x;
            pointPx.y = localPointMm.y * m_mmToPxRatio + worldOriginOffsetPx.y;

            // Normalize angleRadians to be within [0, 2*PI)
            angleRadians = std::fmod(angleRadians, 2.0 * M_PI);
            if (angleRadians < 0)
                angleRadians += 2.0 * M_PI;

            return true;
        }
    }
    // Fallback for the very last point if fmod made distanceMm 0 and it should be totalLength
    if (distanceMm < 1e-3f && !m_trackSegments.empty())
    { // Effectively 0 or very close, check last segment end point
        const auto &lastSeg = m_trackSegments.back();
        if (lastSeg.startDistanceMm + lastSeg.lengthMm >= m_totalTrackLengthMm - 1e-3f)
        {
            // Use the end of the last segment
            // This logic is similar to above, for distIntoSegment = lastSeg.lengthMm
            // ... (implementation for last point needed here if required, or ensure fmod handles endpoints cleanly)
            // For simplicity, if it's 0 after fmod, it implies start of first segment.
        }
    }

    std::cerr << "TrackRenderer: Could not find point for distance " << distanceMm << " / " << m_totalTrackLengthMm << std::endl;
    return false; // Should ideally not be reached if distanceMm is correctly normalized and segments cover track
}

float TrackRenderer::getTotalTrackLengthMm() const
{
    return m_totalTrackLengthMm;
}

void TrackRenderer::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    // Apply the transform of the TrackRenderer itself (e.g. if it's moved/rotated in the scene)
    states.transform *= getTransform();

    // Draw the main track shape (filled area)
    target.draw(m_trackShape, states);

    // Draw the centerline if enabled
    if (m_drawCenterLine)
    {
        target.draw(m_centerLineVisual, states);
    }

    // Optional: Draw borders if not part of m_trackShape
    // If m_trackShape is a TriangleStrip, borders are implicitly the edges.
    // If you want explicit border lines with m_borderColor, you'd draw LineStrips for inner/outer edges here.
}

float TrackRenderer::getMmToPxRatio() const
{
    return m_mmToPxRatio;
}

float TrackRenderer::getStraightLengthMm() const
{
    return m_straightLengthMm;
}

float TrackRenderer::getCurveRadiusMm() const
{
    return m_curveRadiusMm;
}

float TrackRenderer::getTrackWidthMm() const
{
    return m_trackWidthMm;
}