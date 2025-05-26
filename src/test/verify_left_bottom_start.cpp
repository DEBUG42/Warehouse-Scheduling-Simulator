#include <iostream>
#include <SFML/Graphics.hpp>
#include "gui/TrackRenderer.hpp"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

int main()
{
    // 创建TrackRenderer并生成几何体
    TrackRenderer trackRenderer;
    trackRenderer.setMmToPxRatio(0.01f);
    trackRenderer.setScaleFactor(0.5f);
    trackRenderer.generateGeometry(trackRenderer.getTrackLength(), trackRenderer.getCurveRadius());

    std::cout << "=== 验证车辆起始位置修复 ===" << std::endl;
    std::cout << "轨道参数:" << std::endl;
    std::cout << "轨道长度: " << trackRenderer.getTrackLength() << "mm" << std::endl;
    std::cout << "弯道半径: " << trackRenderer.getCurveRadius() << "mm" << std::endl;
    std::cout << "轨道宽度: " << trackRenderer.getTrackWidth() << "mm" << std::endl;
    std::cout << std::endl;

    // 计算理论上的左下角位置（270°）
    float trackLength = trackRenderer.getTrackLength();
    float curveRadius = trackRenderer.getCurveRadius();
    float trackWidth = trackRenderer.getTrackWidth();
    float mmToPxRatio = trackRenderer.getMmToPxRatio();
    float scaleFactor = trackRenderer.getScaleFactor();

    float scaledTrackLength = trackLength * mmToPxRatio * scaleFactor;
    float scaledInnerCurveRadius = curveRadius * mmToPxRatio * scaleFactor;
    float scaledFullTrackWidth = trackWidth * mmToPxRatio * scaleFactor;
    float scaledCenterLineCurveRadius = scaledInnerCurveRadius + (scaledFullTrackWidth / 2.0f);

    float leftCurveX = -scaledTrackLength / 2;
    float leftCurveY = 0.0f;

    // 理论左下角位置（270°）
    float theoreticalLeftBottomX = leftCurveX + scaledCenterLineCurveRadius * std::cos(3 * M_PI / 2);
    float theoreticalLeftBottomY = leftCurveY - scaledCenterLineCurveRadius * std::sin(3 * M_PI / 2);

    std::cout << "理论左下角位置（270°）:" << std::endl;
    std::cout << "后端坐标: (" << theoreticalLeftBottomX / (mmToPxRatio * scaleFactor) << ", "
              << theoreticalLeftBottomY / (mmToPxRatio * scaleFactor) << ")mm" << std::endl;
    std::cout << "像素坐标: (" << theoreticalLeftBottomX << ", " << theoreticalLeftBottomY << ")px" << std::endl;
    std::cout << std::endl;

    // 测试实际的距离0位置
    sf::Vector2f worldOriginOffsetPx(0, 0);
    sf::Vector2f actualPosition;
    float actualAngle;

    if (trackRenderer.getPointAndOrientationOnCenterLine(0.0f, actualPosition, actualAngle, worldOriginOffsetPx))
    {
        sf::Vector2f renderPos = trackRenderer.backendToRenderTransform(actualPosition);

        std::cout << "实际距离0位置:" << std::endl;
        std::cout << "后端坐标: (" << actualPosition.x << ", " << actualPosition.y << ")mm" << std::endl;
        std::cout << "渲染坐标: (" << renderPos.x << ", " << renderPos.y << ")px" << std::endl;
        std::cout << "角度: " << actualAngle * 180.0f / M_PI << "度" << std::endl;
        std::cout << std::endl;

        // 计算差异
        float diffX = actualPosition.x - (theoreticalLeftBottomX / (mmToPxRatio * scaleFactor));
        float diffY = actualPosition.y - (theoreticalLeftBottomY / (mmToPxRatio * scaleFactor));
        float distance = std::sqrt(diffX * diffX + diffY * diffY);

        std::cout << "位置差异:" << std::endl;
        std::cout << "X差异: " << diffX << "mm" << std::endl;
        std::cout << "Y差异: " << diffY << "mm" << std::endl;
        std::cout << "总距离差异: " << distance << "mm" << std::endl;
        std::cout << std::endl;

        if (distance < 10.0f)
        { // 允许10mm的误差
            std::cout << "✅ 修复成功！车辆现在从左下角开始。" << std::endl;
        }
        else
        {
            std::cout << "❌ 位置仍有较大偏差。" << std::endl;
        }

        // 判断方位
        if (actualPosition.x < -trackLength / 4 && actualPosition.y > trackLength / 4)
        {
            std::cout << "✅ 位置确认：车辆在左下角区域。" << std::endl;
        }
        else
        {
            std::cout << "❌ 位置不在左下角区域。" << std::endl;
        }
    }
    else
    {
        std::cout << "❌ 无法获取距离0位置信息" << std::endl;
    }

    return 0;
}
