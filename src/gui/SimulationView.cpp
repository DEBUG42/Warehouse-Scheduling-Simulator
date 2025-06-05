#include "gui/SimulationView.hpp"
#include <iostream>
#include <cmath>
#include "gui/DeviceState.hpp"
#include "gui/CoordinateUtils.hpp"

/**
 * @brief 构造函数，传入全局字体，确保VehicleRenderer等成员能正确初始化
 */
SimulationView::SimulationView(sf::Font &font)
    : m_vehicleRenderer(font, m_trackRenderer)
{
    // 预先分配容器内存，避免频繁重新分配
    m_vehicles.reserve(20); // 预估最大车辆数
    m_devices.reserve(20);  // 预估最大设备数
}

/**
 * @brief 初始化仿真视图（使用新版接口）
 * @param font 字体引用
 * @param simInterface 仿真接口
 * @param initialViewSize The initial size of the viewport for this view in pixels
 */
void SimulationView::initialize(sf::Font &font, std::shared_ptr<SimulationInterface> simInterface, const sf::Vector2f &initialViewSize)
{
    m_simInterface = simInterface; // 渲染参数设置（与VehiclePathPositionTest.cpp一致）
    float mmToPxRatio = 0.01f;     // 毫米到像素转换比例
    float scaleFactor = 2.0f;      // 渲染缩放因子
    float trackWidthMm = 1200.0f;  // 轨道宽度（1.2米）

    // 世界坐标原点偏移量（与测试文件一致）
    m_worldOriginOffsetPx = sf::Vector2f(0.0f, 0.0f);

    // 初始化轨道渲染器
    m_trackRenderer.setMmToPxRatio(mmToPxRatio);
    m_trackRenderer.setScaleFactor(scaleFactor);
    m_trackRenderer.setTrackWidth(trackWidthMm);
    m_trackRenderer.generateGeometry(m_trackLength, m_curveRadius);

    // 初始化仓库渲染器
    m_warehouseRenderer.initialize(m_trackRenderer, m_worldOriginOffsetPx, "resources/icons/");

    // 初始化车辆渲染器（已在构造函数中初始化，这里可以设置额外参数）
    // m_vehicleRenderer 已通过构造函数初始化

    // 设置视图参数
    m_unzoomedWorldViewSize = initialViewSize;
    m_worldView.setSize(m_unzoomedWorldViewSize);
    m_viewCenter = m_worldOriginOffsetPx;
    m_worldView.setCenter(m_viewCenter);

    // 计算适合轨道显示的初始缩放
    float estimatedTrackSystemWidthPx = (m_trackLength + 2.0f * m_curveRadius) * mmToPxRatio * scaleFactor;
    float desiredViewWidthForTrackPx = initialViewSize.x * 0.9f;
    float targetZoomFactor = 1.0f;

    if (desiredViewWidthForTrackPx > 0 && estimatedTrackSystemWidthPx > 0)
    {
        targetZoomFactor = estimatedTrackSystemWidthPx / desiredViewWidthForTrackPx;
    }

    // 应用初始缩放
    m_worldView.zoom(targetZoomFactor);
    if (targetZoomFactor > 0.0001f)
    {
        m_zoomLevel = -std::log2(targetZoomFactor);
    }
    else
    {
        m_zoomLevel = 0.0f;
    }

    // 设置UI视图
    m_uiView.setSize(initialViewSize);
    m_uiView.setCenter(initialViewSize.x / 2.0f, initialViewSize.y / 2.0f);

    // 获取初始仿真状态
    if (m_simInterface)
    {
        m_vehicles = m_simInterface->getVehicleStates();
        m_devices = m_simInterface->getDeviceStates();
        m_warehouseRenderer.updateDeviceStates(m_devices);
    }
}

/**
 * @brief 更新视图变换参数
 * @param deltaTime 帧时间
 */
void SimulationView::updateViewTransforms(float deltaTime)
{
    // Update world view
    m_worldView.setCenter(m_viewCenter);
    // Reset the view size to its unzoomed state before applying the new zoom factor
    m_worldView.setSize(m_unzoomedWorldViewSize);

    float zoomMethodFactor = std::pow(2.0f, m_zoomLevel);
    // sf::View::zoom(factor): factor < 1 zooms IN, factor > 1 zooms OUT.
    // If m_zoomLevel increases (scroll up, want to zoom IN), viewZoomMethodFactor increases.
    // Then 1.0f / viewZoomMethodFactor decreases, which correctly tells sf::View::zoom to zoom IN.
    m_worldView.zoom(1.0f / zoomMethodFactor);
}

