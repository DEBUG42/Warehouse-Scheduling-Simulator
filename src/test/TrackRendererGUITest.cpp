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
    trackRenderer.setScaleFactor(0.25f); // 原为0.01f, 调整以使初始轨道显示更大
    trackRenderer.setMmToPxRatio(0.04f); // 调整毫米到像素比例

    // !!! 重要：先设置轨道宽度，再生成几何形状 !!!
    trackRenderer.setTrackWidth(1200.0f); // 设置为实际宽度1200毫米

    // 生成轨道几何形状 - 使用参考图片中的真实尺寸
    trackRenderer.generateGeometry(40000.0f, 2500.0f); // 直轨长度和弯道半径，单位：毫米

    // 设置轨道颜色 (可以在generateGeometry之后，因为它不影响几何形状)
    trackRenderer.setStraightColor(sf::Color(100, 100, 100)); // 轨道颜色 - 略深一点

    // 设置初始视图
    sf::View view = window.getDefaultView();
    view.setCenter(0, 0); // 轨道中心为原点
    view.zoom(0.8f);      // 调整缩放以使初始轨道显示更大 (原为1.5f)
    window.setView(view);

    float zoomLevel = 1.0f; // 这个 zoomLevel 变量似乎是用于鼠标滚轮的相对缩放，初始设为1.0f是合理的
    bool isDragging = false;
    sf::Vector2f dragStart;
    bool showGrid = false; // 控制是否显示网格

    // 显示说明
    std::cout << "使用说明:\n";
    std::cout << "- 鼠标左键拖动: 移动视图\n";
    std::cout << "- 鼠标滚轮: 缩放视图\n";
    std::cout << "- 空格键: 重置视图\n";
    std::cout << "- T键: 切换显示坐标网格\n";
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
                view.zoom(1.5f); // 重置时也应用初始缩放
                zoomLevel = 1.0f;
                window.setView(view);
                std::cout << "视图已重置\n";
            }

            // 按T键切换网格显示
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::T)
            {
                showGrid = !showGrid;
                std::cout << (showGrid ? "显示坐标网格\n" : "隐藏坐标网格\n");
            }
        }

        window.clear(sf::Color(230, 240, 230)); // 使用浅绿色背景，类似图片

        // 如果开启了网格显示，绘制坐标网格
        if (showGrid)
        {
            // 获取当前视图的可见区域
            sf::Vector2f viewCenter = view.getCenter();
            sf::Vector2f viewSize = view.getSize();
            float gridSpacing = 500.0f * trackRenderer.getMmToPxRatio() * trackRenderer.getScaleFactor(); // 网格间距，对应500mm

            // 计算视图边界
            float viewLeft = viewCenter.x - viewSize.x / 2;
            float viewRight = viewCenter.x + viewSize.x / 2;
            float viewTop = viewCenter.y - viewSize.y / 2;
            float viewBottom = viewCenter.y + viewSize.y / 2;

            // 计算网格起始和结束的线数（稍微扩大一点范围确保覆盖整个视图）
            int startX = static_cast<int>(viewLeft / gridSpacing) - 1;
            int endX = static_cast<int>(viewRight / gridSpacing) + 1;
            int startY = static_cast<int>(viewTop / gridSpacing) - 1;
            int endY = static_cast<int>(viewBottom / gridSpacing) + 1;

            // 绘制垂直网格线
            for (int i = startX; i <= endX; ++i)
            {
                float x = i * gridSpacing;
                sf::Vertex line[] = {
                    sf::Vertex(sf::Vector2f(x, viewTop), sf::Color(200, 200, 200, 100)),
                    sf::Vertex(sf::Vector2f(x, viewBottom), sf::Color(200, 200, 200, 100))};
                window.draw(line, 2, sf::Lines);
            }

            // 绘制水平网格线
            for (int i = startY; i <= endY; ++i)
            {
                float y = i * gridSpacing;
                sf::Vertex line[] = {
                    sf::Vertex(sf::Vector2f(viewLeft, y), sf::Color(200, 200, 200, 100)),
                    sf::Vertex(sf::Vector2f(viewRight, y), sf::Color(200, 200, 200, 100))};
                window.draw(line, 2, sf::Lines);
            }

            // 绘制原点十字线
            sf::Vertex originCrossH[] = {
                sf::Vertex(sf::Vector2f(-5000, 0), sf::Color(255, 0, 0, 150)),
                sf::Vertex(sf::Vector2f(5000, 0), sf::Color(255, 0, 0, 150))};
            window.draw(originCrossH, 2, sf::Lines);

            sf::Vertex originCrossV[] = {
                sf::Vertex(sf::Vector2f(0, -5000), sf::Color(255, 0, 0, 150)),
                sf::Vertex(sf::Vector2f(0, 5000), sf::Color(255, 0, 0, 150))};
            window.draw(originCrossV, 2, sf::Lines);

            /*
            // 在轨道中心点处绘制加粗的点
            const std::vector<sf::Vector2f>& centerPoints = trackRenderer.getCenterPoints();
            for (const auto& point : centerPoints)
            {
                // 绘制加粗点
                sf::CircleShape centerPointDot(2.0f);
                centerPointDot.setFillColor(sf::Color(0, 0, 255, 180));
                centerPointDot.setOrigin(2.0f, 2.0f);
                centerPointDot.setPosition(point);
                window.draw(centerPointDot);
            }
            */
        }

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