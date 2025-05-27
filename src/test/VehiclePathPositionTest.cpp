#include <SFML/Graphics.hpp>
#include "gui/TrackRenderer.hpp"
#include "gui/VehicleRenderer.hpp"
#include "gui/SimObject.hpp"
#include "gui/WarehouseRenderer.hpp" // 添加 WarehouseRenderer 的头文件
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
// 注意：drawCoordinateSystemDebug 的参数需要与 main 函数中调用时一致
void drawCoordinateSystemDebug(sf::RenderWindow &window, TrackRenderer &trackRenderer, const sf::View &view, sf::Font &font, const sf::Vector2f &worldOriginOffset)
{
    const float axisLengthMm = 100.f;  // 轴指示线的长度（毫米）
    const float arrowHeadSizePx = 8.f; // 箭头大小（像素）

    // --- 轨道参数显示 ---
    float trackL = trackRenderer.getTrackLength();
    float curveR = trackRenderer.getCurveRadius();
    char trackInfoBuffer[128];
    snprintf(trackInfoBuffer, sizeof(trackInfoBuffer), "Track Length: %dmm\nCurve Radius: %dmm",
             static_cast<int>(trackL), static_cast<int>(curveR));
    std::string trackInfoStr = trackInfoBuffer;

    sf::Text trackInfoText(trackInfoStr, font, 12);
    trackInfoText.setFillColor(sf::Color::Black);
    sf::Vector2f viewTopLeft = window.mapPixelToCoords(sf::Vector2i(10, 10), view);
    trackInfoText.setPosition(viewTopLeft);
    window.draw(trackInfoText);

    // --- 渲染坐标系 (通常是视图的 (0,0) 或 TrackRenderer 的世界原点) ---
    // Let's assume worldOriginOffset IS the intended render origin for the track
    sf::CircleShape renderOriginDebug(5.f);
    renderOriginDebug.setFillColor(sf::Color::Cyan);
    renderOriginDebug.setOrigin(5.f, 5.f);
    renderOriginDebug.setPosition(worldOriginOffset); // Use the passed worldOriginOffset
    window.draw(renderOriginDebug);

    sf::Text renderLabel("Render Origin (Track)", font, 6);
    renderLabel.setFillColor(sf::Color::Cyan);
    renderLabel.setPosition(renderOriginDebug.getPosition().x + 10.f, renderOriginDebug.getPosition().y - 20.f);
    window.draw(renderLabel);

    float renderAxisDisplayLengthPx = axisLengthMm * trackRenderer.getMmToPxRatio() * trackRenderer.getScaleFactor();

    sf::Vector2f r_origin = worldOriginOffset; // Relative to the passed origin
    sf::Vector2f r_xEnd(r_origin.x + renderAxisDisplayLengthPx, r_origin.y);
    sf::Vertex renderXAxis[] = {sf::Vertex(r_origin, sf::Color::Red), sf::Vertex(r_xEnd, sf::Color::Red)};
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
    sf::Vector2f r_yEnd(r_origin.x, r_origin.y + renderAxisDisplayLengthPx); // Assuming Y-down for typical screen drawing, adjust if Y-up
    // If your track's coordinate system (and thus worldOriginOffset) is Y-up, then it should be r_origin.y - renderAxisDisplayLengthPx
    // Let's stick to Y-up as per previous atan2 comments for track orientation
    r_yEnd = sf::Vector2f(r_origin.x, r_origin.y - renderAxisDisplayLengthPx); // Y-up
    sf::Vertex renderYAxis[] = {sf::Vertex(r_origin, sf::Color::Green), sf::Vertex(r_yEnd, sf::Color::Green)};
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

    // --- 后端坐标系 (通常是轨道的逻辑 (0,0)mm ) ---
    // This should be the track's logical (0,0)mm transformed to screen space, including the worldOriginOffset
    sf::Vector2f backendOriginRawPx;
    float tempAngle;
    // Get the backend (0,0) position *relative to the worldOriginOffset*
    trackRenderer.getPointAndOrientationOnCenterLine(0.0f, backendOriginRawPx, tempAngle, worldOriginOffset);

    sf::CircleShape backendOriginDebug(5.f);
    backendOriginDebug.setFillColor(sf::Color::Magenta);
    backendOriginDebug.setOrigin(5.f, 5.f);
    backendOriginDebug.setPosition(backendOriginRawPx); // This position already includes worldOriginOffset
    window.draw(backendOriginDebug);

    sf::Text backendLabel("Backend Origin (0,0)mm", font, 12);
    backendLabel.setFillColor(sf::Color::Magenta);
    backendLabel.setPosition(backendOriginRawPx.x + 10.f, backendOriginRawPx.y - 20.f);
    window.draw(backendLabel);
}