/**
 * @brief 渲染世界场景 - 按照VehiclePathPositionTest.cpp的验证方法
 * @param target SFML渲染目标
 */
void SimulationView::renderWorld(sf::RenderTarget &target)
{
    // 保存当前视图
    sf::View originalView = target.getView();

    // 不要清除整个窗口！这会覆盖GUI组件
    // target.clear(sf::Color(230, 240, 230)); // 删除这行

    // 设置世界坐标系视图
    target.setView(m_worldView);

    // 绘制仿真区域的背景色（只在视口内）
    sf::FloatRect viewport = m_worldView.getViewport();
    sf::Vector2f viewSize = m_worldView.getSize();
    sf::Vector2f viewCenter = m_worldView.getCenter();

    sf::RectangleShape background;
    background.setSize(viewSize);
    background.setOrigin(viewSize.x / 2.0f, viewSize.y / 2.0f);
    background.setPosition(viewCenter);
    background.setFillColor(sf::Color(230, 240, 230));
    target.draw(background);

    // 1. 渲染网格（如果启用）
    if (m_showGrid)
    {
        renderGrid(target);
    }

    // 2. 渲染轨道（使用测试文件验证的方法）
    target.draw(m_trackRenderer);

    // 3. 渲染仓库设备（如果启用）
    if (m_showWarehouses)
    {
        target.draw(m_warehouseRenderer);
    }    // 4. 渲染车辆（使用测试文件验证的VehicleRenderer方法）
    if (m_showVehicles && !m_vehicles.empty())
    {
        std::cout << "SimulationView::renderWorld() - Rendering vehicles (count: " << m_vehicles.size() << ")" << std::endl;
        // 更新车辆渲染器状态
        m_vehicleRenderer.setVehiclesToRender(m_vehicles);
        target.draw(m_vehicleRenderer);
    }
    else
    {
        if (!m_showVehicles)
            std::cout << "SimulationView::renderWorld() - Vehicle rendering disabled" << std::endl;
        if (m_vehicles.empty())
            std::cout << "SimulationView::renderWorld() - No vehicles available" << std::endl;
    }

    // 5. 渲染路径原点标记（与测试文件一致）
    renderPathOriginMarker(target);

    // 6. 渲染调试信息（如果启用）
    if (m_showDebugInfo)
    {
        renderDebugInfo(target);
    }

    // 切换到UI视图渲染叠加层
    target.setView(m_uiView);

    // 7. 渲染UI叠加层（选择框、HUD等）
    renderUIOverlay(target);

    // 恢复原始视图而不是重置为默认视图
    target.setView(originalView);
}

/**
 * @brief 处理视图相关输入事件
 * @param event SFML事件对象
 * @param mousePos 鼠标当前位置（屏幕坐标）
 */
void SimulationView::handleViewEvent(const sf::Event &event, const sf::Vector2f &mousePos)
{
    // 使用switch语句处理不同类型的事件，比if-else更高效
    switch (event.type)
    {
    case sf::Event::MouseWheelScrolled:
        // 处理鼠标滚轮事件（缩放）
        {
            // 每次滚动改变0.1的缩放级别
            m_zoomLevel += event.mouseWheelScroll.delta * 0.1f;

            // 限制缩放范围
            m_zoomLevel = std::max(-2.0f, std::min(m_zoomLevel, 3.0f));
        }
        break;

    case sf::Event::MouseButtonPressed:
        if (event.mouseButton.button == sf::Mouse::Left)
        {
            // 处理鼠标按下事件（开始拖动）
            m_isDragging = true;
            m_lastMousePos = mousePos;

            // 选择对象
            selectObjectAt(screenToWorld(mousePos));
        }
        break;

    case sf::Event::MouseButtonReleased:
        if (event.mouseButton.button == sf::Mouse::Left)
        {
            // 处理鼠标释放事件（停止拖动）
            m_isDragging = false;
        }
        break;

    case sf::Event::MouseMoved:
        if (m_isDragging)
        {
            // 处理鼠标移动事件（拖动视图）
            // 计算鼠标移动距离
            sf::Vector2f delta = m_lastMousePos - mousePos;

            // 根据当前缩放级别调整拖动灵敏度
            float zoomFactor = std::pow(2.0f, m_zoomLevel);
            delta.x *= 1.0f / zoomFactor;
            delta.y *= 1.0f / zoomFactor;

            // 更新视图中心
            m_viewCenter += delta;

            // 更新上次鼠标位置
            m_lastMousePos = mousePos;
        }
        break;

    default:
        // 忽略其他类型的事件
        break;
    }
}

