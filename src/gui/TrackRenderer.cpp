#include "../../include/gui/TrackRenderer.hpp"
#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

TrackRenderer::TrackRenderer()
    : m_straightSegments(sf::Lines),    // 使用线条绘制直轨道
      m_curveSegments(sf::Lines),       // 使用线条绘制弯曲轨道
      m_outerBorderSegments(sf::Lines), // 外边框线条
      m_innerTrackSegments(sf::Lines)   // 内轨道线条
{
}

void TrackRenderer::generateGeometry(float trackLength, float curveRadius)
{
    // 保存轨道参数以便后续坐标转换
    m_trackLength = trackLength;
    m_curveRadius = curveRadius;

    // 清除现有顶点
    m_straightSegments.clear();
    m_curveSegments.clear();
    m_outerBorderSegments.clear();
    m_innerTrackSegments.clear();

    // 将真实尺寸（毫米）转换为像素，确保宽度足够可见
    float scaledTrackLength = trackLength * m_mmToPxRatio * m_scaleFactor;
    float scaledCurveRadius = curveRadius * m_mmToPxRatio * m_scaleFactor;

    // 轨道实际宽度的一半 (1200mm/2 = 600mm)
    float trackHalfWidth = m_trackWidth * m_mmToPxRatio * m_scaleFactor / 2.0f;

    // 内外轨道之间的距离 (按实际轨道计算)
    float trackGap = trackHalfWidth * 0.8f; // 内轨距外轨的距离

    // 确保轨道宽度至少有2个像素
    if (trackHalfWidth < 1.0f)
    {
        trackHalfWidth = 1.0f;
    }

    // 计算轨道的整体尺寸 - 使用真实比例
    float totalWidth = scaledTrackLength + 2 * scaledCurveRadius;
    float totalHeight = 2 * scaledCurveRadius;

    // 使用轨道中心为坐标原点(0,0)
    float halfTotalWidth = totalWidth / 2.0f;
    float halfTotalHeight = totalHeight / 2.0f;

    // 为确保连接准确，计算关键点
    // 右弯道圆心
    float rightCenterX = halfTotalWidth - scaledCurveRadius;
    float rightCenterY = 0.0f;

    // 左弯道圆心
    float leftCenterX = -halfTotalWidth + scaledCurveRadius;
    float leftCenterY = 0.0f;

    // 轨道外线位置 (根据实际轨道宽度计算)
    float outerTop = -trackHalfWidth;
    float outerBottom = trackHalfWidth;

    // 轨道内线位置 (根据实际轨道宽度计算)
    float innerTop = -trackHalfWidth + trackGap;
    float innerBottom = trackHalfWidth - trackGap;

    // 确保精确的轨道连接
    int segmentCount = 60;                 // 每个弯道的分段数，增加精度
    float angleStep = M_PI / segmentCount; // 弯道角度步进值

    // 计算右弯道的开始和结束点（确保与直线段完美连接）
    float rightTopX = rightCenterX + scaledCurveRadius * std::cos(-M_PI / 2);
    float rightTopY = rightCenterY + scaledCurveRadius * std::sin(-M_PI / 2);
    float rightBottomX = rightCenterX + scaledCurveRadius * std::cos(M_PI / 2);
    float rightBottomY = rightCenterY + scaledCurveRadius * std::sin(M_PI / 2);

    // 计算左弯道的开始和结束点
    float leftTopX = leftCenterX + scaledCurveRadius * std::cos(M_PI / 2);
    float leftTopY = leftCenterY + scaledCurveRadius * std::sin(M_PI / 2);
    float leftBottomX = leftCenterX + scaledCurveRadius * std::cos(3 * M_PI / 2);
    float leftBottomY = leftCenterY + scaledCurveRadius * std::sin(3 * M_PI / 2);

    // ======================= 外轨道绘制 =======================
    // 1. 绘制最外层边框 - 上线
    for (int i = 0; i < 3; i++)
    {
        float offset = 0.5f * i;
        m_outerBorderSegments.append(sf::Vertex(sf::Vector2f(leftBottomX, outerTop - offset), m_outerBorderColor));
        m_outerBorderSegments.append(sf::Vertex(sf::Vector2f(rightTopX, outerTop - offset), m_outerBorderColor));
    }

    // 2. 绘制最外层边框 - 下线
    for (int i = 0; i < 3; i++)
    {
        float offset = 0.5f * i;
        m_outerBorderSegments.append(sf::Vertex(sf::Vector2f(leftTopX, outerBottom + offset), m_outerBorderColor));
        m_outerBorderSegments.append(sf::Vertex(sf::Vector2f(rightBottomX, outerBottom + offset), m_outerBorderColor));
    }

    // 3. 绘制右侧弯道的最外层粗边框
    addCurveBorder(
        rightCenterX, rightCenterY,
        scaledCurveRadius + trackHalfWidth, -M_PI / 2, M_PI / 2, segmentCount);

    // 4. 绘制左侧弯道的最外层粗边框
    addCurveBorder(
        leftCenterX, leftCenterY,
        scaledCurveRadius + trackHalfWidth, M_PI / 2, 3 * M_PI / 2, segmentCount);

    // ======================= 内轨道绘制 =======================
    // 5. 绘制上方内轨道线
    m_innerTrackSegments.append(sf::Vertex(sf::Vector2f(leftBottomX, innerTop), m_straightColor));
    m_innerTrackSegments.append(sf::Vertex(sf::Vector2f(rightTopX, innerTop), m_straightColor));

    // 6. 绘制下方内轨道线
    m_innerTrackSegments.append(sf::Vertex(sf::Vector2f(leftTopX, innerBottom), m_straightColor));
    m_innerTrackSegments.append(sf::Vertex(sf::Vector2f(rightBottomX, innerBottom), m_straightColor));

    // 7. 绘制右侧弯道内轨
    float innerRadius = scaledCurveRadius - trackHalfWidth + trackGap;
    addCurveOutline(
        rightCenterX, rightCenterY,
        innerRadius, -M_PI / 2, M_PI / 2, segmentCount);

    // 8. 绘制左侧弯道内轨
    addCurveOutline(
        leftCenterX, leftCenterY,
        innerRadius, M_PI / 2, 3 * M_PI / 2, segmentCount);
}

