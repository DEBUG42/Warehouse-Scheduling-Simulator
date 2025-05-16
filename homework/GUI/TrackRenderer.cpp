#include "TrackRenderer.hpp"
#include <cmath>

TrackRenderer::TrackRenderer()
    : m_straightSegments(sf::Lines),
      m_curveSegments(sf::Lines)
{
    // 初始化时顶点数组为空，等待generateGeometry调用
}

float TrackRenderer::getActualTrackLength() const
{
    // 计算实际轨道总长度 = 2 * 直道长度 + 2 * π * 弯道半径
    return 2 * m_straightLength + 2 * M_PI * m_curveRadius;
}

void TrackRenderer::generateGeometry(float trackLength, float curveRadius)
{
    // 清除现有顶点
    m_straightSegments.clear();
    m_curveSegments.clear();

    // =====================
    // 轨道参数（全部以mm为单位，中心线为基准）
    // =====================
    float R = curveRadius;                            // 弯道半径（mm，中心线）
    float L = (trackLength - 2.0f * M_PI * R) / 2.0f; // 单段直道长度（mm，中心线）
    if (L < 0)
        L = 40000.0f; // 容错    // 单位换算：1mm = 0.05px
    float scale = 0.05f;
    float L_px = L * scale;
    float R_px = R * scale;
    float trackWidth_px = m_trackWidth; // m_trackWidth 已经是像素单位，在 setTrackWidth 中转换了

    // =====================
    // 轨道中心线几何
    // =====================
    // 上直道（中心线Y=-R）
    addLineSegment(-L_px / 2, -R_px, L_px / 2, -R_px, m_straightColor);
    // 下直道（中心线Y=+R）
    addLineSegment(L_px / 2, R_px, -L_px / 2, R_px, m_straightColor);

    // 右半圆弯道（圆心在(L/2,0)，从上到下，角度270°到90°）
    addArcSegment(L_px / 2, 0, R_px, 1.5f * M_PI, 0.5f * M_PI, 40);

    // 左半圆弯道（圆心在(-L/2,0)，从下到上，角度90°到270°）
    addArcSegment(-L_px / 2, 0, R_px, 0.5f * M_PI, 1.5f * M_PI, 40); // =====================
    // 绘制内外轨道（在半径上加减一半轨道宽度）
    float halfWidth = trackWidth_px / 2.0f;

    // 上直道内轨（内侧）- Y坐标更小的一侧
    addLineSegment(-L_px / 2, -R_px - halfWidth, L_px / 2, -R_px - halfWidth, m_straightColor);
    // 上直道外轨（外侧）- Y坐标更大的一侧
    addLineSegment(-L_px / 2, -R_px + halfWidth, L_px / 2, -R_px + halfWidth, m_straightColor);

    // 下直道内轨（内侧）- Y坐标更大的一侧
    addLineSegment(L_px / 2, R_px + halfWidth, -L_px / 2, R_px + halfWidth, m_straightColor);
    // 下直道外轨（外侧）- Y坐标更小的一侧
    addLineSegment(L_px / 2, R_px - halfWidth, -L_px / 2, R_px - halfWidth, m_straightColor);

    // 右弯道内轨（内侧）- 半径更小
    addArcSegment(L_px / 2, 0, R_px - halfWidth, 1.5f * M_PI, 0.5f * M_PI, 40);
    // 右弯道外轨（外侧）- 半径更大
    addArcSegment(L_px / 2, 0, R_px + halfWidth, 1.5f * M_PI, 0.5f * M_PI, 40);

    // 左弯道内轨（内侧）- 半径更小
    addArcSegment(-L_px / 2, 0, R_px - halfWidth, 0.5f * M_PI, 1.5f * M_PI, 40);
    // 左弯道外轨（外侧）- 半径更大
    addArcSegment(-L_px / 2, 0, R_px + halfWidth, 0.5f * M_PI, 1.5f * M_PI, 40);
}

void TrackRenderer::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    // 首先绘制直线段
    target.draw(m_straightSegments, states);

    // 然后绘制弯道段
    target.draw(m_curveSegments, states);
}

void TrackRenderer::addLineSegment(float x1, float y1, float x2, float y2, const sf::Color &color)
{
    m_straightSegments.append(sf::Vertex(sf::Vector2f(x1, y1), color));
    m_straightSegments.append(sf::Vertex(sf::Vector2f(x2, y2), color));
}

void TrackRenderer::addArcSegment(float centerX, float centerY, float radius,
                                  float startAngle, float endAngle, int segments)
{
    // 计算弧段的角度步长
    float angleStep = (endAngle - startAngle) / segments;

    // 按步长生成多个线段组成圆弧
    for (int i = 0; i < segments; ++i)
    {
        // 计算线段两端点的角度
        float angle1 = startAngle + i * angleStep;
        float angle2 = startAngle + (i + 1) * angleStep;

        // 计算线段两端点的坐标
        float x1 = centerX + radius * cos(angle1);
        float y1 = centerY + radius * sin(angle1);
        float x2 = centerX + radius * cos(angle2);
        float y2 = centerY + radius * sin(angle2);

        // 添加到顶点数组
        m_curveSegments.append(sf::Vertex(sf::Vector2f(x1, y1), m_curveColor));
        m_curveSegments.append(sf::Vertex(sf::Vector2f(x2, y2), m_curveColor));
    }
}