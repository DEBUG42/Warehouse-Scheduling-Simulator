#include <SFML/Graphics.hpp>
#include "gui/TrackRenderer.hpp"
#include "gui/VehicleRenderer.hpp"
#include "gui/SimObject.hpp"
#include <iostream>
#include <string>
#include <cmath>
#include <cstdio>

// Helper to draw text
void drawText(sf::RenderWindow &window, const std::string &str, sf::Vector2f pos, const sf::Font &font, unsigned int size = 15, sf::Color color = sf::Color::White)
{
    sf::Text text;
    text.setFont(font);
    text.setString(str);
    text.setCharacterSize(size);
    text.setFillColor(color);
    text.setPosition(pos);
    window.draw(text);
}

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
    renderLabel.setPosition(renderOriginDebug.getPosition().x + 10.f, renderOriginDebug.getPosition().y - 20.f);
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

    // 渲染坐标系的Y轴正方向 (绿色箭头)
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
    backendLabel.setPosition(backendOriginInRenderCoords.x + 10.f, backendOriginInRenderCoords.y - 20.f);
    window.draw(backendLabel);
}

int main()
{
    // 使用默认窗口大小，因为 TrackRenderer 会自动处理缩放
    sf::RenderWindow window(sf::VideoMode(1280, 720), "Vehicle Path Position Test");
    window.setFramerateLimit(60); // 加载字体
    sf::Font font;
    if (!font.loadFromFile("../../assets/fonts/arial.ttf"))
    {
        std::cerr << "Error loading font ../../assets/fonts/arial.ttf" << std::endl;
        if (!font.loadFromFile("assets/fonts/arial.ttf"))
        {
            std::cerr << "Error loading font assets/fonts/arial.ttf" << std::endl;
            if (!font.loadFromFile("../assets/fonts/arial.ttf"))
            {
                std::cerr << "Error loading font ../assets/fonts/arial.ttf" << std::endl;
                return -1;
            }
        }
    }

    // 使用默认构造函数创建 TrackRenderer，它会使用预设的参数
    TrackRenderer trackRenderer;
    // 设置合适的毫米到像素的比例
    trackRenderer.setMmToPxRatio(0.01f); // 使用更小的比例让轨道和车辆看起来更小
    trackRenderer.setScaleFactor(0.5f);  // 额外的缩放因子
    // 为了能显示轨道，需要生成几何形状
    trackRenderer.generateGeometry(trackRenderer.getTrackLength(), trackRenderer.getCurveRadius());
    // 创建车辆渲染器
    VehicleRenderer vehicleRenderer(font);
    // 重要：同步VehicleRenderer的缩放参数与TrackRenderer
    vehicleRenderer.setMmToPxRatio(trackRenderer.getMmToPxRatio());

    // 创建测试用车辆状态，提供所有必需的参数
    gui::VehicleState vehicleState(
        "VTest1",                 // ID
        sf::Vector2f(0, 0),       // 初始位置
        0.0f,                     // 初始轨道位置
        0.0f,                     // 初始速度
        gui::VehicleStatus::IDLE, // 初始状态
        "",                       // 无任务
        false,                    // 未载货
        gui::CargoDisplayInfo(),  // 默认货物信息
        1.0f                      // 满电量
    );

    float currentPathDistanceMm = 0.0f;
    float pathDistanceStep = 20.0f;
    float totalPathLength = trackRenderer.getTotalCenterLineLengthMm();

    // 设置默认视图，使轨道位于窗口中心，并调整缩放
    sf::View view = window.getDefaultView();
    view.setCenter(0, 0);
    view.zoom(2.0f);      // 增大缩放系数以显示更大的整体视图
    window.setView(view); // 输出测试信息
    std::cout << "TrackRenderer initialized with default parameters:" << std::endl;
    std::cout << "Track Length: " << trackRenderer.getTrackLength() << "mm" << std::endl;
    std::cout << "Inner Curve Radius: " << trackRenderer.getCurveRadius() << "mm" << std::endl;
    std::cout << "Track Width: " << trackRenderer.getTrackWidth() << "mm" << std::endl;
    std::cout << "Total center line length: " << totalPathLength << "mm" << std::endl;
    std::cout << "\nControls:" << std::endl;
    std::cout << "- RIGHT/LEFT: Move vehicle forward/backward" << std::endl;
    std::cout << "- R: Reset to start position" << std::endl;
    std::cout << "- D: Toggle debug coordinate system" << std::endl;
    std::cout << "- C: Toggle debug info" << std::endl;
    std::cout << "- T: Toggle grid display" << std::endl;
    std::cout << "- SPACE: Reset view" << std::endl;
    std::cout << "- Mouse drag: Pan view" << std::endl;
    std::cout << "- Mouse wheel: Zoom" << std::endl;

    // 创建路径原点标记
    sf::Vector2f pathOriginPx;
    float angleRad = 0.0f;
    sf::Vector2f worldOriginOffsetPx(0, 0);
    trackRenderer.getPointAndOrientationOnCenterLine(0.0f, pathOriginPx, angleRad, worldOriginOffsetPx);
    sf::Vector2f pathOriginRenderCoords = trackRenderer.backendToRenderTransform(pathOriginPx);

    sf::CircleShape pathOriginMarker(6.f);
    pathOriginMarker.setFillColor(sf::Color::Yellow);
    pathOriginMarker.setOrigin(pathOriginMarker.getRadius(), pathOriginMarker.getRadius());
    pathOriginMarker.setPosition(pathOriginRenderCoords);

    // UI控制变量
    bool showDebugInfo = true;
    bool showDebugCoords = true;
    bool showGrid = false;
    float zoomLevel = 1.0f;
    bool isDragging = false;
    sf::Vector2f dragStart;
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

            if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::Right)
                {
                    currentPathDistanceMm += pathDistanceStep;
                    if (currentPathDistanceMm > totalPathLength)
                    {
                        currentPathDistanceMm = std::fmod(currentPathDistanceMm, totalPathLength);
                    }
                }
                else if (event.key.code == sf::Keyboard::Left)
                {
                    currentPathDistanceMm -= pathDistanceStep;
                    if (currentPathDistanceMm < 0)
                    {
                        currentPathDistanceMm += totalPathLength;
                    }
                }
                else if (event.key.code == sf::Keyboard::R)
                {
                    currentPathDistanceMm = 0.0f;
                }
                else if (event.key.code == sf::Keyboard::D)
                {
                    showDebugCoords = !showDebugCoords;
                    std::cout << (showDebugCoords ? "显示调试坐标系信息\n" : "隐藏调试坐标系信息\n");
                }
                else if (event.key.code == sf::Keyboard::C)
                {
                    showDebugInfo = !showDebugInfo;
                    std::cout << (showDebugInfo ? "显示调试信息\n" : "隐藏调试信息\n");
                }
                else if (event.key.code == sf::Keyboard::T)
                {
                    showGrid = !showGrid;
                    std::cout << (showGrid ? "显示坐标网格\n" : "隐藏坐标网格\n");
                }
                else if (event.key.code == sf::Keyboard::Space)
                {
                    view = window.getDefaultView();
                    view.setCenter(0, 0);
                    view.zoom(2.0f);
                    zoomLevel = 1.0f;
                    window.setView(view);
                    std::cout << "视图已重置\n";
                }
            }
        }

        // 更新车辆状态
        vehicleState.rawTrackPositionMm = currentPathDistanceMm;

        // 获取当前位置和方向
        sf::Vector2f currentPosPx;
        float angleDegrees = 0.0f;
        trackRenderer.getPointAndOrientationOnCenterLine(currentPathDistanceMm, currentPosPx, angleDegrees, worldOriginOffsetPx);
        vehicleState.position = currentPosPx;

        // 更新车辆渲染器状态
        vehicleRenderer.updateState(vehicleState, trackRenderer, worldOriginOffsetPx);
        window.clear(sf::Color(230, 240, 230)); // 使用浅绿色背景

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

            // 计算网格起始和结束的线数
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
        }

        // 绘制轨道
        window.draw(trackRenderer);

        // 绘制车辆
        window.draw(vehicleRenderer);

        // 绘制路径原点标记
        window.draw(pathOriginMarker);

        // 绘制调试坐标系信息
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
        } // 显示调试信息
        if (showDebugInfo)
        {
            sf::Vector2f renderPosition = trackRenderer.backendToRenderTransform(vehicleState.position);
            char buffer[256];
            snprintf(buffer, sizeof(buffer),
                     "Path Distance: %.1f / %.1f mm\n"
                     "Backend Position: (%.1f, %.1f)\n"
                     "Backend Orientation: %.1f deg\n"
                     "VehicleRenderer mmToPx: %.3f\n"
                     "TrackRenderer mmToPx: %.3f",
                     currentPathDistanceMm, totalPathLength,
                     vehicleState.position.x, vehicleState.position.y,
                     angleDegrees,
                     vehicleRenderer.getMmToPxRatio(),
                     trackRenderer.getMmToPxRatio());

            drawText(window, buffer, sf::Vector2f(10, 100), font, 14, sf::Color::Black);
            drawText(window, "Path Origin (Dist=0)",
                     pathOriginRenderCoords + sf::Vector2f(10, -10),
                     font, 12, sf::Color::Yellow);

            // 控制说明
            std::string controls = "Controls:\n"
                                   "C: Toggle Debug Info\n"
                                   "D: Toggle Debug Coords\n"
                                   "T: Toggle Grid\n"
                                   "Space: Reset View";
            drawText(window, controls,
                     sf::Vector2f(10, window.getSize().y - 100),
                     font, 12, sf::Color::Cyan);
        }

        window.display();
    }

    return 0;
}