// 使用线条绘制曲线轮廓
void TrackRenderer::addCurveOutline(float centerX, float centerY, float radius,
                                    float startAngle, float endAngle, int segments)
{
    float angleStep = (endAngle - startAngle) / segments;

    for (int i = 0; i < segments; ++i)
    {
        float angle1 = startAngle + i * angleStep;
        float angle2 = startAngle + (i + 1) * angleStep;

        float x1 = centerX + radius * std::cos(angle1);
        float y1 = centerY + radius * std::sin(angle1);
        float x2 = centerX + radius * std::cos(angle2);
        float y2 = centerY + radius * std::sin(angle2);

        m_innerTrackSegments.append(sf::Vertex(sf::Vector2f(x1, y1), m_straightColor));
        m_innerTrackSegments.append(sf::Vertex(sf::Vector2f(x2, y2), m_straightColor));
    }
}

// 绘制多层线条来创建粗边框效果
void TrackRenderer::addCurveBorder(float centerX, float centerY, float radius,
                                   float startAngle, float endAngle, int segments)
{
    float angleStep = (endAngle - startAngle) / segments;

    // 绘制多条线以创建粗线效果
    for (int layer = 0; layer < 3; layer++)
    {
        float layerRadius = radius + 0.5f * layer;

        for (int i = 0; i < segments; ++i)
        {
            float angle1 = startAngle + i * angleStep;
            float angle2 = startAngle + (i + 1) * angleStep;

            float x1 = centerX + layerRadius * std::cos(angle1);
            float y1 = centerY + layerRadius * std::sin(angle1);
            float x2 = centerX + layerRadius * std::cos(angle2);
            float y2 = centerY + layerRadius * std::sin(angle2);

            m_outerBorderSegments.append(sf::Vertex(sf::Vector2f(x1, y1), m_outerBorderColor));
            m_outerBorderSegments.append(sf::Vertex(sf::Vector2f(x2, y2), m_outerBorderColor));
        }
    }
}

// 添加具有宽度的曲线段（使用三角形扇形）- 保留但不使用
void TrackRenderer::addCurveSegment(float centerX, float centerY, float radius, float halfWidth,
                                    float startAngle, float endAngle, int segments)
{
    float angleStep = (endAngle - startAngle) / segments;

    for (int i = 0; i < segments; ++i)
    {
        float angle1 = startAngle + i * angleStep;
        float angle2 = startAngle + (i + 1) * angleStep;

        // 内圈点
        float innerX1 = centerX + (radius - halfWidth) * std::cos(angle1);
        float innerY1 = centerY + (radius - halfWidth) * std::sin(angle1);
        float innerX2 = centerX + (radius - halfWidth) * std::cos(angle2);
        float innerY2 = centerY + (radius - halfWidth) * std::sin(angle2);

        // 外圈点
        float outerX1 = centerX + (radius + halfWidth) * std::cos(angle1);
        float outerY1 = centerY + (radius + halfWidth) * std::sin(angle1);
        float outerX2 = centerX + (radius + halfWidth) * std::cos(angle2);
        float outerY2 = centerY + (radius + halfWidth) * std::sin(angle2);

        // 添加两个三角形组成一个四边形段
        // 三角形1：内1, 外1, 外2
        m_curveSegments.append(sf::Vertex(sf::Vector2f(innerX1, innerY1), m_curveColor));
        m_curveSegments.append(sf::Vertex(sf::Vector2f(outerX1, outerY1), m_curveColor));
        m_curveSegments.append(sf::Vertex(sf::Vector2f(outerX2, outerY2), m_curveColor));

        // 三角形2：内1, 外2, 内2
        m_curveSegments.append(sf::Vertex(sf::Vector2f(innerX1, innerY1), m_curveColor));
        m_curveSegments.append(sf::Vertex(sf::Vector2f(outerX2, outerY2), m_curveColor));
        m_curveSegments.append(sf::Vertex(sf::Vector2f(innerX2, innerY2), m_curveColor));
    }
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
        scaledY - halfTotalHeight);
}

// 坐标转换：渲染坐标系(轨道中心为原点) -> 后端坐标系(左下角弯道与直道交汇点为原点)
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
    float scaledX = renderPoint.x + (halfTotalWidth - scaledCurveRadius);
    float scaledY = renderPoint.y + halfTotalHeight;

    // 转换回毫米单位
    return sf::Vector2f(
        scaledX / (m_mmToPxRatio * m_scaleFactor),
        scaledY / (m_mmToPxRatio * m_scaleFactor));
}

// 可选的位置偏移绘制方法
void TrackRenderer::render(sf::RenderTarget &target, const sf::Vector2f &position)
{
    sf::RenderStates states;
    states.transform.translate(position);
    target.draw(m_outerBorderSegments, states);
    target.draw(m_innerTrackSegments, states);
}

void TrackRenderer::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    // 绘制外边框
    target.draw(m_outerBorderSegments, states);

    // 绘制内轨道
    target.draw(m_innerTrackSegments, states);
}