/**
 * @brief 更新视口
 * @param viewport 视口矩形
 */
void SimulationView::updateViewport(const sf::FloatRect &viewport)
{
    // 更新世界视图和UI视图的视口
    m_worldView.setViewport(viewport);
    m_uiView.setViewport(viewport);
}

/**
 * @brief 将屏幕坐标转换为世界坐标 - 优化版本
 * @param screenPos 屏幕坐标
 * @return 世界坐标
 */
sf::Vector2f SimulationView::screenToWorld(const sf::Vector2f &screenPos) const
{
    // 缓存计算结果以提升性能
    static float lastZoomLevel = -1.0f;
    static sf::Vector2f lastViewCenter(0, 0);
    static sf::Vector2f lastViewSize(0, 0);
    static sf::FloatRect lastViewport(0, 0, 0, 0);
    static sf::Transform cachedTransform;
    static sf::Vector2f cachedViewSize;

    // 检查是否需要重新计算变换矩阵
    bool needsUpdate =
        lastZoomLevel != m_zoomLevel ||
        lastViewCenter != m_worldView.getCenter() ||
        lastViewSize != m_worldView.getSize() ||
        lastViewport != m_worldView.getViewport();

    if (needsUpdate)
    {
        // 更新缓存的变量
        lastZoomLevel = m_zoomLevel;
        lastViewCenter = m_worldView.getCenter();
        lastViewSize = m_worldView.getSize();
        lastViewport = m_worldView.getViewport();
        cachedViewSize = m_worldView.getSize();

        // 计算新的变换矩阵
        float zoomFactor = std::pow(2.0f, m_zoomLevel);

        cachedTransform = sf::Transform()
                              .translate(m_worldView.getCenter())
                              .scale(1.0f / zoomFactor, 1.0f / zoomFactor)
                              .translate(-cachedViewSize.x / 2.0f, -cachedViewSize.y / 2.0f);
    }

    // 应用视口变换
    sf::FloatRect viewport = m_worldView.getViewport();
    sf::Vector2f normalizedPos(
        (screenPos.x - viewport.left * 1280) / (viewport.width * 1280),
        (screenPos.y - viewport.top * 720) / (viewport.height * 720));

    // 使用缓存的视图大小
    sf::Vector2f worldPos = cachedTransform.transformPoint(
        normalizedPos.x * cachedViewSize.x,
        normalizedPos.y * cachedViewSize.y);

    return worldPos;
}

/**
 * @brief 选择指定位置的对象 - 统一的对象选择逻辑
 * @param worldPos 世界坐标位置
 */
void SimulationView::selectObjectAt(const sf::Vector2f &worldPos)
{
    m_selectedObject = nullptr;
    // 清除 VehicleRenderer 的选中状态
    m_vehicleRenderer.setSelectedVehicle(nullptr);

    // 1. 优先检查设备/接口的选择
    const auto *selectedInterface = m_warehouseRenderer.getInterfaceAt(worldPos);
    if (selectedInterface)
    {
        std::cout << "Selected Warehouse Interface ID: " << selectedInterface->id
                  << " at (" << selectedInterface->worldCenterPx.x << "," << selectedInterface->worldCenterPx.y << ")" << std::endl;
        // 注意：当前只支持车辆选择，设备选择暂时不存储
        return;
    }

    // 2. 检查车辆的选择
    selectVehicleAt(worldPos);
}

/**
 * @brief 选择指定位置的车辆
 * @param worldPos 世界坐标位置
 */
