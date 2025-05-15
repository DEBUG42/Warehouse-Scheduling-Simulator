#include "TrackRenderer.hpp"
#include <cmath>

TrackRenderer::TrackRenderer()
    : m_straightSegments(sf::Lines),
      m_curveSegments(sf::Lines)
{
}

void TrackRenderer::generateGeometry(float trackLength, float curveRadius)
{
    // 清除现有顶点
    m_straightSegments.clear();
    m_curveSegments.clear();

    // 计算轨道参数
    // 赛道形状 = 两条平行直线 + 两个半圆
    // 直线部分长度 = 轨道总长 - (2 * 半圆弧长)
    // 半圆弧长 = PI * 弯道半径
    float arcLength = M_PI * curveRadius;
    float totalArcLength = 2 * arcLength;
    float straightLength = trackLength - totalArcLength;

    // 轨道的直线部分长度（两侧各一半）
    float sideLength = straightLength / 2;

    // 赛道总尺寸
    float width = sideLength;
    float height = 2 * curveRadius;

    // 轨道中心
    float centerX = 0.0f;
    float centerY = 0.0f;

    // 轨道内外边轨，考虑轨道宽度
    float innerOffset = m_trackWidth / 2;
    float outerOffset = m_trackWidth / 2;

    // 生成直线部分（上下两条平行线）
    // 上边 - 两条平行线（内外轨）
    // 上部外轨
    addLineSegment(centerX - width / 2, centerY - height / 2 + innerOffset,
                   centerX + width / 2, centerY - height / 2 + innerOffset,
                   m_straightColor);
    // 上部内轨
    addLineSegment(centerX - width / 2, centerY - height / 2 + outerOffset + m_trackWidth,
                   centerX + width / 2, centerY - height / 2 + outerOffset + m_trackWidth,
                   m_straightColor);

    // 下边 - 两条平行线（内外轨）
    // 下部外轨
    addLineSegment(centerX + width / 2, centerY + height / 2 - innerOffset,
                   centerX - width / 2, centerY + height / 2 - innerOffset,
                   m_straightColor);
    // 下部内轨
    addLineSegment(centerX + width / 2, centerY + height / 2 - outerOffset - m_trackWidth,
                   centerX - width / 2, centerY + height / 2 - outerOffset - m_trackWidth,
                   m_straightColor);

    // 生成两侧半圆弯道
    // 左侧半圆 - 外轨
    addArcSegment(centerX - width / 2, centerY,
                  height / 2 - innerOffset, M_PI * 1.5f, M_PI * 0.5f, 30);
    // 左侧半圆 - 内轨
    addArcSegment(centerX - width / 2, centerY,
                  height / 2 - outerOffset - m_trackWidth, M_PI * 1.5f, M_PI * 0.5f, 30);

    // 右侧半圆 - 外轨
    addArcSegment(centerX + width / 2, centerY,
                  height / 2 - innerOffset, M_PI * 0.5f, M_PI * 1.5f, 30);
    // 右侧半圆 - 内轨
    addArcSegment(centerX + width / 2, centerY,
                  height / 2 - outerOffset - m_trackWidth, M_PI * 0.5f, M_PI * 1.5f, 30);
}

void TrackRenderer::addLineSegment(float x1, float y1, float x2, float y2, const sf::Color &color)
{
    sf::Vertex v1(sf::Vector2f(x1, y1), color);
    sf::Vertex v2(sf::Vector2f(x2, y2), color);

    m_straightSegments.append(v1);
    m_straightSegments.append(v2);
}

void TrackRenderer::addArcSegment(float centerX, float centerY, float radius,
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

        sf::Vertex v1(sf::Vector2f(x1, y1), m_curveColor);
        sf::Vertex v2(sf::Vector2f(x2, y2), m_curveColor);

        m_curveSegments.append(v1);
        m_curveSegments.append(v2);
    }
}

void TrackRenderer::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    // 绘制直线部分
    target.draw(m_straightSegments, states);

    // 绘制弯道部分
    target.draw(m_curveSegments, states);
}