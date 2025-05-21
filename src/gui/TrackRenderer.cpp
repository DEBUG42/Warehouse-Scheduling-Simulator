#include "../../include/gui/TrackRenderer.hpp"
#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

TrackRenderer::TrackRenderer()
    : m_innersTrack(sf::LineStrip), // 内轨道线条
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
    m_centerPoints.clear();

    // 将真实尺寸（毫米）转换为像素
    float scaledTrackLength = trackLength * m_mmToPxRatio * m_scaleFactor;
    float scaledCurveRadius = curveRadius * m_mmToPxRatio * m_scaleFactor;

    // 轨道宽度计算 - 转换为像素
    float trackOffset = m_trackWidth * m_mmToPxRatio * m_scaleFactor / 2.0f; // 内轨道偏移量
    float trackWidthInPixels = m_trackWidth * m_mmToPxRatio * m_scaleFactor; // 轨道宽度(1200mm)转换为像素
    float outerTrackOffset = trackOffset + trackWidthInPixels;               // 外轨道偏移量 = 内轨道偏移量 + 轨道宽度

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
            m_centerPoints.push_back(sf::Vector2f(
                cx + r * std::cos(angle),
                cy + r * std::sin(angle)));
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
                sf::Vector2f normal(-dir1.y, dir1.x);
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
                sf::Vector2f normal(-avgDir.y, avgDir.x);

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
    generateTrack(m_centerPoints, trackOffset, m_innersTrack, m_straightColor);

    // 生成外轨道线 - 使用内轨道偏移量加上轨道宽度
    generateTrack(m_centerPoints, outerTrackOffset, m_outerTrack, m_straightColor);
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
    draw(target, states);
}

void TrackRenderer::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    // 先绘制内轨道，再绘制外轨道
    target.draw(m_innersTrack, states);
    target.draw(m_outerTrack, states);
}