void SimulationView::selectVehicleAt(const sf::Vector2f &worldPos)
{
    const float clickRadius = 5.0f; // 点击检测半径（像素）

    for (const auto *vehicle : m_vehicles)
    {
        if (!vehicle)
            continue;

        sf::Vector2f vehiclePosition;
        float vehicleRotation;

        // 计算车辆的渲染位置
        m_vehicleRenderer.calculateScreenPositionAndRotation(
            *vehicle,
            m_trackRenderer,
            m_worldOriginOffsetPx,
            vehiclePosition,
            vehicleRotation);

        // 计算点击位置与车辆位置的距离
        float distance = std::hypot(worldPos.x - vehiclePosition.x, worldPos.y - vehiclePosition.y);        if (distance < clickRadius)
        {
            std::cout << "Selected Vehicle ID: " << vehicle->id << std::endl;

            // 创建非拥有的共享指针
            m_selectedObject = std::shared_ptr<Vehicle>(
                const_cast<Vehicle *>(vehicle),
                [](Vehicle *) {} // 空删除器，因为我们不拥有这个指针
            );

            // 设置 VehicleRenderer 的选中车辆，用于高亮显示
            m_vehicleRenderer.setSelectedVehicle(vehicle);

            // 触发车辆选择回调
            if (m_onVehicleSelected)
            {
                m_onVehicleSelected(vehicle->id);
            }
            return;
        }
    }
}

/**
 * @brief 获取当前选中的对象
 * @return 选中对象指针（可能为nullptr）
 */
std::shared_ptr<Vehicle> SimulationView::getSelectedObject() const
{
    return m_selectedObject;
}

/**
 * @brief 更新车辆状态
 * @param vehicles 车辆状态列表
 */
void SimulationView::updateVehicles(const std::vector<Vehicle *> &vehicles)
{
    m_vehicles = vehicles;
    m_vehicleRenderer.setVehiclesToRender(m_vehicles);
    // m_vehicleRenderer.updateVehicleStates(m_vehicles);
}

/**
 * @brief 更新设备状态
 * @param devices 设备状态列表
 */
void SimulationView::updateDevices(const std::vector<DeviceBase *> &devices)
{
    m_devices = devices;
    m_warehouseRenderer.updateDeviceStates(m_devices);
}

/**
 * @brief 设置车辆选择回调
 * @param callback 车辆选择回调函数，参数为车辆ID，-1表示取消选择
 */
void SimulationView::setVehicleSelectedCallback(std::function<void(int)> callback)
{
    m_onVehicleSelected = callback;
}

void SimulationView::resize(unsigned int width, unsigned int height)
{
    // 更新UI视图
    m_uiView.setSize(static_cast<float>(width), static_cast<float>(height));
    m_uiView.setCenter(static_cast<float>(width) / 2.f, static_cast<float>(height) / 2.f);

    // 更新世界视图
    float aspectRatio = static_cast<float>(width) / height;
    m_unzoomedWorldViewSize = sf::Vector2f(static_cast<float>(width), static_cast<float>(height));
    // The aspect ratio change is handled by setSize. Center remains the same unless explicitly changed.
    // updateViewTransforms will correctly apply the zoom to the new base size.
    // No need to directly call m_worldView.setSize here if updateViewTransforms is called in the main loop.
    // However, if called standalone, ensure the view is updated:
    m_worldView.setSize(m_unzoomedWorldViewSize);
    m_worldView.setCenter(m_viewCenter); // Ensure center is maintained
    float zoomMethodFactor = std::pow(2.0f, m_zoomLevel);
    m_worldView.zoom(1.0f / zoomMethodFactor); // Re-apply zoom to the new size
}

/**
 * @brief 渲染网格 - 按照VehiclePathPositionTest.cpp的方法
 * @param target SFML渲染目标
 */
