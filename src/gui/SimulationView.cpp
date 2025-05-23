#include "gui/SimulationView.hpp"
#include "gui/MockSimulationInterface.hpp"
#include <iostream>
#include <cmath>
#include "gui/DeviceState.hpp"
#include "gui/WarehouseState.hpp"
#include "gui/SimObject.hpp"

/**
 * @brief 构造函数，传入全局字体，确保VehicleRenderer等成员能正确初始化
 */
SimulationView::SimulationView(sf::Font &font)
    : m_vehicleRenderer(font)
{
    // 预先分配容器内存，避免频繁重新分配
    m_vehicles.reserve(20); // 预估最大车辆数
    m_devices.reserve(20);  // 预估最大设备数
}

/**
 * @brief 初始化仿真视图（使用新版接口）
 * @param font 字体引用
 * @param simInterface 仿真接口
 */
void SimulationView::initialize(sf::Font &font, std::shared_ptr<SimulationInterface> simInterface)
{
    m_simInterface = simInterface;

    // 初始化视图变换
    m_viewCenter = sf::Vector2f(0.0f, 0.0f);
    m_zoomLevel = 0.4f;

    // 初始化世界视图（用于场景元素）
    m_worldView.setSize(1280, 720);
    m_worldView.setCenter(m_viewCenter);

    // 初始化UI视图（用于叠加UI元素，不受世界变换影响）
    m_uiView.setSize(1280, 720);
    m_uiView.setCenter(640, 360);

    // 初始化轨道渲染器
    float straightTrackSegmentLength = 40000.0f; // 根据文档，单个直道段长度40米
    m_curveRadius = 2500.0f;                     // 弯道半径2.5米，与文档一致
    m_trackLength = straightTrackSegmentLength;  // 存储轨道长度供后续使用

    m_trackRenderer.setTrackWidth(600.0f); // 轨道宽度600mm (视觉效果)
    m_trackRenderer.generateGeometry(straightTrackSegmentLength, m_curveRadius);

    // 初始化仓库渲染器
    m_warehouseRenderer.initialize(m_curveRadius);

    // 获取初始车辆和设备状态
    if (m_simInterface)
    {
        // 获取模拟接口中的车辆和设备状态
        m_vehicles = m_simInterface->getVehicleStates();
        m_devices = m_simInterface->getDeviceStates();

        // 更新仓库状态
        m_warehouseRenderer.updateDeviceStates(m_devices);
    }
}

/**
 * @brief 更新视图变换参数
 * @param deltaTime 帧时间
 */
void SimulationView::updateViewTransforms(float deltaTime)
{
    // 更新世界视图
    m_worldView.setCenter(m_viewCenter);
    float zoomFactor = std::pow(2.0f, m_zoomLevel); // 指数缩放
    m_worldView.zoom(1.0f / zoomFactor);
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
    // TrackRenderer内部以其几何中心为(0,0)绘制（像素单位）。
    // 文档原点是左下弯道中心 (0,0)_doc。
    // 轨道几何中心 O_track_geom 相对于文档原点 O_doc 的坐标是：
    // X_doc = m_curveRadius + m_trackLength / 2.0f (其中m_trackLength是直段长度)
    // Y_doc = 0 (假设轨道上下对称于文档原点的X轴)
    // 将这个毫米单位的偏移转换为像素单位。
    float offsetX_doc_mm = m_curveRadius + (m_trackLength / 2.0f);
    float offsetY_doc_mm = 0.0f;

    // 使用统一的 MM_TO_PIXEL 转换
    // TrackRenderer内部的m_scaleFactor已设为1.0，所以其内部单位与WarehouseRenderer的MM_TO_PIXEL效果一致
    float scaledOffsetX = offsetX_doc_mm * WarehouseRenderer::MM_TO_PIXEL;
    float scaledOffsetY = offsetY_doc_mm * WarehouseRenderer::MM_TO_PIXEL;

    sf::RenderStates trackStates;
    // 我们希望TrackRenderer的局部原点(其几何中心)被绘制在世界坐标的(scaledOffsetX, scaledOffsetY)处
    // 这样，TrackRenderer的几何形状就会以文档原点为参考正确放置。
    trackStates.transform.translate(scaledOffsetX, scaledOffsetY);
    target.draw(m_trackRenderer, trackStates);
}

/**
 * @brief 渲染仓库/接口设备
 * @param target SFML渲染目标
 */
void SimulationView::renderWarehouses(sf::RenderTarget &target)
{
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
            m_vehicleRenderer.calculatePosition(
                m_vehicles[i],
                m_trackLength,
                m_curveRadius,
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
    }

    // 然后绘制所有车辆
    for (size_t i = 0; i < m_vehicles.size(); ++i)
    {
        m_vehicleRenderer.renderVehicle(
            target,
            m_vehicles[i],
            vehiclePositions[i].position,
            vehiclePositions[i].rotation);
    }
}

