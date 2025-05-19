#include "TrackRenderer_fixed.hpp"
#include <cmath>
#include <iostream>

TrackRenderer::TrackRenderer()
    : m_centerLines(sf::Lines), m_innerTrackLines(sf::Lines), m_outerTrackLines(sf::Lines)
{
}

void TrackRenderer::generateGeometry(float trackLength, float curveRadius)
{
    // 记录参数
    m_trackLength = trackLength;
    m_curveRadius = curveRadius;

    // 清除原有几何数据
    m_centerLines.clear();
    m_innerTrackLines.clear();
    m_outerTrackLines.clear();

    // 计算直道部分长度 (mm)，总长减去两个半圆的周长
    m_straightLength = trackLength - 2.0f * M_PI * curveRadius;

    // 转换为像素单位
    float L_px = m_straightLength * MM_TO_PIXEL;
    float R_px = m_curveRadius * MM_TO_PIXEL;
    float trackWidth_px = m_trackWidth; // 已经是像素单位

    // 轨道宽度的一半，用于计算内外轨
    float halfWidth = trackWidth_px / 2.0f;

    // 绘制中心线
    // 上方直线段中心线
    addLineSegment(-L_px / 2, -R_px, L_px / 2, -R_px, m_centerLineColor);

    // 下方直线段中心线
    addLineSegment(-L_px / 2, R_px, L_px / 2, R_px, m_centerLineColor);

    // 左侧半圆中心线
    addArcSegment(-L_px / 2, 0, R_px, -M_PI / 2, M_PI / 2, 20, m_centerLineColor);

    // 右侧半圆中心线
    addArcSegment(L_px / 2, 0, R_px, M_PI / 2, 3 * M_PI / 2, 20, m_centerLineColor);

    // 绘制外轨道线
    // 上方直线段外轨（较小Y值）
    addLineSegment(-L_px / 2, -R_px - halfWidth, L_px / 2, -R_px - halfWidth, m_straightColor);

    // 下方直线段外轨（较大Y值）
    addLineSegment(-L_px / 2, R_px + halfWidth, L_px / 2, R_px + halfWidth, m_straightColor);

    // 左侧半圆外轨
    addArcSegment(-L_px / 2, 0, R_px + halfWidth, -M_PI / 2, M_PI / 2, 20, m_curveColor);

    // 右侧半圆外轨
    addArcSegment(L_px / 2, 0, R_px + halfWidth, M_PI / 2, 3 * M_PI / 2, 20, m_curveColor);

    // 绘制内轨道线
    // 上方直线段内轨（较小Y值）
    addLineSegment(-L_px / 2, -R_px + halfWidth, L_px / 2, -R_px + halfWidth, m_straightColor);

    // 下方直线段内轨（较大Y值）
    addLineSegment(-L_px / 2, R_px - halfWidth, L_px / 2, R_px - halfWidth, m_straightColor);

    // 左侧半圆内轨
    addArcSegment(-L_px / 2, 0, R_px - halfWidth, -M_PI / 2, M_PI / 2, 20, m_curveColor);

    // 右侧半圆内轨
    addArcSegment(L_px / 2, 0, R_px - halfWidth, M_PI / 2, 3 * M_PI / 2, 20, m_curveColor);
}

void TrackRenderer::setTrackWidth(float width)
{
    // 存储像素单位的轨道宽度
    m_trackWidth = width * MM_TO_PIXEL;

    // 如果已经生成了几何形状，则重新生成
    if (m_trackLength > 0.0f && m_curveRadius > 0.0f)
    {
        generateGeometry(m_trackLength, m_curveRadius);
    }
}

float TrackRenderer::getActualTrackLength() const
{
    return m_trackLength;
}

void TrackRenderer::setCenterLineColor(const sf::Color &color)
{
    m_centerLineColor = color;
}

void TrackRenderer::setStraightColor(const sf::Color &color)
{
    m_straightColor = color;
}

void TrackRenderer::setCurveColor(const sf::Color &color)
{
    m_curveColor = color;
}

void TrackRenderer::addLineSegment(float x1, float y1, float x2, float y2, const sf::Color &color)
{
    sf::Vertex v1(sf::Vector2f(x1, y1), color);
    sf::Vertex v2(sf::Vector2f(x2, y2), color);

    if (color == m_centerLineColor)
    {
        m_centerLines.append(v1);
        m_centerLines.append(v2);
    }
    else if (std::abs(y1) > std::abs(y2 ? y2 : y1))
    { // 判断是否为外轨
        m_outerTrackLines.append(v1);
        m_outerTrackLines.append(v2);
    }
    else
    {
        m_innerTrackLines.append(v1);
        m_innerTrackLines.append(v2);
    }
}

void TrackRenderer::addArcSegment(float centerX, float centerY, float radius,
                                  float startAngle, float endAngle, int segments, const sf::Color &color)
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

        addLineSegment(x1, y1, x2, y2, color);
    }
}

void TrackRenderer::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    target.draw(m_centerLines, states);
    target.draw(m_innerTrackLines, states);
    target.draw(m_outerTrackLines, states);
}