void SimulationView::renderGrid(sf::RenderTarget &target)
{
    // 获取当前视图的可见区域
    sf::Vector2f viewCenter = m_worldView.getCenter();
    sf::Vector2f viewSize = m_worldView.getSize();
    float gridSpacing = 500.0f * m_trackRenderer.getMmToPxRatio() * m_trackRenderer.getScaleFactor(); // 网格间距，对应500mm

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
        sf::Vertex line[] = {
            sf::Vertex(sf::Vector2f(x, viewTop), sf::Color(200, 200, 200, 100)),
            sf::Vertex(sf::Vector2f(x, viewBottom), sf::Color(200, 200, 200, 100))};
        target.draw(line, 2, sf::Lines);
    }

    // 绘制水平网格线
    for (int i = startY; i <= endY; ++i)
    {
        float y = i * gridSpacing;
        sf::Vertex line[] = {
            sf::Vertex(sf::Vector2f(viewLeft, y), sf::Color(200, 200, 200, 100)),
            sf::Vertex(sf::Vector2f(viewRight, y), sf::Color(200, 200, 200, 100))};
        target.draw(line, 2, sf::Lines);
    }

    // 原点十字线（在世界坐标系的原点位置）
    sf::Vertex originCrossH[] = {
        sf::Vertex(sf::Vector2f(-50000 * m_trackRenderer.getMmToPxRatio(), 0), sf::Color(255, 0, 0, 150)),
        sf::Vertex(sf::Vector2f(50000 * m_trackRenderer.getMmToPxRatio(), 0), sf::Color(255, 0, 0, 150))};
    target.draw(originCrossH, 2, sf::Lines);

    sf::Vertex originCrossV[] = {
        sf::Vertex(sf::Vector2f(0, -50000 * m_trackRenderer.getMmToPxRatio()), sf::Color(255, 0, 0, 150)),
        sf::Vertex(sf::Vector2f(0, 50000 * m_trackRenderer.getMmToPxRatio()), sf::Color(255, 0, 0, 150))};
    target.draw(originCrossV, 2, sf::Lines);
}

/**
 * @brief 渲染路径原点标记 - 按照VehiclePathPositionTest.cpp的方法
 * @param target SFML渲染目标
 */
void SimulationView::renderPathOriginMarker(sf::RenderTarget &target)
{
    // 绘制路径距离为0的标记
    sf::Vector2f originPosPx;
    float originAngleRad;

    if (m_trackRenderer.getPointAndOrientationOnCenterLine(0.0f, originPosPx, originAngleRad, m_worldOriginOffsetPx))
    {
        sf::CircleShape originMarker(5.0f * m_trackRenderer.getScaleFactor());
        originMarker.setFillColor(sf::Color::Yellow);
        originMarker.setOrigin(originMarker.getRadius(), originMarker.getRadius());
        originMarker.setPosition(originPosPx);
        target.draw(originMarker);

        // 可以在这里添加文本标签，但需要字体支持
        // drawText(target, "Path Origin (0mm)", originPosPx + sf::Vector2f(10, -10) * m_trackRenderer.getScaleFactor(), font, 9, sf::Color::Black);
    }
}

/**
 * @brief 渲染UI叠加层 - 包括选择框等
 * @param target SFML渲染目标
 */
void SimulationView::renderUIOverlay(sf::RenderTarget &target)
{
    if (m_selectedObject)
    {
        // 创建高亮矩形
        static sf::RectangleShape selectionRect;
        static bool initialized = false;

        if (!initialized)
        {
            selectionRect.setFillColor(sf::Color::Transparent);
            selectionRect.setOutlineColor(sf::Color::Yellow);
            selectionRect.setOutlineThickness(3.0f);
            initialized = true;
        }

        // 查找选中的车辆并计算其屏幕位置
        for (const auto *vehicle : m_vehicles)
        {
            if (vehicle && vehicle->id == m_selectedObject->id)
            {
                sf::Vector2f position;
                float rotation;

                // 计算车辆的屏幕位置
                m_vehicleRenderer.calculateScreenPositionAndRotation(
                    *vehicle,
                    m_trackRenderer,
                    m_worldOriginOffsetPx,
                    position,
                    rotation);

                // 转换为屏幕坐标用于UI层绘制
                sf::Vector2i screenPos = target.mapCoordsToPixel(position, m_worldView);
                sf::Vector2f screenPosF(static_cast<float>(screenPos.x), static_cast<float>(screenPos.y));

                // 设置高亮框的大小和位置
                const float highlightSize = 60.0f;
                selectionRect.setSize(sf::Vector2f(highlightSize, highlightSize * 0.6f));
                selectionRect.setOrigin(highlightSize / 2.0f, highlightSize * 0.3f);
                selectionRect.setPosition(screenPosF);
                selectionRect.setRotation(rotation);

                target.draw(selectionRect);
                break;
            }
        }
    }
}

/**
 * @brief 渲染调试信息
 * @param target SFML渲染目标
 */
