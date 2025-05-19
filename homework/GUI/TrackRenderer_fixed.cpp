#include "TrackRenderer_fixed.hpp"
#include <cmath>
#include <iostream>

/**
 * @brief 构造函数
 *
 * 初始化顶点数组为各自适当的图元类型
 */
TrackRenderer_fixed::TrackRenderer_fixed()
    : m_straightSegments(sf::Lines),
      m_curveSegments(sf::Lines),
      m_trackMarkings(sf::Lines)
{
    // 默认生成标准轨道几何形状
    generateGeometry(m_trackLength, m_curveRadius);
}

/**
 * @brief 根据物理参数生成轨道几何形状
 * @param trackLength 轨道总长（毫米）
 * @param curveRadius 弯道半径（毫米）
 */
void TrackRenderer_fixed::generateGeometry(float trackLength, float curveRadius)
{
    // 清除现有顶点
    m_straightSegments.clear();
    m_curveSegments.clear();
    m_trackMarkings.clear();

    // =====================
    // 轨道参数（全部以mm为单位，中心线为基准）
    // =====================
    m_trackLength = trackLength;
    m_curveRadius = curveRadius;

    float R = curveRadius; // 弯道半径（mm，中心线）

    // 计算单段直道长度（mm，中心线）
    // 轨道总长度 = 2 * 直道长度 + 2 * π * 弯道半径
    float L = (trackLength - 2.0f * M_PI * R) / 2.0f;

    // 容错：如果计算出的直道长度为负，使用默认值
    if (L < 0)
    {
        L = 40000.0f;
        std::cerr << "警告: 计算的直道长度为负值，使用默认值40000mm" << std::endl;
    }

    m_straightLength = L;

    // 单位换算：1mm = MM_TO_PIXEL px
    float L_px = mmToPixel(L);
    float R_px = mmToPixel(R);
    float trackWidth_px = m_trackWidth; // 已经转换为像素单位

    // =====================
    // 轨道中心线几何
    // =====================
    // 上直道（中心线Y=-R）
    addLineSegment(-L_px / 2, -R_px, L_px / 2, -R_px, m_straightColor);

    // 下直道（中心线Y=+R）
    addLineSegment(L_px / 2, R_px, -L_px / 2, R_px, m_straightColor);

    // 右半圆弯道（圆心在(L/2,0)，从上到下，角度270°到90°）
    addArcSegment(L_px / 2, 0, R_px, 1.5f * M_PI, 0.5f * M_PI, 40, m_curveColor);

    // 左半圆弯道（圆心在(-L/2,0)，从下到上，角度90°到270°）
    addArcSegment(-L_px / 2, 0, R_px, 0.5f * M_PI, 1.5f * M_PI, 40, m_curveColor);

    // =====================
    // 绘制内外轨道（在半径上加减一半轨道宽度）
    // =====================
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
    addArcSegment(L_px / 2, 0, R_px - halfWidth, 1.5f * M_PI, 0.5f * M_PI, 40, m_curveColor);

    // 右弯道外轨（外侧）- 半径更大
    addArcSegment(L_px / 2, 0, R_px + halfWidth, 1.5f * M_PI, 0.5f * M_PI, 40, m_curveColor);

    // 左弯道内轨（内侧）- 半径更小
    addArcSegment(-L_px / 2, 0, R_px - halfWidth, 0.5f * M_PI, 1.5f * M_PI, 40, m_curveColor);

    // 左弯道外轨（外侧）- 半径更大
    addArcSegment(-L_px / 2, 0, R_px + halfWidth, 0.5f * M_PI, 1.5f * M_PI, 40, m_curveColor);

    // 添加轨道标记
    addTrackMarkings();
}

/**
 * @brief 重载的绘制方法
 * @param target 渲染目标
 * @param states 渲染状态
 */
void TrackRenderer_fixed::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    // 首先绘制直线段
    target.draw(m_straightSegments, states);

    // 然后绘制弯道段
    target.draw(m_curveSegments, states);

    // 最后绘制轨道标记
    target.draw(m_trackMarkings, states);
}

/**
 * @brief 添加直线段到顶点数组
 * @param x1 起点X坐标（像素）
 * @param y1 起点Y坐标（像素）
 * @param x2 终点X坐标（像素）
 * @param y2 终点Y坐标（像素）
 * @param color 线段颜色
 */
