#include <iostream>
#include <SFML/Graphics.hpp>
#include "gui/TrackRenderer.hpp"

int main()
{
    // 创建TrackRenderer并生成几何体
    TrackRenderer trackRenderer;
    trackRenderer.setMmToPxRatio(0.01f);
    trackRenderer.setScaleFactor(0.5f);
    trackRenderer.generateGeometry(trackRenderer.getTrackLength(), trackRenderer.getCurveRadius());

    // 测试前几个距离点的位置
    std::cout << "=== 验证轨道点生成修复 ===" << std::endl;
    std::cout << "轨道参数:" << std::endl;
    std::cout << "轨道长度: " << trackRenderer.getTrackLength() << "mm" << std::endl;
    std::cout << "弯道半径: " << trackRenderer.getCurveRadius() << "mm" << std::endl;
    std::cout << std::endl;

    // 测试关键距离点
    float testDistances[] = {0.0f, 1000.0f, 5000.0f, 10000.0f};
    sf::Vector2f worldOriginOffsetPx(0, 0);

    for (float distance : testDistances)
    {
        sf::Vector2f position;
        float angle;

        if (trackRenderer.getPointAndOrientationOnCenterLine(distance, position, angle, worldOriginOffsetPx))
        {
            // 转换为渲染坐标系
            sf::Vector2f renderPos = trackRenderer.backendToRenderTransform(position);

            std::cout << "距离 " << distance << "mm:" << std::endl;
            std::cout << "  后端坐标: (" << position.x << ", " << position.y << ")" << std::endl;
            std::cout << "  渲染坐标: (" << renderPos.x << ", " << renderPos.y << ")" << std::endl;
            std::cout << "  角度: " << angle * 180.0f / 3.14159f << "度" << std::endl;

            // 判断位置
            if (renderPos.x < -50)
            {
                std::cout << "  位置: 左侧 ✓" << std::endl;
            }
            else if (renderPos.x > 50)
            {
                std::cout << "  位置: 右侧" << std::endl;
            }
            else
            {
                std::cout << "  位置: 中间" << std::endl;
            }
            std::cout << std::endl;
        }
        else
        {
            std::cout << "无法获取距离 " << distance << "mm 的位置信息" << std::endl;
        }
    }

    std::cout << "=== 结论 ===" << std::endl;
    std::cout << "如果距离0mm显示在左侧，则修复成功！" << std::endl;

    return 0;
}