void SimulationView::renderDebugInfo(sf::RenderTarget &target)
{
    // 创建调试文本
    static sf::Text debugText;
    static bool textInitialized = false;

    if (!textInitialized)
    {
        // 注意：这里需要字体支持，但为了避免依赖问题，我们先创建文本对象
        debugText.setCharacterSize(12);
        debugText.setFillColor(sf::Color::White);
        textInitialized = true;
    }

    // 获取当前视图信息
    sf::Vector2f viewCenter = m_worldView.getCenter();
    sf::Vector2f viewSize = m_worldView.getSize();
    float zoomFactor = std::pow(2.0f, m_zoomLevel);

    // 获取轨道信息
    float trackLength = m_trackRenderer.getTrackLength();
    float curveRadius = m_trackRenderer.getCurveRadius();
    float totalPathLength = m_trackRenderer.getTotalCenterLineLengthMm();

    // 构建调试信息字符串
    char debugBuffer[512];
    snprintf(debugBuffer, sizeof(debugBuffer),
             "=== SimulationView Debug Info ===\n"
             "View Center: (%.1f, %.1f)\n"
             "View Size: (%.1f, %.1f)\n"
             "Zoom Level: %.2f (Factor: %.2fx)\n"
             "Track Length: %.0fmm\n"
             "Curve Radius: %.0fmm\n"
             "Total Path: %.0fmm\n"
             "Vehicle Count: %d\n"
             "Grid: %s | Warehouses: %s | Vehicles: %s\n"
             "Track Scale: %.2f | MmToPx: %.4f",
             viewCenter.x, viewCenter.y,
             viewSize.x, viewSize.y,
             m_zoomLevel, zoomFactor,
             trackLength, curveRadius, totalPathLength,
             (int)m_vehicles.size(),
             m_showGrid ? "ON" : "OFF",
             m_showWarehouses ? "ON" : "OFF",
             m_showVehicles ? "ON" : "OFF",
             m_trackRenderer.getScaleFactor(),
             m_trackRenderer.getMmToPxRatio());

    // 设置调试文本位置（世界坐标系中的固定位置）
    sf::Vector2f debugPos = viewCenter - viewSize * 0.4f; // 左上角区域
    debugText.setPosition(debugPos);
    debugText.setString(debugBuffer);

    // 绘制半透明背景
    sf::RectangleShape debugBackground;
    debugBackground.setSize(sf::Vector2f(400, 200));
    debugBackground.setPosition(debugPos - sf::Vector2f(5, 5));
    debugBackground.setFillColor(sf::Color(0, 0, 0, 128));
    target.draw(debugBackground);

    // 绘制调试文本（如果有字体支持）
    target.draw(debugText);

    // 绘制坐标系原点标记
    sf::CircleShape originMarker(8.0f);
    originMarker.setFillColor(sf::Color::Red);
    originMarker.setOrigin(8.0f, 8.0f);
    originMarker.setPosition(m_worldOriginOffsetPx);
    target.draw(originMarker);

    // 绘制坐标轴
    float axisLength = 100.0f * m_trackRenderer.getMmToPxRatio() * m_trackRenderer.getScaleFactor();

    // X轴（红色）
    sf::Vertex xAxis[] = {
        sf::Vertex(m_worldOriginOffsetPx, sf::Color::Red),
        sf::Vertex(m_worldOriginOffsetPx + sf::Vector2f(axisLength, 0), sf::Color::Red)};
    target.draw(xAxis, 2, sf::Lines);

    // Y轴（绿色）
    sf::Vertex yAxis[] = {
        sf::Vertex(m_worldOriginOffsetPx, sf::Color::Green),
        sf::Vertex(m_worldOriginOffsetPx + sf::Vector2f(0, -axisLength), sf::Color::Green)};
    target.draw(yAxis, 2, sf::Lines);
}

// 显示控制方法实现
void SimulationView::setShowGrid(bool show)
{
    m_showGrid = show;
}

bool SimulationView::getShowGrid() const
{
    return m_showGrid;
}

void SimulationView::setShowWarehouses(bool show)
{
    m_showWarehouses = show;
}

bool SimulationView::getShowWarehouses() const
{
    return m_showWarehouses;
}

void SimulationView::setShowVehicles(bool show)
{
    m_showVehicles = show;
}

bool SimulationView::getShowVehicles() const
{
    return m_showVehicles;
}

void SimulationView::setShowDebugInfo(bool show)
{
    m_showDebugInfo = show;
}

bool SimulationView::getShowDebugInfo() const
{
    return m_showDebugInfo;
}

const TrackRenderer &SimulationView::getTrackRenderer() const
{
    return m_trackRenderer;
}
