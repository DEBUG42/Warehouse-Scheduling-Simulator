#include <iostream>
#include <cmath>
#include <vector>
#include <SFML/Graphics.hpp>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

int main()
{
    // 模拟轨道参数
    float trackLength = 40000.0f; // mm
    float curveRadius = 2500.0f;  // mm (inner radius)
    float trackWidth = 1200.0f;   // mm
    float mmToPxRatio = 0.01f;
    float scaleFactor = 0.5f;

    // 计算像素值
    float scaledTrackLength = trackLength * mmToPxRatio * scaleFactor;
    float scaledInnerCurveRadius = curveRadius * mmToPxRatio * scaleFactor;
    float scaledFullTrackWidth = trackWidth * mmToPxRatio * scaleFactor;
    float scaledCenterLineCurveRadius = scaledInnerCurveRadius + (scaledFullTrackWidth / 2.0f);

    std::cout << "轨道参数:" << std::endl;
    std::cout << "轨道长度: " << trackLength << "mm (" << scaledTrackLength << "px)" << std::endl;
    std::cout << "内弯半径: " << curveRadius << "mm (" << scaledInnerCurveRadius << "px)" << std::endl;
    std::cout << "中心线弯半径: " << scaledCenterLineCurveRadius << "px" << std::endl;
    std::cout << std::endl;

    // 弯道中心
    float rightCurveX = scaledTrackLength / 2;
    float rightCurveY = 0.0f;
    float leftCurveX = -scaledTrackLength / 2;
    float leftCurveY = 0.0f;

    std::cout << "弯道中心:" << std::endl;
    std::cout << "右弯道中心: (" << rightCurveX << ", " << rightCurveY << ")" << std::endl;
    std::cout << "左弯道中心: (" << leftCurveX << ", " << leftCurveY << ")" << std::endl;
    std::cout << std::endl;

    // 计算关键点位置
    std::cout << "右弯道关键点 (半径=" << scaledCenterLineCurveRadius << "):" << std::endl;

    // -90度 (右下角)
    float angle_neg90 = -M_PI / 2;
    float x_neg90 = rightCurveX + scaledCenterLineCurveRadius * std::cos(angle_neg90);
    float y_neg90 = rightCurveY - scaledCenterLineCurveRadius * std::sin(angle_neg90);
    std::cout << "-90度 (起始点): (" << x_neg90 << ", " << y_neg90 << ")" << std::endl;

    // 0度 (右侧)
    float angle_0 = 0;
    float x_0 = rightCurveX + scaledCenterLineCurveRadius * std::cos(angle_0);
    float y_0 = rightCurveY - scaledCenterLineCurveRadius * std::sin(angle_0);
    std::cout << "0度 (右侧): (" << x_0 << ", " << y_0 << ")" << std::endl;

    // 90度 (右上角)
    float angle_90 = M_PI / 2;
    float x_90 = rightCurveX + scaledCenterLineCurveRadius * std::cos(angle_90);
    float y_90 = rightCurveY - scaledCenterLineCurveRadius * std::sin(angle_90);
    std::cout << "90度 (右上角): (" << x_90 << ", " << y_90 << ")" << std::endl;

    std::cout << std::endl;
    std::cout << "左弯道关键点:" << std::endl;

    // 90度 (左上角)
    float left_x_90 = leftCurveX + scaledCenterLineCurveRadius * std::cos(angle_90);
    float left_y_90 = leftCurveY - scaledCenterLineCurveRadius * std::sin(angle_90);
    std::cout << "90度 (左上角): (" << left_x_90 << ", " << left_y_90 << ")" << std::endl;

    // 180度 (左侧)
    float angle_180 = M_PI;
    float left_x_180 = leftCurveX + scaledCenterLineCurveRadius * std::cos(angle_180);
    float left_y_180 = leftCurveY - scaledCenterLineCurveRadius * std::sin(angle_180);
    std::cout << "180度 (左侧): (" << left_x_180 << ", " << left_y_180 << ")" << std::endl;

    // 270度 (左下角)
    float angle_270 = 3 * M_PI / 2;
    float left_x_270 = leftCurveX + scaledCenterLineCurveRadius * std::cos(angle_270);
    float left_y_270 = leftCurveY - scaledCenterLineCurveRadius * std::sin(angle_270);
    std::cout << "270度 (左下角): (" << left_x_270 << ", " << left_y_270 << ")" << std::endl;

    std::cout << std::endl;
    std::cout << "结论:" << std::endl;
    std::cout << "如果车辆应该从左侧开始，起始点应该是: (" << left_x_180 << ", " << left_y_180 << ")" << std::endl;
    std::cout << "但当前起始点是: (" << x_neg90 << ", " << y_neg90 << ")" << std::endl;

    return 0;
}