/**
 * @brief 渲染UI层
 * @param target SFML渲染目标
 */
void SimulationView::renderUI(sf::RenderTarget &target)
{
    // 如果有选中对象，显示选择框或高亮效果
    if (m_selectedObject)
    {
        // 对象为车辆
        if (m_selectedObject->getType() == gui::SimObjectType::Vehicle)
        {
            std::string vehicleId = m_selectedObject->getId();

            // 常量设置，避免重复创建
            static const float selectionWidth = 50.0f;
            static const float selectionHeight = 25.0f;
            static sf::RectangleShape selectionRect(sf::Vector2f(selectionWidth, selectionHeight));
            static bool shapeInitialized = false;

            // 只初始化一次形状属性
            if (!shapeInitialized)
            {
                selectionRect.setOrigin(selectionWidth / 2, selectionHeight / 2);
                selectionRect.setFillColor(sf::Color::Transparent);
                selectionRect.setOutlineColor(sf::Color::Yellow);
                selectionRect.setOutlineThickness(2.0f);
                shapeInitialized = true;
            }

            // 查找对应车辆
            for (const auto &vehicle : m_vehicles)
            {
                if (vehicle.getId() == vehicleId)
                {
                    sf::Vector2f position;
                    float rotation;

                    // 计算车辆位置
                    m_vehicleRenderer.calculatePosition(vehicle, m_trackLength, m_curveRadius, position, rotation);

                    // 将世界坐标转换为屏幕坐标 - 使用快捷方法
                    sf::Vector2i screenPos = target.mapCoordsToPixel(position, m_worldView);
                    sf::Vector2f screenPosF(static_cast<float>(screenPos.x), static_cast<float>(screenPos.y));

                    // 更新选择框位置并绘制
                    selectionRect.setPosition(screenPosF);
                    target.draw(selectionRect);
                    break;
                }
            }
        }
        else if (m_selectedObject->getType() == gui::SimObjectType::Device)
        {
            // TODO: 实现设备选择高亮效果
        }
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
    // 先清除当前选择
    m_selectedObject = nullptr;

    // TODO: 实现对象选择逻辑
    // 检查是否点击了仓库
    const WarehouseRenderer::WarehouseInterface *interface =
        m_warehouseRenderer.getInterfaceAt(worldPos);

    if (interface)
    {
        std::cout << "选中了仓库接口 ID: " << interface->id << std::endl;
        // 创建仓库对象
        m_selectedObject = std::make_shared<gui::SimObject>(gui::SimObjectType::Device, std::to_string(interface->id));
        return;
    }

    // 检查是否点击了车辆
    for (const auto &vehicle : m_vehicles)
    {
        sf::Vector2f position;
        float rotation;
        m_vehicleRenderer.calculatePosition(vehicle, m_trackLength, m_curveRadius, position, rotation);

        // 简单的矩形判断（可优化为更精确的碰撞检测）
        float vehicleHalfWidth = 8.0f;   // 像素单位
        float vehicleHalfLength = 20.0f; // 像素单位

        // 转换点击位置到车辆局部坐标系
        float angle = -rotation * M_PI / 180.0f;
        sf::Vector2f relativePos = worldPos - position;
        sf::Vector2f localPos(
            relativePos.x * std::cos(angle) - relativePos.y * std::sin(angle),
            relativePos.x * std::sin(angle) + relativePos.y * std::cos(angle));

        if (std::abs(localPos.x) < vehicleHalfLength && std::abs(localPos.y) < vehicleHalfWidth)
        {
            std::cout << "选中了车辆 ID: " << vehicle.getId() << std::endl;
            m_selectedObject = std::make_shared<gui::SimObject>(gui::SimObjectType::Vehicle, vehicle.getId());
            return;
        }
    }
}

/**
 * @brief 获取当前选中的对象
 * @return 选中对象指针（可能为nullptr）
 */
std::shared_ptr<gui::SimObject> SimulationView::getSelectedObject() const
{
    return m_selectedObject;
}

/**
 * @brief 更新车辆状态
 * @param vehicles 车辆状态列表
 */
void SimulationView::updateVehicles(const std::vector<gui::VehicleState> &vehicles)
{
    m_vehicles = vehicles;
}

/**
 * @brief 更新设备状态
 * @param devices 设备状态列表
 */
void SimulationView::updateDevices(const std::vector<gui::DeviceState> &devices)
{
    m_devices = devices;
    m_warehouseRenderer.updateDeviceStates(devices);
}
void SimulationView::resize(unsigned int width, unsigned int height)
{
    // 更新UI视图
    m_uiView.setSize(width, height);
    m_uiView.setCenter(width / 2.f, height / 2.f);

    // 更新世界视图
    float aspectRatio = static_cast<float>(width) / height;
    m_worldView.setSize(m_trackLength * aspectRatio, m_trackLength);
    m_worldView.setCenter(0.f, 0.f);
}
