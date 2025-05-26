#include "../../include/gui/TrackRenderer.hpp"
#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

TrackRenderer::TrackRenderer()
    : m_innersTrack(sf::LineStrip), // 内轨道线条//枚举值表示一组连接的线段，其中每个点与前一个点连接形成一条线。它的值为 2，通常用于图形绘制中表示线条序列的绘制模式。
      m_outerTrack(sf::LineStrip)   // 外轨道线条
{
}

void TrackRenderer::generateGeometry(float trackLength, float curveRadius) // curveRadius is now inner radius
{
    // 保存轨道参数以便后续坐标转换
    m_trackLength = trackLength;
    m_curveRadius = curveRadius; // m_curveRadius now stores the INNER radius

    // 清除现有顶点
    m_innersTrack.clear();
    m_outerTrack.clear();
    m_centerPoints.clear();

    // 根据现在的实验显示，长度占了40*2=80的格子，圆的半径占了5个格子

    // 将真实尺寸（毫米）转换为像素
    float scaledTrackLength = trackLength * m_mmToPxRatio * m_scaleFactor;
    // 'curveRadius' (and thus m_curveRadius) is the inner radius.
    float scaledInnerCurveRadius = m_curveRadius * m_mmToPxRatio * m_scaleFactor;
    float scaledFullTrackWidth = m_trackWidth * m_mmToPxRatio * m_scaleFactor; // Full track width in pixels
    // Calculate the radius for the actual center line (path for vehicles)
    float scaledCenterLineCurveRadius = scaledInnerCurveRadius + (scaledFullTrackWidth / 2.0f);

    // 轨道宽度计算 - 转换为像素 (this variable is effectively scaledFullTrackWidth)
    // float trackOffset = m_trackWidth * m_mmToPxRatio * m_scaleFactor; // This is scaledFullTrackWidth
    // float innerDefaultOffset = 0.0f; // 内轨道偏移 (Unused by current inner/outer track generation)

    // ============= 生成中心线坐标点 =============
    // 赛道由四段组成：上直道、右弯道、下直道、左弯道
    int pointsPerCurve = 520; // 增加弯道分段数，使曲线更平滑

    // 生成弯道和直道点的函数
    auto generateCurve = [&](float cx, float cy, float r, float startAngle, float endAngle)
    {
        float angleStep = (endAngle - startAngle) / pointsPerCurve;
        for (int i = 0; i <= pointsPerCurve; ++i)
        {
            float angle = startAngle + i * angleStep;
            m_centerPoints.push_back(sf::Vector2f( // push_back 是一个内联成员函数，用于将一个右值引用类型的元素 (value_type &&__x) 添加到 std::vector 的末尾。
                cx + r * std::cos(angle),
                cy - r * std::sin(angle))); // Inverted Y-axis: use -r*sin(angle) for Y-up
        }
    }; // 各弯道中心
    float rightCurveX = scaledTrackLength / 2;
    float rightCurveY = 0.0f;
    float leftCurveX = -scaledTrackLength / 2;
    float leftCurveY = 0.0f; // 修改轨道生成顺序：从左下角开始（距离0对应左下角位置）
    // 1. 从左弯道270°位置开始，沿轨道中心线顺时针生成点
    // 先生成左下角点 (270°)
    float leftBottomX = leftCurveX + scaledCenterLineCurveRadius * std::cos(3 * M_PI / 2);
    float leftBottomY = leftCurveY - scaledCenterLineCurveRadius * std::sin(3 * M_PI / 2);
    m_centerPoints.push_back(sf::Vector2f(leftBottomX, leftBottomY));

    // 2. 生成下直道点
    sf::Vector2f rightBottom(rightCurveX, leftBottomY); // 右弯道底部点
    m_centerPoints.push_back(rightBottom);

    // 3. 生成右弯道点 (-90° ~ 90°)
    generateCurve(rightCurveX, rightCurveY, scaledCenterLineCurveRadius, -M_PI / 2, M_PI / 2);

    // 4. 生成上直道点
    sf::Vector2f rightTop = m_centerPoints.back(); // 右弯道最后一点
    sf::Vector2f leftTop(leftCurveX, rightTop.y);  // 左弯道顶部点
    m_centerPoints.push_back(leftTop);

    // 5. 生成左弯道点 (90° ~ 270°)
    generateCurve(leftCurveX, leftCurveY, scaledCenterLineCurveRadius, M_PI / 2, 3 * M_PI / 2);

    // 不再添加起始点重复点，防止线条连接处出现交叉

    // ============= 生成轨道内外轮廓 =============
    // 根据中心线生成平行偏移的轨道线
    auto generateTrack = [](const std::vector<sf::Vector2f> &centerLine, float offset,
                            sf::VertexArray &track, const sf::Color &color)
    {
        // 清空顶点数组
        track.clear();

        // 计算每个点的偏移位置
        std::vector<sf::Vector2f> offsetPoints;

        for (size_t i = 0; i < centerLine.size(); ++i)
        {
            // 获取当前点和相邻点
            const sf::Vector2f &current = centerLine[i];
            sf::Vector2f prev, next;

            if (i == 0)
            {
                prev = centerLine.back();
                next = centerLine[i + 1];
            }
            else if (i == centerLine.size() - 1)
            {
                prev = centerLine[i - 1];
                next = centerLine[0];
            }
            else
            {
                prev = centerLine[i - 1];
                next = centerLine[i + 1];
            }

            // 计算前后方向向量
            sf::Vector2f dir1(current.x - prev.x, current.y - prev.y);
            sf::Vector2f dir2(next.x - current.x, next.y - current.y);

            // 归一化
            float len1 = std::sqrt(dir1.x * dir1.x + dir1.y * dir1.y);
            float len2 = std::sqrt(dir2.x * dir2.x + dir2.y * dir2.y);

            if (len1 < 0.0001f || len2 < 0.0001f)
            {
                continue; // 防止除零错误
            }

            dir1.x /= len1;
            dir1.y /= len1;
            dir2.x /= len2;
            dir2.y /= len2;

            // 计算平均方向的法向量
            sf::Vector2f avgDir((dir1.x + dir2.x) / 2, (dir1.y + dir2.y) / 2);
            float avgLen = std::sqrt(avgDir.x * avgDir.x + avgDir.y * avgDir.y);

            if (avgLen < 0.0001f)
            {
                // 如果平均方向太小，使用单一方向
                sf::Vector2f normal(-dir1.y, dir1.x); // Standard normal for Y-down
                // For Y-up, normal would be (dir1.y, -dir1.x) or (-dir1.y, dir1.x) depending on desired offset direction.
                // Given SFML's Y-down for rendering, and our internal Y-up, the offset logic needs care.
                // If offset is positive for 'outer' and negative for 'inner' relative to clockwise path:
                // For Y-up internal: normal for outer offset (right side of path) is (dy, -dx)
                // For Y-up internal: normal for inner offset (left side of path) is (-dy, dx)
                // Current code uses (-dy, dx) for normal. If offset > 0 means left, offset < 0 means right.
                // Let's assume the existing normal calculation (-dir1.y, dir1.x) is for a Y-down system and produces an outward normal for a clockwise path.
                // If our internal coordinates are now Y-up, this normal calculation is still valid for finding a perpendicular vector.
                // The direction of offset (positive or negative) will determine if it's inner or outer.
                // No change needed here if the offset sign convention is maintained.
                offsetPoints.push_back(sf::Vector2f(
                    current.x + normal.x * offset,
                    current.y + normal.y * offset));
            }
            else
            {
                // 归一化平均方向
                avgDir.x /= avgLen;
                avgDir.y /= avgLen;

                // 计算法向量 (垂直于平均方向)
                sf::Vector2f normal(-avgDir.y, avgDir.x); // Same reasoning as above.

                // 计算偏移点
                offsetPoints.push_back(sf::Vector2f(
                    current.x + normal.x * offset,
                    current.y + normal.y * offset));
            }
        }

        // 添加所有偏移点到顶点数组
        for (const auto &point : offsetPoints)
        {
            track.append(sf::Vertex(point, color));
        }

        // 闭合曲线 - 添加第一个点
        if (!offsetPoints.empty())
        {
            track.append(sf::Vertex(offsetPoints[0], color));
        }
    };

    // Correctly define halfScaledTrackWidth using scaledFullTrackWidth before its use.
    float halfScaledTrackWidth = scaledFullTrackWidth / 2.0f;

    // 生成内轨道线
    generateTrack(m_centerPoints, halfScaledTrackWidth, m_innersTrack, m_trackColor); // Inner edge

    // 生成外轨道线 - 使用对称的负偏移量
    generateTrack(m_centerPoints, -halfScaledTrackWidth, m_outerTrack, m_trackColor); // Outer edge
}

