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

void TrackRenderer::generateGeometry(float trackLength, float curveRadius)
{
    // 保存轨道参数以便后续坐标转换
    m_trackLength = trackLength;
    m_curveRadius = curveRadius;

    // 清除现有顶点
    m_innersTrack.clear();
    m_outerTrack.clear();
    m_centerPoints.clear(); //    std::vector<sf::Vector2f> m_centerPoints; // 中心线点

    // 将真实尺寸（毫米）转换为像素
    float scaledTrackLength = trackLength * m_mmToPxRatio * m_scaleFactor;
    float scaledCurveRadius = curveRadius * m_mmToPxRatio * m_scaleFactor;

    // 轨道宽度计算 - 转换为像素
    float trackOffset = m_trackWidth * m_mmToPxRatio * m_scaleFactor;
    float innerDefaultOffset = 0.0f; // 内轨道偏移

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
    };

    // 各弯道中心
    float rightCurveX = scaledTrackLength / 2;
    float rightCurveY = 0.0f;
    float leftCurveX = -scaledTrackLength / 2;
    float leftCurveY = 0.0f;

    // 1. 从右上开始，沿轨道中心线顺时针生成点
    // 生成右弯道点 (-90° ~ 90°)
    generateCurve(rightCurveX, rightCurveY, scaledCurveRadius, -M_PI / 2, M_PI / 2);

    // 2. 生成下直道点
    sf::Vector2f rightBottom = m_centerPoints.back();   // 右弯道最后一点
    sf::Vector2f leftBottom(leftCurveX, rightBottom.y); // 左弯道底部点
    m_centerPoints.push_back(leftBottom);

    // 3. 生成左弯道点 (90° ~ 270°)
    generateCurve(leftCurveX, leftCurveY, scaledCurveRadius, M_PI / 2, 3 * M_PI / 2);

    // 4. 生成上直道点
    sf::Vector2f leftTop = m_centerPoints.back();  // 左弯道最后一点
    sf::Vector2f rightTop(rightCurveX, leftTop.y); // 右弯道顶部点
    m_centerPoints.push_back(rightTop);

    // 不再添加起始点重复点，防止线条连接处出现交叉

    // ============= 生成轨道内外轮廓 =============
    // 根据中心线生成平行偏移的轨道线

    // sf::VertexArray 是一个表示二维图形顶点数组的类，用于存储和管理一组顶点及其相关的图元类型。它支持顶点的添加、访问、清除、调整大小，以及计算边界矩形，并可用于绘制点、线、三角形等基本图元。

    // 用于根据给定的中心线（centerLine）和偏移量（offset），生成轨道的顶点数组（sf::VertexArray），并为其指定颜色（sf::Color）。该函数主要用于处理二维图形的顶点数据，适合绘制轨道或路径相关的图形元素
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

    // 生成内轨道线
    generateTrack(m_centerPoints, innerDefaultOffset, m_innersTrack, m_trackColor); // Positive offset for inner (left side of Y-up path)

    // 生成外轨道线 - 使用对称的负偏移量
    generateTrack(m_centerPoints, trackOffset, m_outerTrack, m_trackColor); // Negative offset for outer (right side of Y-up path) -> actually this should be positive for outer if normal points left. Let's verify offset meaning.
    // If trackOffset is half of m_trackWidth (positive value):
    // Inner track should be to the left (e.g., +trackOffset/2 with normal pointing left)
    // Outer track should be to the right (e.g., -trackOffset/2 with normal pointing left)
    // The original code had: generateTrack(m_centerPoints, innerDefaultOffset, m_innersTrack, m_trackColor);
    // generateTrack(m_centerPoints, -trackOffset, m_outerTrack, m_trackColor);
    // Assuming innerDefaultOffset is 0 or small positive for inner edge, and -trackOffset for outer edge.
    // If normal (-dy, dx) points left of the segment direction for Y-up:
    // To get inner track (left): offset should be positive.
    // To get outer track (right): offset should be negative.
    // So, the original -trackOffset for m_outerTrack seems correct if trackOffset is positive width.
    // And innerDefaultOffset for m_innersTrack (if it's meant to be the center or slightly to one side).

    // Let's clarify the trackOffset meaning. m_trackWidth is total width.
    // trackOffset in generateGeometry was m_trackWidth * ... / 2.0f (this was removed, now it's full m_trackWidth * ...)
    // No, it's still: float trackOffset = m_trackWidth * m_mmToPxRatio * m_scaleFactor; (this is the full width in px)
    // And then: float innerDefaultOffset = 0.0f;
    // generateTrack(m_centerPoints, innerDefaultOffset, m_innersTrack, m_trackColor);
    // generateTrack(m_centerPoints, -trackOffset, m_outerTrack, m_trackColor);
    // This means m_innersTrack is at the center line (offset 0), and m_outerTrack is offset by -totalWidth.
    // This is not right for inner/outer edges. It should be +/- halfWidth.

    // Re-evaluating based on the user's previous selection: `float trackOffset = m_trackWidth * m_mmToPxRatio * m_scaleFactor / 2.0f;`
    // If that was the case, then `innerDefaultOffset` (if 0) is the center, and `trackOffset` is half width.
    // Then `generateTrack(m_centerPoints, trackOffset, m_innersTrack, m_trackColor);` // Inner edge (e.g. left)
    // And `generateTrack(m_centerPoints, -trackOffset, m_outerTrack, m_trackColor);` // Outer edge (e.g. right)
    // This seems more logical for inner/outer edges if the normal points left.

    // Let's assume the current `trackOffset` is full width and `innerDefaultOffset` is 0.
    // The code is `generateTrack(m_centerPoints, innerDefaultOffset, m_innersTrack, m_trackColor);` -> center line
    // `generateTrack(m_centerPoints, -trackOffset, m_outerTrack, m_trackColor);` -> one edge (offset by full width)
    // This needs to be inner edge and outer edge based on m_trackWidth.
    // Let halfScaledTrackWidth = m_trackWidth * m_mmToPxRatio * m_scaleFactor / 2.0f;
    // generateTrack(m_centerPoints, halfScaledTrackWidth, m_innersTrack, m_trackColor); // Inner edge
    // generateTrack(m_centerPoints, -halfScaledTrackWidth, m_outerTrack, m_trackColor); // Outer edge

    float halfScaledTrackWidth = m_trackWidth * m_mmToPxRatio * m_scaleFactor / 2.0f;
    generateTrack(m_centerPoints, halfScaledTrackWidth, m_innersTrack, m_trackColor);  // Inner edge (positive offset if normal points left)
    generateTrack(m_centerPoints, -halfScaledTrackWidth, m_outerTrack, m_trackColor); // Outer edge (negative offset if normal points left)
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

    // 计算轨道的整体尺寸（像素）
    float scaledTrackLength = m_trackLength * m_mmToPxRatio * m_scaleFactor;
    float scaledCurveRadius = m_curveRadius * m_mmToPxRatio * m_scaleFactor;
    float totalWidth = scaledTrackLength + 2 * scaledCurveRadius;
    float totalHeight = 2 * scaledCurveRadius;
    float halfTotalWidth = totalWidth / 2.0f;
    float halfTotalHeight = totalHeight / 2.0f;

    // 转换坐标系
    return sf::Vector2f(
        scaledX - (halfTotalWidth - scaledCurveRadius),
        -(scaledY - halfTotalHeight) // Invert Y for render (Y-down) from internal (Y-up)
    );
}

