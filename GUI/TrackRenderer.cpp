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

    // 根据开发界面信息设置轨道参数
    // 直轨长度：40000mm
    // 弯轨半径：2500mm
    float straightLength = 40000.0f;

    // 轨道中心点（左下角弯道中心）
    float centerX = curveRadius;
    float centerY = curveRadius;

    // 生成直线部分
    // 下方直轨
    addLineSegment(centerX, centerY - curveRadius,
                   centerX + straightLength, centerY - curveRadius,
                   m_straightColor);

    // 右侧直轨
    addLineSegment(centerX + straightLength + curveRadius, centerY,
                   centerX + straightLength + curveRadius, centerY + straightLength,
                   m_straightColor);

    // 上方直轨
    addLineSegment(centerX + straightLength, centerY + straightLength + curveRadius,
                   centerX, centerY + straightLength + curveRadius,
                   m_straightColor);

    // 左侧直轨
    addLineSegment(centerX - curveRadius, centerY + straightLength,
                   centerX - curveRadius, centerY,
                   m_straightColor);

    // 生成四个弯道（圆弧）
    // 左下角弯道（原点）
    addArcSegment(centerX, centerY,
                  curveRadius, 0.5f * M_PI, M_PI, 20);

    // 右下角弯道
    addArcSegment(centerX + straightLength, centerY,
                  curveRadius, 0, 0.5f * M_PI, 20);

    // 右上角弯道
    addArcSegment(centerX + straightLength, centerY + straightLength,
                  curveRadius, 1.5f * M_PI, 2.0f * M_PI, 20);

    // 左上角弯道
    addArcSegment(centerX, centerY + straightLength,
                  curveRadius, M_PI, 1.5f * M_PI, 20);
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