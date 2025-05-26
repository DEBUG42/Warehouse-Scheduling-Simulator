#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include <vector> // Required for std::vector
#include "../../include/gui/TrackRenderer.hpp"

// 新增调试函数：绘制坐标系原点和轴向指示
void drawCoordinateSystemDebug(sf::RenderWindow &window, TrackRenderer &trackRenderer, const sf::View &view, sf::Font &font)
{
    const float axisLengthMm = 100.f;  // 轴指示线的长度（毫米）
    const float arrowHeadSizePx = 8.f; // 箭头大小（像素）

    // --- 轨道参数显示 ---
    float trackL = trackRenderer.getTrackLength();
    float curveR = trackRenderer.getCurveRadius();
    std::string trackInfoStr = "Track Length: " + std::to_string(static_cast<int>(trackL)) + "mm\\n" +
                               "Curve Radius: " + std::to_string(static_cast<int>(curveR)) + "mm";
    sf::Text trackInfoText(trackInfoStr, font, 12);
    trackInfoText.setFillColor(sf::Color::Black);
    // 将文本定位在视图的左上角，需要将视图坐标转换为世界坐标
    sf::Vector2f viewTopLeft = window.mapPixelToCoords(sf::Vector2i(10, 10), view);
    trackInfoText.setPosition(viewTopLeft);
    window.draw(trackInfoText);

    // --- 渲染坐标系 (轨道中心为原点) ---
    sf::CircleShape renderOriginDebug(5.f);
    renderOriginDebug.setFillColor(sf::Color::Cyan);
    renderOriginDebug.setOrigin(5.f, 5.f);
    renderOriginDebug.setPosition(0.f, 0.f); // 世界坐标的(0,0)
    window.draw(renderOriginDebug);

    sf::Text renderLabel("Render Origin (0,0)", font, 12);
    renderLabel.setFillColor(sf::Color::Cyan);
    renderLabel.setPosition(renderOriginDebug.getPosition().x + 10.f, renderOriginDebug.getPosition().y - 20.f); // 调整标签位置
    window.draw(renderLabel);

    float renderAxisDisplayLengthPx = axisLengthMm * trackRenderer.getMmToPxRatio() * trackRenderer.getScaleFactor();

    // 渲染坐标系的X轴正方向 (红色箭头)
    sf::Vector2f r_origin(0.f, 0.f);
    sf::Vector2f r_xEnd(renderAxisDisplayLengthPx, 0.f);
    sf::Vertex renderXAxis[] = {
        sf::Vertex(r_origin, sf::Color::Red),
        sf::Vertex(r_xEnd, sf::Color::Red)};
    window.draw(renderXAxis, 2, sf::Lines);
    sf::Vertex renderXArrow[] = {
        sf::Vertex(r_xEnd, sf::Color::Red),
        sf::Vertex(sf::Vector2f(r_xEnd.x - arrowHeadSizePx, r_xEnd.y - arrowHeadSizePx / 2.f), sf::Color::Red),
        sf::Vertex(r_xEnd, sf::Color::Red),
        sf::Vertex(sf::Vector2f(r_xEnd.x - arrowHeadSizePx, r_xEnd.y + arrowHeadSizePx / 2.f), sf::Color::Red)};
    window.draw(renderXArrow, 4, sf::Lines);
    sf::Text renderXLabel("R_X+", font, 10);
    renderXLabel.setFillColor(sf::Color::Red);
    renderXLabel.setPosition(100.f * trackRenderer.getMmToPxRatio() * trackRenderer.getScaleFactor() + 5.f, -5.f);
    window.draw(renderXLabel);

    // 渲染坐标系的Y轴正方向 (绿色箭头) - SFML默认Y轴向下为正
    sf::Vector2f r_yEnd(0.f, renderAxisDisplayLengthPx);
    sf::Vertex renderYAxis[] = {
        sf::Vertex(r_origin, sf::Color::Green),
        sf::Vertex(r_yEnd, sf::Color::Green)};
    window.draw(renderYAxis, 2, sf::Lines);
    sf::Vertex renderYArrow[] = {
        sf::Vertex(r_yEnd, sf::Color::Green),
        sf::Vertex(sf::Vector2f(r_yEnd.x - arrowHeadSizePx / 2.f, r_yEnd.y - arrowHeadSizePx), sf::Color::Green),
        sf::Vertex(r_yEnd, sf::Color::Green),
        sf::Vertex(sf::Vector2f(r_yEnd.x + arrowHeadSizePx / 2.f, r_yEnd.y - arrowHeadSizePx), sf::Color::Green)};
    window.draw(renderYArrow, 4, sf::Lines);
    sf::Text renderYLabel("R_Y+", font, 10);
    renderYLabel.setFillColor(sf::Color::Green);
    renderYLabel.setPosition(5.f, 100.f * trackRenderer.getMmToPxRatio() * trackRenderer.getScaleFactor() + 5.f);
    window.draw(renderYLabel);

    // --- 后端坐标系 (左下角弯道与直道交汇点为原点) ---
    sf::Vector2f backendOriginInRenderCoords = trackRenderer.backendToRenderTransform(sf::Vector2f(0.f, 0.f));

    sf::CircleShape backendOriginDebug(5.f);
    backendOriginDebug.setFillColor(sf::Color::Magenta);
    backendOriginDebug.setOrigin(5.f, 5.f);
    backendOriginDebug.setPosition(backendOriginInRenderCoords);
    window.draw(backendOriginDebug);

    sf::Text backendLabel("Backend Origin (0,0)mm", font, 12);
    backendLabel.setFillColor(sf::Color::Magenta);
    backendLabel.setPosition(backendOriginInRenderCoords.x + 10.f, backendOriginInRenderCoords.y - 20.f); // 调整标签位置
    window.draw(backendLabel);

    // 后端坐标系的X轴正方向 (橙色箭头)
    sf::Vector2f backendXStartRender = backendOriginInRenderCoords;
    sf::Vector2f backendXEndRender = trackRenderer.backendToRenderTransform(sf::Vector2f(axisLengthMm, 0.f));
    sf::Vertex backendXAxis[] = {
        sf::Vertex(backendXStartRender, sf::Color(255, 165, 0)), // Orange
        sf::Vertex(backendXEndRender, sf::Color(255, 165, 0))};
    window.draw(backendXAxis, 2, sf::Lines);
    // Arrowhead for backend X (points from start to end)
    sf::Vector2f dirBX = backendXEndRender - backendXStartRender;
    float lenBX = std::sqrt(dirBX.x * dirBX.x + dirBX.y * dirBX.y);
    sf::Vector2f unitDirBX = (lenBX > 0) ? dirBX / lenBX : sf::Vector2f(0, 0);
    sf::Vector2f normalBX(-unitDirBX.y, unitDirBX.x); // Perpendicular

    sf::Vertex backendXArrow[] = {
        sf::Vertex(backendXEndRender, sf::Color(255, 165, 0)),
        sf::Vertex(backendXEndRender - unitDirBX * arrowHeadSizePx + normalBX * (arrowHeadSizePx / 2.f), sf::Color(255, 165, 0)),
        sf::Vertex(backendXEndRender, sf::Color(255, 165, 0)),
        sf::Vertex(backendXEndRender - unitDirBX * arrowHeadSizePx - normalBX * (arrowHeadSizePx / 2.f), sf::Color(255, 165, 0))};
    window.draw(backendXArrow, 4, sf::Lines);
    sf::Text backendXLabel("B_X+", font, 10);
    backendXLabel.setFillColor(sf::Color(255, 165, 0));
    backendXLabel.setPosition(backendXEndRender.x + 5.f, backendXEndRender.y - 5.f);
    window.draw(backendXLabel);

    // 后端坐标系的Y轴正方向 (黄色箭头)
    sf::Vector2f backendYStartRender = backendOriginInRenderCoords;
    sf::Vector2f backendYEndRender = trackRenderer.backendToRenderTransform(sf::Vector2f(0.f, axisLengthMm));
    sf::Vertex backendYAxis[] = {
        sf::Vertex(backendYStartRender, sf::Color::Yellow),
        sf::Vertex(backendYEndRender, sf::Color::Yellow)};
    window.draw(backendYAxis, 2, sf::Lines);
    // Arrowhead for backend Y (points from start to end)
    sf::Vector2f dirBY = backendYEndRender - backendYStartRender;
    float lenBY = std::sqrt(dirBY.x * dirBY.x + dirBY.y * dirBY.y);
    sf::Vector2f unitDirBY = (lenBY > 0) ? dirBY / lenBY : sf::Vector2f(0, 0);
    sf::Vector2f normalBY(-unitDirBY.y, unitDirBY.x); // Perpendicular

    sf::Vertex backendYArrow[] = {
        sf::Vertex(backendYEndRender, sf::Color::Yellow),
        sf::Vertex(backendYEndRender - unitDirBY * arrowHeadSizePx + normalBY * (arrowHeadSizePx / 2.f), sf::Color::Yellow),
        sf::Vertex(backendYEndRender, sf::Color::Yellow),
        sf::Vertex(backendYEndRender - unitDirBY * arrowHeadSizePx - normalBY * (arrowHeadSizePx / 2.f), sf::Color::Yellow)};
    window.draw(backendYArrow, 4, sf::Lines);
    sf::Text backendYLabel("B_Y+", font, 10);
    backendYLabel.setFillColor(sf::Color::Yellow);
    backendYLabel.setPosition(backendYEndRender.x + 5.f, backendYEndRender.y + 5.f);
    window.draw(backendYLabel);
}