// 坐标转换：渲染坐标系(轨道中心为原点) -> 后端坐标系(左下角弯道与直道交汇点为原点)
// renderPoint is Y-down from SFML
sf::Vector2f TrackRenderer::renderToBackendTransform(const sf::Vector2f &renderPoint) const
{
    // 计算轨道的整体尺寸（像素）
    float scaledTrackLength = m_trackLength * m_mmToPxRatio * m_scaleFactor;
    float scaledCurveRadius = m_curveRadius * m_mmToPxRatio * m_scaleFactor;
    float totalWidth = scaledTrackLength + 2 * scaledCurveRadius;
    float totalHeight = 2 * scaledCurveRadius;
    float halfTotalWidth = totalWidth / 2.0f;
    float halfTotalHeight = totalHeight / 2.0f;

    // 从渲染坐标系转换到后端坐标系（像素）
    // renderPoint.y is Y-down. To convert to internal Y-up for backend calculation:
    float internalRenderY = -renderPoint.y;

    float scaledX = renderPoint.x + (halfTotalWidth - scaledCurveRadius);
    float scaledY = internalRenderY + halfTotalHeight; // Now using Y-up internalRenderY

    // 转换回毫米单位
    return sf::Vector2f(
        scaledX / (m_mmToPxRatio * m_scaleFactor),
        scaledY / (m_mmToPxRatio * m_scaleFactor) // Resulting backend Y is Y-up
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