int main()
{
    // 使用默认窗口大小，因为 TrackRenderer 会自动处理缩放
    sf::RenderWindow window(sf::VideoMode(1280, 720), "Vehicle Path Position Test");
    window.setFramerateLimit(60);
    sf::Font font;
    // Adjusted font loading to be more robust relative to executable location
    if (!font.loadFromFile("assets/fonts/arial.ttf"))
    {
        if (!font.loadFromFile("../assets/fonts/arial.ttf"))
        { // Common if executable is in a 'bin' or 'test' subdir
            std::cerr << "Error loading font arial.ttf" << std::endl;
            return -1;
        }
    }

    TrackRenderer trackRenderer;
    trackRenderer.setMmToPxRatio(0.01f);
    trackRenderer.setScaleFactor(0.5f);
    trackRenderer.generateGeometry(trackRenderer.getTrackLength(), trackRenderer.getCurveRadius());

    VehicleRenderer vehicleRenderer(font, trackRenderer); // Pass trackRenderer

    // Declare and initialize view and related variables FIRST
    sf::View view = window.getDefaultView();
    view.setCenter(0, 0); // Initial center for the view
    view.zoom(2.0f);
    window.setView(view);
    sf::Vector2f initialViewSize = view.getSize(); // Store initial size for zoom calculation

    // Declare other necessary variables
    const float pathDistanceStep = 500.0f; // Example step in mm
    const float totalPathLength = trackRenderer.getTotalCenterLineLengthMm();

    gui::VehicleState vehicleState(
        "VTest1",                 // _id
        sf::Vector2f(0, 0),       // _pos
        0.0f,                     // _rawTrackPosMm
        0.0f,                     // _worldRotDeg (rotation in degrees)
        0.0f,                     // _speed_mps
        gui::VehicleStatus::IDLE, // _status
        "",                       // _currentOrder
        false,                    // _isLoaded
        gui::CargoDisplayInfo(),  // _cargoInfo
        1.0f                      // _visualScale
    );
    float currentPathDistanceMm = 0.0f;

    WarehouseRenderer warehouseRenderer;
    // This is the offset where the track's logical (0,0) will be placed in the world.
    // If view center is (0,0), and track origin is at view center, this is (0,0).
    // If view is panned, this offset effectively defines the "world" origin for the track.
    // Let's make it so the track's (0,0)mm point is at the view's initial center.
    // sf::Vector2f renderWorldOriginOffset = view.getCenter(); // This was problematic due to order
    sf::Vector2f renderWorldOriginOffset(0.f, 0.f); // Let's start with track origin at world (0,0), view will handle panning.
                                                    // Or, if you want track origin always at screen center:
                                                    // renderWorldOriginOffset = sf::Vector2f(window.getSize().x / 2.f, window.getSize().y / 2.f);
                                                    // This needs to be mapped to world coords if view is not identity.
                                                    // Simplest: keep renderWorldOriginOffset = (0,0) and let view manage what's visible.

    warehouseRenderer.initialize(trackRenderer, renderWorldOriginOffset, "assets/icons/");

    std::cout << "TrackRenderer initialized with default parameters:" << std::endl;
    std::cout << "Track Length: " << trackRenderer.getTrackLength() << "mm" << std::endl;
    std::cout << "Inner Curve Radius: " << trackRenderer.getCurveRadius() << "mm" << std::endl;
    std::cout << "Track Width: " << trackRenderer.getTrackWidth() << "mm" << std::endl;
    std::cout << "Total center line length: " << trackRenderer.getTotalCenterLineLengthMm() << "mm" << std::endl;
    std::cout << "\nControls:" << std::endl;
    std::cout << "- RIGHT/LEFT: Move vehicle forward/backward" << std::endl;
    std::cout << "- R: Reset to start position" << std::endl;
    std::cout << "- D: Toggle debug coordinate system" << std::endl;
    std::cout << "- C: Toggle debug info" << std::endl;
    std::cout << "- T: Toggle grid display" << std::endl;
    std::cout << "- SPACE: Reset view" << std::endl;
    std::cout << "- Mouse drag: Pan view" << std::endl;
    std::cout << "- Mouse wheel: Zoom" << std::endl;

    // 创建路径原点标记 (This marker setup is not drawn in the main loop, consider if it's needed)
    // If needed, ensure it uses the consistent renderWorldOriginOffset
    sf::Vector2f initialPathOriginPx; // Renamed for clarity
    float initialAngleRad = 0.0f;     // Renamed for clarity
    // REMOVE the local: sf::Vector2f worldOriginOffsetPx(0, 0);
    // USE renderWorldOriginOffset instead if this block is kept:
    // trackRenderer.getPointAndOrientationOnCenterLine(0.0f, initialPathOriginPx, initialAngleRad, renderWorldOriginOffset);
    // sf::Vector2f pathOriginRenderCoords = trackRenderer.backendToRenderTransform(initialPathOriginPx); // This transform might be redundant

    // sf::CircleShape pathOriginMarker(6.f); // This is the marker that's not drawn in the loop
    // pathOriginMarker.setFillColor(sf::Color::Yellow);
    // pathOriginMarker.setOrigin(pathOriginMarker.getRadius(), pathOriginMarker.getRadius());
    // pathOriginMarker.setPosition(pathOriginRenderCoords);

    // UI控制变量
    bool showDebugInfo = true;
    bool showDebugCoords = true;
    bool showGrid = false;
    bool showWarehouses = true; // 新增：控制仓库显示
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
                dragStart = window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y), view);
            }

            // 处理鼠标左键释放事件
            if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left)
            {
                isDragging = false;
            }

            // 处理鼠标移动事件
            if (event.type == sf::Event::MouseMoved && isDragging)
            {
                sf::Vector2f currentPos = window.mapPixelToCoords(sf::Vector2i(event.mouseMove.x, event.mouseMove.y), view);
                sf::Vector2f delta = dragStart - currentPos;

                // 移动视图
                view.move(delta);
                window.setView(view);

                // 更新拖动起始点
                dragStart = window.mapPixelToCoords(sf::Vector2i(event.mouseMove.x, event.mouseMove.y), view);
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
                    view = window.getDefaultView(); // Gets a fresh default view
                    view.setCenter(0, 0);           // Reset center
                    view.setSize(initialViewSize);  // Reset size to original default's size
                    view.zoom(2.0f);                // Apply initial zoom
                    // zoomLevel = 1.0f; // This was for display, not direct control here
                    window.setView(view);
                    std::cout << "视图已重置\n";
                }
                else if (event.key.code == sf::Keyboard::W)
                {
                    showWarehouses = !showWarehouses;
                    std::cout << (showWarehouses ? "显示仓库\n" : "隐藏仓库\n");
                }
            }
        }

        // 更新车辆状态
        vehicleState.rawTrackPositionMm = currentPathDistanceMm;

        // 获取当前位置和方向
        sf::Vector2f vehiclePosPx_world;    // Renamed from currentPosPx for clarity
        float vehicleAngleRad_world = 0.0f; // Renamed from angleDegrees, this is the variable for snprintf
                                            // Use the consistent renderWorldOriginOffset
        trackRenderer.getPointAndOrientationOnCenterLine(currentPathDistanceMm, vehiclePosPx_world, vehicleAngleRad_world, renderWorldOriginOffset);
        vehicleState.position = vehiclePosPx_world;

        // 更新车辆渲染器状态
        // Use the consistent renderWorldOriginOffset
        // vehicleRenderer.updateState(vehicleState, trackRenderer, renderWorldOriginOffset); // This updates a single m_currentState
        std::vector<gui::VehicleState> currentVehicles = {vehicleState};
        vehicleRenderer.updateVehicleStates(currentVehicles, renderWorldOriginOffset); // This updates m_vehicles

        window.clear(sf::Color(230, 240, 230)); // 使用浅绿色背景
        window.setView(view);                   // Apply the potentially panned/zoomed view

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
            int startX = static_cast<int>(viewLeft / gridSpacing);
            int endX = static_cast<int>(viewRight / gridSpacing);
            int startY = static_cast<int>(viewTop / gridSpacing);
            int endY = static_cast<int>(viewBottom / gridSpacing);

            // 绘制垂直网格线
            for (int i = startX; i <= endX; ++i)
            {
                float x = i * gridSpacing;
                sf::Vertex line[] = {sf::Vertex(sf::Vector2f(x, viewTop), sf::Color(200, 200, 200, 100)), sf::Vertex(sf::Vector2f(x, viewBottom), sf::Color(200, 200, 200, 100))};
                window.draw(line, 2, sf::Lines);
            }

            // 绘制水平网格线
            for (int i = startY; i <= endY; ++i)
            {
                float y = i * gridSpacing;
                sf::Vertex line[] = {sf::Vertex(sf::Vector2f(viewLeft, y), sf::Color(200, 200, 200, 100)), sf::Vertex(sf::Vector2f(viewRight, y), sf::Color(200, 200, 200, 100))};
                window.draw(line, 2, sf::Lines);
            }
            // 原点十字线（在世界坐标系的原点位置）
            sf::Vertex originCrossH[] = {sf::Vertex(sf::Vector2f(-50000 * trackRenderer.getMmToPxRatio(), 0), sf::Color(255, 0, 0, 150)), sf::Vertex(sf::Vector2f(50000 * trackRenderer.getMmToPxRatio(), 0), sf::Color(255, 0, 0, 150))};
            window.draw(originCrossH, 2, sf::Lines);
            sf::Vertex originCrossV[] = {sf::Vertex(sf::Vector2f(0, -50000 * trackRenderer.getMmToPxRatio()), sf::Color(255, 0, 0, 150)), sf::Vertex(sf::Vector2f(0, 50000 * trackRenderer.getMmToPxRatio()), sf::Color(255, 0, 0, 150))};
            window.draw(originCrossV, 2, sf::Lines);
        }

        window.draw(trackRenderer);

        if (showWarehouses)
        {
            window.draw(warehouseRenderer);
        }

        window.draw(vehicleRenderer);

        // 绘制路径距离为0的标记 (This is the marker drawn in the loop)
        sf::Vector2f originPosPx; // This is local to this block
        float originAngleRad;     // This is local to this block
        if (trackRenderer.getPointAndOrientationOnCenterLine(0.0f, originPosPx, originAngleRad, renderWorldOriginOffset))
        {
            sf::CircleShape originMarker(5.0f * trackRenderer.getScaleFactor()); // This is the marker drawn in the loop
            originMarker.setFillColor(sf::Color::Yellow);
            originMarker.setOrigin(originMarker.getRadius(), originMarker.getRadius());
            originMarker.setPosition(originPosPx);
            window.draw(originMarker);
            drawText(window, "Path Origin (0mm)", originPosPx + sf::Vector2f(10, -10) * trackRenderer.getScaleFactor(), font, 9, sf::Color::Black);
        }

        window.setView(window.getDefaultView()); // 重置视图以绘制UI元素
        if (showDebugInfo)
        {
            char buffer[256];
            // Make sure vehicleState.position is what you intend to display (world coords)
            // And vehicleAngleRad_world for the angle
            snprintf(buffer, sizeof(buffer),
                     "Path Dist: %.1f / %.1f mm\n"
                     "Vehicle Pos (WorldPx): (%.1f, %.1f)\n" // Displaying world coordinates
                     "Vehicle Angle: %.1f deg\n"
                     "View Center (WorldPx): (%.1f, %.1f)\n"
                     "View Size (WorldPx): (%.1f, %.1f)\n"
                     "Zoom Factor: %.2fx\n" // Display calculated zoom
                     "Track Scale: %.2f | MmToPx: %.4f",
                     currentPathDistanceMm, totalPathLength,
                     vehicleState.position.x, vehicleState.position.y, // Use the world coordinates
                     vehicleAngleRad_world * 180.0f / M_PI,            // Use the world angle
                     view.getCenter().x, view.getCenter().y,
                     view.getSize().x, view.getSize().y,
                     initialViewSize.x / view.getSize().x, // Zoom factor relative to initial
                     trackRenderer.getScaleFactor(), trackRenderer.getMmToPxRatio());
            drawText(window, buffer, sf::Vector2f(10, 10), font, 15, sf::Color::Black); // Ensure text is visible
        }

        if (showDebugCoords)
        {
            // Pass the renderWorldOriginOffset to drawCoordinateSystemDebug
            drawCoordinateSystemDebug(window, trackRenderer, view, font, renderWorldOriginOffset);
        }

        std::string controls = "Controls:\n"
                               "Left/Right: Move Vehicle\n"
                               "R: Reset to Start\n"
                               "C: Toggle Debug Info\n"
                               "D: Toggle Debug Coords\n"
                               "T: Toggle Grid\n"
                               "W: Toggle Warehouses\n"
                               "Mouse: Pan/Zoom View\n"
                               "Space: Reset View";
        drawText(window, controls, sf::Vector2f(10, window.getSize().y - 120), font, 12, sf::Color::Black); // Adjusted Y for visibility

        window.display();
    }

    return 0;
}
