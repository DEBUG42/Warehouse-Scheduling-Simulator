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
    // 假设轨道是一个矩形带圆角的形状
    // 轨道的直线部分总长度 = 轨道总长 - (4 * 90度弧长)
    // 一个90度弧的长度 = 0.5 * PI * 弯道半径
    float arcLength = 0.5f * M_PI * curveRadius;
    float totalArcLength = 4 * arcLength;
    float straightLength = trackLength - totalArcLength;

    // 每边的直线长度（平均分配）
    float sideLength = straightLength / 4;

    // 轨道矩形的尺寸
    float width = 2 * curveRadius + sideLength;
    float height = 2 * curveRadius + sideLength;

    // 轨道中心
    float centerX = 0.0f;
    float centerY = 0.0f;

    // 生成直线部分（四边形的四条边）
    // 上边
    addLineSegment(centerX - width / 2 + curveRadius, centerY - height / 2,
                   centerX + width / 2 - curveRadius, centerY - height / 2,
                   m_straightColor);

    // 右边
    addLineSegment(centerX + width / 2, centerY - height / 2 + curveRadius,
                   centerX + width / 2, centerY + height / 2 - curveRadius,
                   m_straightColor);

    // 下边
    addLineSegment(centerX + width / 2 - curveRadius, centerY + height / 2,
                   centerX - width / 2 + curveRadius, centerY + height / 2,
                   m_straightColor);

    // 左边
    addLineSegment(centerX - width / 2, centerY + height / 2 - curveRadius,
                   centerX - width / 2, centerY - height / 2 + curveRadius,
                   m_straightColor);

    // 生成四个弯道（圆弧）
    // 左上角弯道
    addArcSegment(centerX - width / 2 + curveRadius, centerY - height / 2 + curveRadius,
                  curveRadius, M_PI, 1.5f * M_PI, 20);

    // 右上角弯道
    addArcSegment(centerX + width / 2 - curveRadius, centerY - height / 2 + curveRadius,
                  curveRadius, 1.5f * M_PI, 2.0f * M_PI, 20);

    // 右下角弯道
    addArcSegment(centerX + width / 2 - curveRadius, centerY + height / 2 - curveRadius,
                  curveRadius, 0, 0.5f * M_PI, 20);

    // 左下角弯道
    addArcSegment(centerX - width / 2 + curveRadius, centerY + height / 2 - curveRadius,
                  curveRadius, 0.5f * M_PI, M_PI, 20);
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