int main()
{
    sf::RenderWindow window(sf::VideoMode(1280, 720), "TrackRenderer GUI Test");
    window.setFramerateLimit(60);

    // 加载字体用于标签
    sf::Font font;
    if (!font.loadFromFile("D:/CodeMaster/GitHub/C++SFML/Warehouse-sch/assets/fonts/arial.ttf"))
    { // Ensure you have this font or change path
        std::cerr << "Error loading font for debug" << std::endl;
        return -1;
    }

    // 创建轨道渲染器
    TrackRenderer trackRenderer;

    // 设置缩放系数，使轨道适合图片中的形状
    trackRenderer.setScaleFactor(0.25f); // 调整缩放因子
    trackRenderer.setMmToPxRatio(0.04f); // 调整毫米到像素比例

    // 生成轨道几何形状 - 使用参考图片中的真实尺寸
    trackRenderer.generateGeometry(40000.0f, 2500.0f); // 直轨长度和弯道半径，单位：毫米

    // 设置轨道颜色 (可以在generateGeometry之后，因为它不影响几何形状)
    trackRenderer.setTrackColor(sf::Color(0, 0, 0)); // 轨道颜色 - 略深一点

    // 设置初始视图
    sf::View view = window.getDefaultView();
    view.setCenter(0, 0); // 轨道中心为原点
    view.zoom(1.0f);      // 初始缩小一点，显示整个轨道
    window.setView(view);

    float zoomLevel = 1.0f; // 初始缩放比例
    bool isDragging = false;
    sf::Vector2f dragStart;
    bool showGrid = false;       // 控制是否显示网格
    bool showDebugCoords = true; // 新增：控制是否显示调试坐标信息

    // 显示说明
    std::cout << "使用说明:\\n";
    std::cout << "- 鼠标左键拖动: 移动视图\\n";
    std::cout << "- 鼠标滚轮: 缩放视图\\n";
    std::cout << "- 空格键: 重置视图\\n";
    std::cout << "- T键: 切换显示坐标网格\\n";
    std::cout << "- D键: 切换显示调试坐标系信息\\n"; // 新增说明
    std::cout << "- 轨道参数: 长度=40000mm, 弯道半径=2500mm, 宽度=1200mm\\n";

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

            // 新增：按D键切换调试坐标系信息显示
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::D)
            {
                showDebugCoords = !showDebugCoords;
                std::cout << (showDebugCoords ? "显示调试坐标系信息\n" : "隐藏调试坐标系信息\n");
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

            // 在轨道中心点处绘制加粗的点
            const std::vector<sf::Vector2f> &centerPoints = trackRenderer.getCenterPoints();
            for (const auto &point : centerPoints)
            {
                // 绘制加粗点
                sf::CircleShape centerPointDot(2.0f);
                centerPointDot.setFillColor(sf::Color(0, 0, 255, 180));
                centerPointDot.setOrigin(2.0f, 2.0f);
                centerPointDot.setPosition(point);
                window.draw(centerPointDot);
            }

            const std::vector<sf::Vector2f> &backendCenterPoints = trackRenderer.getCenterPoints();
            for (const auto &backendPoint : backendCenterPoints)
            {
                sf::Vector2f renderPoint = trackRenderer.backendToRenderTransform(backendPoint);
                sf::CircleShape centerPointDot(2.0f);
                centerPointDot.setFillColor(sf::Color(0, 0, 255, 180));
                centerPointDot.setOrigin(2.0f, 2.0f);
                centerPointDot.setPosition(renderPoint); // Use transformed render coordinates
                window.draw(centerPointDot);
            }
        }

        // 绘制轨道
        window.draw(trackRenderer);

        // 新增：如果开启，绘制调试坐标系信息
        if (showDebugCoords)
        {
            drawCoordinateSystemDebug(window, trackRenderer, view, font);
        }

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