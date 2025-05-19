#include "TrackRenderer_fixed.hpp"
#include <cmath>
#include <iostream>

TrackRenderer::TrackRenderer()
    : m_centerLines(sf::Lines), m_innerTrackLines(sf::Lines), m_outerTrackLines(sf::Lines)
{
    // 默认设置
    m_centerLineColor = sf::Color(180, 180, 180);
    m_straightColor = sf::Color(160, 160, 160);
    m_curveColor = sf::Color(160, 160, 160);
}

void TrackRenderer::generateGeometry(float trackLength, float curveRadius)
{
    // 记录参数（毫米单位）
    m_trackLength = trackLength; // 全长（毫米）
    m_curveRadius = curveRadius; // 弯道半径（毫米）

    // 清除原有几何数据
    m_centerLines.clear();
    m_innerTrackLines.clear();
    m_outerTrackLines.clear();

    // 计算直道部分长度 (mm)
    float straightLength = 40000.0f; // 题目要求直道长度固定为40000mm

    // 转换为像素单位
    float straightLength_px = straightLength * MM_TO_PIXEL;
    float curveRadius_px = m_curveRadius * MM_TO_PIXEL;
    float trackWidth_px = m_trackWidth; // 已经是像素单位

    // 轨道宽度的一半，用于计算内外轨
    float halfWidth = trackWidth_px / 2.0f;

    // 按照指定坐标系统和原点位置绘制轨道
    // 原点(0,0)在左下角弯道中心

    // 绘制中心线
    // 下方直线段中心线 (左侧原点(0,-2500)到右侧(40000,-2500))
    addLineSegment(0, -curveRadius_px, straightLength_px, -curveRadius_px, m_centerLineColor);

    // 上方直线段中心线 (右侧(40000,2500)到左侧(0,2500))
    addLineSegment(straightLength_px, curveRadius_px, 0, curveRadius_px, m_centerLineColor);

    // 左侧半圆中心线 (从下方(0,-2500)到上方(0,2500))
    addArcSegment(0, 0, curveRadius_px, -M_PI / 2, M_PI / 2, 40, m_centerLineColor);

    // 右侧半圆中心线 (从上方(40000,2500)到下方(40000,-2500))
    addArcSegment(straightLength_px, 0, curveRadius_px, M_PI / 2, 3 * M_PI / 2, 40, m_centerLineColor);

    // 绘制外轨道线
    // 下方直线段外轨（较大Y值）
    addLineSegment(0, -curveRadius_px - halfWidth, straightLength_px, -curveRadius_px - halfWidth, m_straightColor);

    // 上方直线段外轨（较大Y值）
    addLineSegment(straightLength_px, curveRadius_px + halfWidth, 0, curveRadius_px + halfWidth, m_straightColor);

    // 左侧半圆外轨
    addArcSegment(0, 0, curveRadius_px + halfWidth, -M_PI / 2, M_PI / 2, 40, m_curveColor);

    // 右侧半圆外轨
    addArcSegment(straightLength_px, 0, curveRadius_px + halfWidth, M_PI / 2, 3 * M_PI / 2, 40, m_curveColor);

    // 绘制内轨道线
    // 下方直线段内轨（较小Y值）
    addLineSegment(0, -curveRadius_px + halfWidth, straightLength_px, -curveRadius_px + halfWidth, m_straightColor);

    // 上方直线段内轨（较小Y值）
    addLineSegment(straightLength_px, curveRadius_px - halfWidth, 0, curveRadius_px - halfWidth, m_straightColor);

    // 左侧半圆内轨
    addArcSegment(0, 0, curveRadius_px - halfWidth, -M_PI / 2, M_PI / 2, 40, m_curveColor);

    // 右侧半圆内轨
    addArcSegment(straightLength_px, 0, curveRadius_px - halfWidth, M_PI / 2, 3 * M_PI / 2, 40, m_curveColor);
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
