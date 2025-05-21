#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include "../../include/gui/TrackRenderer.hpp"

int main()
{
    sf::RenderWindow window(sf::VideoMode(1280, 720), "TrackRenderer GUI 测试");
    window.setFramerateLimit(60);

    // 创建轨道渲染器
    TrackRenderer trackRenderer;

    // 设置缩放系数，使轨道适合图片中的形状
    trackRenderer.setScaleFactor(0.015f); // 降低缩放因子，使轨道看起来更小
    trackRenderer.setMmToPxRatio(0.05f);

    // 生成轨道几何形状 - 使用参考图片中的真实尺寸
    trackRenderer.generateGeometry(40000.0f, 2500.0f); // 直轨长度和弯道半径，单位：毫米

    // 设置轨道宽度和颜色
    trackRenderer.setTrackWidth(1200.0f); // 设置为实际宽度1200毫米
    trackRenderer.setStraightColor(sf::Color(100, 100, 100));
    trackRenderer.setCurveColor(sf::Color(100, 100, 100));

    // 设置初始视图
    sf::View view = window.getDefaultView();
    view.setCenter(0, 0); // 轨道中心为原点
    window.setView(view);

    float zoomLevel = 1.0f; // 初始缩放比例
    bool isDragging = false;
    sf::Vector2f dragStart;

    // 显示说明
    std::cout << "使用说明:\n";
    std::cout << "- 鼠标左键拖动: 移动视图\n";
    std::cout << "- 鼠标滚轮: 缩放视图\n";
    std::cout << "- 轨道参数: 长度=40000mm, 弯道半径=2500mm, 宽度=1200mm\n";

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            // 处理鼠标滚轮事件
            if (event.type == sf::Event::MouseWheelScrolled)
            {
                if (event.mouseWheelScroll.delta < 0)
                {
                    // 缩小 (放大视图)
                    zoomLevel *= 1.1f;
                    view.zoom(1.1f);
                }
                else
                {
                    // 放大 (缩小视图)
                    zoomLevel *= 0.9f;
                    view.zoom(0.9f);
                }
                window.setView(view);
            }

            // 处理鼠标左键按下事件
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
            {
                isDragging = true;
                dragStart = window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
            }

            // 处理鼠标左键释放事件
            if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left)
            {
                isDragging = false;
            }

            // 处理鼠标移动事件
            if (event.type == sf::Event::MouseMoved && isDragging)
            {
                sf::Vector2f currentPos = window.mapPixelToCoords(sf::Vector2i(event.mouseMove.x, event.mouseMove.y));
                sf::Vector2f delta = dragStart - currentPos;

                // 移动视图
                view.move(delta);
                window.setView(view);

                // 更新拖动起始点
                dragStart = window.mapPixelToCoords(sf::Vector2i(event.mouseMove.x, event.mouseMove.y));
            }

            // 按空格键重置视图
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space)
            {
                view = window.getDefaultView();
                view.setCenter(0, 0);
                zoomLevel = 1.0f;
                window.setView(view);
                std::cout << "视图已重置\n";
            }
        }

        window.clear(sf::Color(230, 240, 230)); // 使用浅绿色背景，类似图片

        // 绘制轨道
        window.draw(trackRenderer);

        // 绘制拖动指示器
        if (isDragging)
        {
            sf::CircleShape dragIndicator(5);
            dragIndicator.setFillColor(sf::Color::Green);
            dragIndicator.setOrigin(5, 5);
            dragIndicator.setPosition(dragStart);
            window.draw(dragIndicator);
        }

        window.display();
    }
    return 0;
}