float TrackRenderer::getTotalCenterLineLengthMm() const
{
    if (m_centerPoints.empty())
        return 0.0f;

    float totalPixelLength = 0.0f;
    for (size_t i = 0; i < m_centerPoints.size() - 1; ++i)
    {
        sf::Vector2f p1 = m_centerPoints[i];
        sf::Vector2f p2 = m_centerPoints[i + 1];
        totalPixelLength += std::sqrt(std::pow(p2.x - p1.x, 2) + std::pow(p2.y - p1.y, 2));
    }
    // Add length of closing segment (last point to first point)
    sf::Vector2f p_last = m_centerPoints.back();
    sf::Vector2f p_first = m_centerPoints.front();
    totalPixelLength += std::sqrt(std::pow(p_first.x - p_last.x, 2) + std::pow(p_first.y - p_last.y, 2));

    if (m_mmToPxRatio == 0.0f || m_scaleFactor == 0.0f)
        return 0.0f; // Avoid division by zero
    return totalPixelLength / (m_mmToPxRatio * m_scaleFactor);
}

bool TrackRenderer::getPointAndOrientationOnCenterLine(float distanceMm, sf::Vector2f &pointPx, float &angleRadians, const sf::Vector2f &worldOriginOffsetPx) const
{
    if (m_centerPoints.empty() || m_mmToPxRatio == 0.0f || m_scaleFactor == 0.0f)
    {
        pointPx = worldOriginOffsetPx;
        angleRadians = 0.0f;
        return false;
    }

    float targetPixelDistance = distanceMm * m_mmToPxRatio * m_scaleFactor;
    float currentPixelDistance = 0.0f;

    for (size_t i = 0; i < m_centerPoints.size(); ++i)
    {
        sf::Vector2f p1 = m_centerPoints[i];
        sf::Vector2f p2 = m_centerPoints[(i + 1) % m_centerPoints.size()]; // Loop back to the first point for the last segment

        sf::Vector2f segmentVec = p2 - p1;
        float segmentPixelLength = std::sqrt(segmentVec.x * segmentVec.x + segmentVec.y * segmentVec.y);

        if (segmentPixelLength < 0.0001f && i < m_centerPoints.size() - 1) // Avoid issues with zero-length segments unless it's the very last point connecting to first
        {
            if (i == m_centerPoints.size() - 1 && targetPixelDistance == 0.0f)
            { // Special case: distance is 0, use first point
                pointPx = m_centerPoints[0] + worldOriginOffsetPx;
                sf::Vector2f nextSegmentStart = m_centerPoints[0];
                sf::Vector2f nextSegmentEnd = m_centerPoints[1 % m_centerPoints.size()];
                angleRadians = std::atan2(nextSegmentEnd.y, nextSegmentEnd.x); // Standard atan2 for Y-up coordinates
                return true;
            }
            // For other zero-length segments, effectively skip them by not adding to currentPixelDistance
            // and let the loop continue to the next segment.
            // If it's the last segment and it's zero length, and we haven't found the point,
            // it means the distance is beyond the track length or exactly at the end.
            // The logic below will handle falling off the end of this segment.
        }

        if (targetPixelDistance >= currentPixelDistance && targetPixelDistance <= (currentPixelDistance + segmentPixelLength + 0.0001f)) // Add epsilon for float comparison
        {
            float ratio = 0.0f;
            if (segmentPixelLength > 0.0001f)
            { // Avoid division by zero for non-zero length segments
                ratio = (targetPixelDistance - currentPixelDistance) / segmentPixelLength;
            }
            else if (targetPixelDistance == currentPixelDistance)
            { // If segment is zero length and we are at its start
                ratio = 0.0f;
            }
            else
            { // Should not happen if targetPixelDistance is within currentPixelDistance and segmentPixelLength is zero
                // This case implies target is beyond a zero-length segment start.
                // Let it be handled by falling to the next segment or end of track.
                // For safety, treat as if at the end of this zero-length segment.
                ratio = 1.0f;
            }

            pointPx.x = p1.x + ratio * segmentVec.x;
            pointPx.y = p1.y + ratio * segmentVec.y; // Y is already Y-up from m_centerPoints
            pointPx += worldOriginOffsetPx;

            angleRadians = std::atan2(segmentVec.y, segmentVec.x); // Standard atan2 for Y-up coordinates
            return true;
        }
        currentPixelDistance += segmentPixelLength;
    }

    // If distanceMm is greater than total track length or some other issue,
    // snap to the last point (which is effectively the first point as it's a loop) or handle as an error.
    // For now, let's return the start point if loop completes (e.g. distance > total length)
    // This might indicate an issue with distanceMm or if the track is very small.
    // A more robust solution might involve clamping distanceMm to total length.
    // Or, if distance is very large, it could indicate an error.
    // For now, if loop completes, it means targetPixelDistance > total length of m_centerPoints segments.
    // We can default to the starting point or the last point.
    // Let's use the first point and its initial orientation.
    if (!m_centerPoints.empty())
    {
        pointPx = m_centerPoints[0] + worldOriginOffsetPx;
        if (m_centerPoints.size() > 1)
        {
            sf::Vector2f firstSegmentVec = m_centerPoints[1] - m_centerPoints[0];
            angleRadians = std::atan2(firstSegmentVec.y, firstSegmentVec.x); // Standard atan2 for Y-up coordinates
        }
        else
        {
            angleRadians = 0.0f; // Single point track
        }
        return true; // Or false, to indicate the distance was out of bounds. Let's return true but position at start.
    }

    pointPx = worldOriginOffsetPx;
    angleRadians = 0.0f;
    return false; // Should not be reached if m_centerPoints is not empty
}