void TrackRenderer_fixed::addLineSegment(float x1, float y1, float x2, float y2, const sf::Color &color)
{
    m_straightSegments.append(sf::Vertex(sf::Vector2f(x1, y1), color));
    m_straightSegments.append(sf::Vertex(sf::Vector2f(x2, y2), color));
}

/**
 * @brief 添加圆弧段到顶点数组
 * @param centerX 圆心X坐标（像素）
 * @param centerY 圆心Y坐标（像素）
 * @param radius 圆弧半径（像素）
 * @param startAngle 起始角度(弧度)
 * @param endAngle 结束角度(弧度)
 * @param segments 圆弧分段数
 * @param color 圆弧颜色
 */
void TrackRenderer_fixed::addArcSegment(float centerX, float centerY, float radius,
                                        float startAngle, float endAngle, int segments,
                                        const sf::Color &color)
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
        m_curveSegments.append(sf::Vertex(sf::Vector2f(x1, y1), color));
        m_curveSegments.append(sf::Vertex(sf::Vector2f(x2, y2), color));
    }
}

/**
 * @brief 添加轨道标记（距离刻度）
 *
 * 每隔固定距离（如1000mm）在轨道上添加一个标记线
 * 标记线垂直于轨道方向，跨越轨道宽度
 */
void TrackRenderer_fixed::addTrackMarkings()
{
    // 标记间隔（毫米）
    const float markInterval = 5000.0f; // 每5米一个标记

    // 轨道总长（毫米）
    float trackLength = m_trackLength;

    // 计算标记数量
    int markCount = static_cast<int>(trackLength / markInterval);

    // 轨道参数（像素）
    float R_px = mmToPixel(m_curveRadius);
    float L_px = mmToPixel(m_straightLength);
    float halfWidth = m_trackWidth / 2.0f;

    // 轨道总长度（像素）
    float totalLength_px = 2 * L_px + 2 * M_PI * R_px;

    // 为每个标记添加一条短线
    for (int i = 0; i < markCount; ++i)
    {
        // 标记位置（毫米）
        float markPos_mm = i * markInterval;

        // 标记位置占轨道总长的比例
        float relativePos = markPos_mm / trackLength;

        // 标记在轨道上的位置（像素）
        float pos_px = relativePos * totalLength_px;

        // 确定标记在轨道的哪个部分
        float upperStraight = L_px;                     // 上直道终点
        float rightCurve = upperStraight + M_PI * R_px; // 右弯道终点
        float lowerStraight = rightCurve + L_px;        // 下直道终点

        sf::Vector2f pos1, pos2;

        if (pos_px < upperStraight)
        {
            // 在上直道
            float x = pos_px - L_px / 2;
            pos1 = sf::Vector2f(x, -R_px - halfWidth);
            pos2 = sf::Vector2f(x, -R_px + halfWidth);
        }
        else if (pos_px < rightCurve)
        {
            // 在右弯道
            float angle = (pos_px - upperStraight) / R_px;
            float sinAngle = sin(angle);
            float cosAngle = cos(angle);

            // 计算标记内外两端点（垂直于轨道）
            sf::Vector2f center(L_px / 2, 0);
            sf::Vector2f direction(sinAngle, cosAngle); // 从圆心指向轨道点的方向向量

            pos1 = center + direction * (R_px - halfWidth);
            pos2 = center + direction * (R_px + halfWidth);
        }
        else if (pos_px < lowerStraight)
        {
            // 在下直道
            float x = L_px / 2 - (pos_px - rightCurve);
            pos1 = sf::Vector2f(x, R_px - halfWidth);
            pos2 = sf::Vector2f(x, R_px + halfWidth);
        }
        else
        {
            // 在左弯道
            float angle = (pos_px - lowerStraight) / R_px;
            float sinAngle = sin(angle);
            float cosAngle = cos(angle);

            // 计算标记内外两端点（垂直于轨道）
            sf::Vector2f center(-L_px / 2, 0);
            sf::Vector2f direction(-sinAngle, -cosAngle); // 从圆心指向轨道点的方向向量

            pos1 = center + direction * (R_px - halfWidth);
            pos2 = center + direction * (R_px + halfWidth);
        }

        // 添加标记线
        m_trackMarkings.append(sf::Vertex(pos1, m_markingColor));
        m_trackMarkings.append(sf::Vertex(pos2, m_markingColor));
    }
}
