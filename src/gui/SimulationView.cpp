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
    m_simInterface = simInterface;

    float mmToPxRatio = 0.04f;
    float trackWidthMm = 40000.0f;

    // The track's (0,0) doc origin is at world (0,0) for now.
    m_worldOriginOffsetPx = sf::Vector2f(0.0f, 0.0f);

    // Initialize Track Renderer
    m_trackRenderer.setMmToPxRatio(mmToPxRatio);
    m_trackRenderer.setTrackWidth(trackWidthMm);
    m_trackRenderer.generateGeometry(m_trackLength, m_curveRadius); // Pass MM dimensions

    // Initialize Warehouse Renderer
    m_warehouseRenderer.initialize(m_trackRenderer, m_worldOriginOffsetPx, "resources/icons/");

    // Store the initial size as the base size for zoom level 0
    m_unzoomedWorldViewSize = initialViewSize;
    m_worldView.setSize(m_unzoomedWorldViewSize); // Set initial size

    m_viewCenter = m_worldOriginOffsetPx;
    m_worldView.setCenter(m_viewCenter);

    // Calculate the zoom factor needed to fit the track into 90% of the view width
    float estimatedTrackSystemWidthPx = (m_trackLength + 2.0f * m_curveRadius) * mmToPxRatio;
    float desiredViewWidthForTrackPx = initialViewSize.x * 0.9f;
    float targetZoomFactor = 1.0f; // This is the factor for sf::View::zoom()

    if (desiredViewWidthForTrackPx > 0 && estimatedTrackSystemWidthPx > 0)
    {
        targetZoomFactor = estimatedTrackSystemWidthPx / desiredViewWidthForTrackPx;
    }

    // Apply this initial zoom directly
    m_worldView.zoom(targetZoomFactor);

    // Set m_zoomLevel (abstract, for mouse wheel) based on this targetZoomFactor
    // targetZoomFactor = 1.0f / pow(2.0f, m_zoomLevel_abstract)
    // pow(2.0f, m_zoomLevel_abstract) = 1.0f / targetZoomFactor
    // m_zoomLevel_abstract = log2(1.0f / targetZoomFactor)
    if (targetZoomFactor > 0.0001f)
    {
        m_zoomLevel = -std::log2(targetZoomFactor);
    }
    else
    {
        m_zoomLevel = 0.0f; // Default if targetZoomFactor is problematic
    }

    m_uiView.setSize(initialViewSize);
    m_uiView.setCenter(initialViewSize.x / 2.0f, initialViewSize.y / 2.0f);

    // Get initial simulation states
    if (m_simInterface)
    {
        m_vehicles = m_simInterface->getVehicleStates();
        m_devices = m_simInterface->getDeviceStates();
        m_warehouseRenderer.updateDeviceStates(m_devices);
        // m_vehicleRenderer might need an update too, if it uses m_trackLength, m_curveRadius and mmToPxRatio
        // It seems VehicleRenderer::calculatePosition already takes these as params.
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
 * @brief 渲染世界场景
 * @param target SFML渲染目标
 */
void SimulationView::renderWorld(sf::RenderTarget &target)
{
    // 设置世界坐标系视图（用于场景元素）
    target.setView(m_worldView);

    // 渲染轨道
    renderTrack(target);

    // 渲染仓库/接口设备
    renderWarehouses(target);

    // 渲染车辆
    renderVehicles(target);

    // 切换到UI视图（用于叠加元素）
    target.setView(m_uiView);

    // 渲染UI层（选择框、提示文本等）
    renderUI(target);

    // 重置为默认视图
    target.setView(target.getDefaultView());
}

/**
 * @brief 渲染轨道
 * @param target SFML渲染目标
 */
void SimulationView::renderTrack(sf::RenderTarget &target)
{
    sf::RenderStates trackStates;
    // TrackRenderer is assumed to draw its geometry relative to its own (0,0),
    // which corresponds to the documentation's origin point (bottom-left inner track point).
    // m_worldOriginOffsetPx shifts this entire track system in the world.
    trackStates.transform.translate(m_worldOriginOffsetPx);
    target.draw(m_trackRenderer, trackStates);
}

/**
 * @brief 渲染仓库/接口设备
 * @param target SFML渲染目标
 */
void SimulationView::renderWarehouses(sf::RenderTarget &target)
{
    // WarehouseRenderer has been initialized with the trackRenderer and worldOriginOffsetPx,
    // so it should be drawing its devices directly in the correct world coordinates.
    // No additional transform should be needed here if WarehouseRenderer handles it.
    target.draw(m_warehouseRenderer);
}

/**
 * @brief 渲染车辆
 * @param target SFML渲染目标
 */
void SimulationView::renderVehicles(sf::RenderTarget &target)
{
    // 创建临时数组存储车辆位置和旋转角度，避免重复计算
    struct VehiclePositionData
    {
        sf::Vector2f position;
        float rotation;
    };

    std::vector<VehiclePositionData> vehiclePositions(m_vehicles.size());
    static float lastCalculatedTime = 0.0f;
    static sf::Clock positionUpdateClock;

    // 如果车辆数量大于0且自上次计算以来至少过了16.67ms (60fps)，则重新计算位置
    float currentTime = positionUpdateClock.getElapsedTime().asSeconds();
    bool shouldRecalculate = (currentTime - lastCalculatedTime) > 0.01667f || vehiclePositions.empty();

    if (shouldRecalculate)
    {
        // 先计算所有车辆的位置和朝向
        for (size_t i = 0; i < m_vehicles.size(); ++i)
        {
            // 使用车辆渲染器计算位置
            m_vehicleRenderer.calculateScreenPositionAndRotation(
                *m_vehicles[i],        // Dereference pointer to get Vehicle reference
                m_trackRenderer,       // Pass TrackRenderer reference
                m_worldOriginOffsetPx, // Pass world origin offset
                vehiclePositions[i].position,
                vehiclePositions[i].rotation);
        }
        lastCalculatedTime = currentTime;
    }

    // 先绘制所有车辆阴影（增强3D效果），确保阴影在车辆下方
    for (size_t i = 0; i < m_vehicles.size(); ++i)
    {
        m_vehicleRenderer.renderShadow(
            target,
            vehiclePositions[i].position,
            vehiclePositions[i].rotation);
    } // 然后绘制所有车辆
    for (size_t i = 0; i < m_vehicles.size(); ++i)
    {
        m_vehicleRenderer.renderSingleVehicle(
            target,
            *m_vehicles[i], // Dereference pointer to get Vehicle reference
            vehiclePositions[i].position,
            vehiclePositions[i].rotation);
    }
}

/**
 * @brief 渲染UI层
 * @param target SFML渲染目标
 */
void SimulationView::renderUI(sf::RenderTarget &target)
{ // 如果有选中对象，显示选择框或高亮效果
    if (m_selectedObject)
    {
        // 对象为车辆（m_selectedObject is always a Vehicle pointer now）
        std::string vehicleId = std::to_string(m_selectedObject->getId());
        static const float selectionWidth = 50.0f;
        static const float selectionHeight = 25.0f;
        static sf::RectangleShape selectionRect(sf::Vector2f(selectionWidth, selectionHeight));
        static bool shapeInitialized = false;
        if (!shapeInitialized)
        {
            selectionRect.setOrigin(selectionWidth / 2, selectionHeight / 2);
            selectionRect.setFillColor(sf::Color::Transparent);
            selectionRect.setOutlineColor(sf::Color::Yellow);
            selectionRect.setOutlineThickness(2.0f);
            shapeInitialized = true;
        }
        for (const auto &vehicle : m_vehicles)
        {
            if (vehicle && vehicle->getId() == std::stoi(vehicleId))
            {
                sf::Vector2f position;
                float rotation;
                // Updated call to calculateScreenPositionAndRotation
                m_vehicleRenderer.calculateScreenPositionAndRotation(
                    *vehicle,
                    m_trackRenderer,
                    m_worldOriginOffsetPx,
                    position,
                    rotation);
                sf::Vector2i screenPos = target.mapCoordsToPixel(position, m_worldView);
                sf::Vector2f screenPosF(static_cast<float>(screenPos.x), static_cast<float>(screenPos.y));
                selectionRect.setPosition(screenPosF);
                target.draw(selectionRect);
                break;
            }
        }

        // Example for device selection highlight:
        // sf::Vector2f devicePos = m_selectedObject->getPosition(); // This is world position
        // sf::Vector2i screenPos = target.mapCoordsToPixel(devicePos, m_worldView);
        // sf::CircleShape selectionCircle(10.f); // Or RectangleShape
        // selectionCircle.setOrigin(10.f, 10.f);
        // selectionCircle.setPosition(static_cast<float>(screenPos.x), static_cast<float>(screenPos.y));
        // selectionCircle.setFillColor(sf::Color::Transparent);
        // selectionCircle.setOutlineColor(sf::Color::Cyan);
        // selectionCircle.setOutlineThickness(2.0f);
        // target.draw(selectionCircle);
    }
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
 * @brief 选择指定位置的对象
 * @param worldPos 世界坐标位置
 */
void SimulationView::selectObjectAt(const sf::Vector2f &worldPos)
{
    m_selectedObject = nullptr;

    const auto *selectedInterface = m_warehouseRenderer.getInterfaceAt(worldPos);
    if (selectedInterface)
    {
        std::cout << "Selected Warehouse Interface ID: " << selectedInterface->id
                  << " at (" << selectedInterface->worldCenterPx.x << "," << selectedInterface->worldCenterPx.y << ")" << std::endl;
        // 注意：这里选择的是设备接口，不是车辆，所以设置为nullptr
        m_selectedObject = nullptr;
        return;
    }
    for (const auto &vehicle : m_vehicles)
    {
        if (!vehicle)
            continue; // 跳过空指针

        sf::Vector2f vehicleRenderPosPx;
        float vehicleRotation;
        // Updated call to calculateScreenPositionAndRotation
        m_vehicleRenderer.calculateScreenPositionAndRotation(
            *vehicle,
            m_trackRenderer,
            m_worldOriginOffsetPx,
            vehicleRenderPosPx,
            vehicleRotation);

        float clickRadiusMm = 500.f;
        float clickRadiusPx = clickRadiusMm * m_trackRenderer.getMmToPxRatio();
        if (std::hypot(worldPos.x - vehicleRenderPosPx.x, worldPos.y - vehicleRenderPosPx.y) < clickRadiusPx)
        {
            std::cout << "Selected Vehicle ID: " << vehicle->getId() << std::endl;
            // 设置选中的车辆
            m_selectedObject = std::shared_ptr<Vehicle>(vehicle, [](Vehicle *) {}); // 非拥有的共享指针

            // 调用车辆选择回调
            if (m_onVehicleSelected)
            {
                m_onVehicleSelected(vehicle->getId());
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