// 坐标转换：后端坐标系(左下角弯道与直道交汇点为原点) -> 渲染坐标系(轨道中心为原点)
sf::Vector2f TrackRenderer::backendToRenderTransform(const sf::Vector2f &backendPoint) const
{
    // 将后端坐标（毫米）转换为像素，并应用缩放
    float scaledX = backendPoint.x * m_mmToPxRatio * m_scaleFactor;
    float scaledY = backendPoint.y * m_mmToPxRatio * m_scaleFactor;

    // 计算轨道参数的像素尺寸
    float scaledTrackLength = m_trackLength * m_mmToPxRatio * m_scaleFactor;
    // m_curveRadius is inner radius, m_trackWidth is full width
    float scaledInnerCurveRadius = m_curveRadius * m_mmToPxRatio * m_scaleFactor;
    float scaledFullTrackWidth = m_trackWidth * m_mmToPxRatio * m_scaleFactor;
    // Calculate the center-line radius in pixels, which defines the backend coordinate system's reference path
    float scaledCenterLineRadius_forTransform = scaledInnerCurveRadius + scaledFullTrackWidth / 2.0f;

    // The X offset needed to align backend origin (start of straight section on center line) with render origin (track center)
    float x_offset_to_render_origin = scaledTrackLength / 2.0f;
    // The Y offset needed (for Y-up backend coords) based on center-line radius
    float y_offset_to_render_origin_Yup = scaledCenterLineRadius_forTransform;

    // 转换坐标系
    return sf::Vector2f(
        scaledX - x_offset_to_render_origin,
        -(scaledY - y_offset_to_render_origin_Yup) // Invert Y for render (Y-down) and apply Y shift
    );
}

// 坐标转换：渲染坐标系(轨道中心为原点) -> 后端坐标系(左下角弯道与直道交汇点为原点)
// renderPoint is Y-down from SFML
sf::Vector2f TrackRenderer::renderToBackendTransform(const sf::Vector2f &renderPoint) const
{
    // 计算轨道参数的像素尺寸
    float scaledTrackLength = m_trackLength * m_mmToPxRatio * m_scaleFactor;
    // m_curveRadius is inner radius, m_trackWidth is full width
    float scaledInnerCurveRadius = m_curveRadius * m_mmToPxRatio * m_scaleFactor;
    float scaledFullTrackWidth = m_trackWidth * m_mmToPxRatio * m_scaleFactor;
    // Calculate the center-line radius in pixels
    float scaledCenterLineRadius_forTransform = scaledInnerCurveRadius + scaledFullTrackWidth / 2.0f;

    // The X offset from render origin (track center) to backend origin (start of straight section on center line)
    float x_offset_from_render_origin = scaledTrackLength / 2.0f;
    // The Y offset (for Y-up internal coords) based on center-line radius
    float y_offset_from_render_origin_Yup = scaledCenterLineRadius_forTransform;

    // 从渲染坐标系转换到后端坐标系（像素）
    // renderPoint.y is Y-down. To convert to internal Y-up for backend calculation:
    float internalRenderY_Yup = -renderPoint.y;

    float scaled_BackendX_relative_pixels = renderPoint.x + x_offset_from_render_origin;
    float scaled_BackendY_relative_pixels = internalRenderY_Yup + y_offset_from_render_origin_Yup;

    // 转换回毫米单位
    if (m_mmToPxRatio == 0.0f || m_scaleFactor == 0.0f)
    {
        // Avoid division by zero, return a sensible default or handle error
        return sf::Vector2f(0.0f, 0.0f);
    }

    return sf::Vector2f(
        scaled_BackendX_relative_pixels / (m_mmToPxRatio * m_scaleFactor),
        scaled_BackendY_relative_pixels / (m_mmToPxRatio * m_scaleFactor) // Resulting backend Y is Y-up
    );
}

// 可选的位置偏移绘制方法
void TrackRenderer::render(sf::RenderTarget &target, const sf::Vector2f &position)
{
    sf::RenderStates states;
    states.transform.translate(position);
    // sf::RenderStates 是一个来自 SFML（Simple and Fast Multimedia Library）的类，用于定义渲染时的状态组合，例如变换、纹理、着色器等。states 是该类的一个实例，通常用于在绘制图形时指定自定义的渲染设置。
    draw(target, states);
}

void TrackRenderer::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    // 先绘制内轨道，再绘制外轨道
    target.draw(m_innersTrack, states);
    target.draw(m_outerTrack, states);
}