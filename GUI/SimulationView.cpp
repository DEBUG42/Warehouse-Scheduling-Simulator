#include "SimulationView.hpp"
#include "TestSimulationEngine.hpp"
#include "MockSimulationInterface.hpp"
#include <iostream>
#include <cmath>
#include "DeviceState.hpp"

/**
 * @brief 初始化仿真视图（使用新版接口）
 * @param font 字体引用
 * @param simInterface 仿真接口
 */
void SimulationView::initialize(sf::Font &font, std::shared_ptr<SimulationInterface> simInterface)
{
    m_simInterface = simInterface;
    m_engine = nullptr; // 不使用旧接口
    
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
    m_trackLength = 90000.0f; // 估计总长度约90米
    m_curveRadius = 2500.0f;  // 弯道半径2.5米
    
    m_trackRenderer.setTrackWidth(600.0f); // 轨道宽度600mm
    m_trackRenderer.setCenterLineColor(sf::Color(180, 180, 180));
    m_trackRenderer.setStraightColor(sf::Color(140, 140, 140));
    m_trackRenderer.setCurveColor(sf::Color(120, 120, 120));
    m_trackRenderer.generateGeometry(m_trackLength, m_curveRadius);
    
    // 初始化仓库渲染器
    m_warehouseRenderer.initialize(m_curveRadius);
    
    // 获取初始车辆和设备状态
    if (m_simInterface) {
        m_vehicles = m_simInterface->getVehicleStates();
        m_devices = m_simInterface->getDeviceStates();
        
        // 更新仓库状态
        m_warehouseRenderer.updateDeviceStates(m_devices);
    }
}

/**
 * @brief 初始化仿真视图（兼容旧版接口）
 * @param font 字体引用
 * @param engine 仿真引擎引用
 */
void SimulationView::initialize(sf::Font &font, TestSimulationEngine &engine)
{
    m_engine = &engine;
    m_simInterface = nullptr; // 不使用新接口
    
    // 初始化视图变换
    m_viewCenter = sf::Vector2f(0.0f, 0.0f);
    m_zoomLevel = 0.4f;
    
    // 初始化视图
    m_worldView.setSize(1280, 720);
    m_worldView.setCenter(m_viewCenter);
    
    m_uiView.setSize(1280, 720);
    m_uiView.setCenter(640, 360);
    
    // 初始化轨道渲染器
    m_trackLength = 90000.0f;
    m_curveRadius = 2500.0f;
    
    m_trackRenderer.setTrackWidth(600.0f);
    m_trackRenderer.setCenterLineColor(sf::Color(180, 180, 180));
    m_trackRenderer.setStraightColor(sf::Color(140, 140, 140));
    m_trackRenderer.setCurveColor(sf::Color(120, 120, 120));
    m_trackRenderer.generateGeometry(m_trackLength, m_curveRadius);
    
    // 向旧引擎传递轨道总长度
    m_engine->setTrackLength(m_trackLength);
    
    // 初始化仓库渲染器（旧版不完全支持）
    m_warehouseRenderer.initialize(m_curveRadius);
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
    target.draw(m_trackRenderer);
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
    // 遍历所有车辆
    for (const auto& vehicle : m_vehicles) {
        // 计算车辆位置和朝向
        sf::Vector2f position;
        float rotation;
        
        // 使用车辆渲染器计算位置
        m_vehicleRenderer.calculatePosition(vehicle, m_trackLength, m_curveRadius, position, rotation);
        
        // 渲染车辆（增强3D效果）
        m_vehicleRenderer.renderVehicle(target, vehicle, position, rotation);
    }
    
    // 绘制车辆阴影（增强3D效果）
    for (const auto& vehicle : m_vehicles) {
        sf::Vector2f position;
        float rotation;
        m_vehicleRenderer.calculatePosition(vehicle, m_trackLength, m_curveRadius, position, rotation);
        m_vehicleRenderer.renderShadow(target, position, rotation);
    }
}

/**
 * @brief 渲染UI层
 * @param target SFML渲染目标
 */
void SimulationView::renderUI(sf::RenderTarget &target)
{
    // 如果有选中对象，显示选择框或高亮效果
    if (m_selectedObject) {
        if (m_selectedObject->getType() == SimObject::ObjectType::Vehicle) {
            int vehicleId = m_selectedObject->getId();
            
            // 查找对应车辆
            for (const auto& vehicle : m_vehicles) {
                if (vehicle.id == vehicleId) {
                    sf::Vector2f position;
                    float rotation;
                    m_vehicleRenderer.calculatePosition(vehicle, m_trackLength, m_curveRadius, position, rotation);
                    
                    // 将世界坐标转换为屏幕坐标
                    sf::Vector2f screenPos = target.mapCoordsToPixel(position, m_worldView);
                    
                    // 绘制选择框
                    sf::RectangleShape selectionRect(sf::Vector2f(50, 25));
                    selectionRect.setOrigin(25, 12.5f);
                    selectionRect.setPosition(screenPos);
                    selectionRect.setFillColor(sf::Color::Transparent);
                    selectionRect.setOutlineColor(sf::Color::Yellow);
                    selectionRect.setOutlineThickness(2.0f);
                    target.draw(selectionRect);
                    
                    // 显示车辆详细信息
                    // TODO: 显示更多信息
                    break;
                }
            }
        } else if (m_selectedObject->getType() == SimObject::ObjectType::Device) {
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
    // 处理鼠标滚轮事件（缩放）
    if (event.type == sf::Event::MouseWheelScrolled)
    {
        // 每次滚动改变0.1的缩放级别
        m_zoomLevel += event.mouseWheelScroll.delta * 0.1f;
        
        // 限制缩放范围
        m_zoomLevel = std::max(-2.0f, std::min(m_zoomLevel, 3.0f));
    }
    
    // 处理鼠标按下事件（开始拖动）
    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
    {
        m_isDragging = true;
        m_lastMousePos = mousePos;
        
        // 选择对象
        selectObjectAt(screenToWorld(mousePos));
    }
    
    // 处理鼠标释放事件（停止拖动）
    if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left)
    {
        m_isDragging = false;
    }
    
    // 处理鼠标移动事件（拖动视图）
    if (event.type == sf::Event::MouseMoved && m_isDragging)
    {
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
 * @brief 将屏幕坐标转换为世界坐标
 * @param screenPos 屏幕坐标
 * @return 世界坐标
 */
sf::Vector2f SimulationView::screenToWorld(const sf::Vector2f &screenPos) const
{
    // 获取当前视图的变换矩阵
    sf::Transform transform = sf::Transform().translate(m_worldView.getCenter())
                             .scale(1.0f / std::pow(2.0f, m_zoomLevel), 
                                   1.0f / std::pow(2.0f, m_zoomLevel))
                             .translate(-m_worldView.getSize().x / 2.0f, 
                                        -m_worldView.getSize().y / 2.0f);
    
    // 应用视口变换
    sf::FloatRect viewport = m_worldView.getViewport();
    sf::Vector2f normalizedPos(
        (screenPos.x - viewport.left * 1280) / (viewport.width * 1280),
        (screenPos.y - viewport.top * 720) / (viewport.height * 720)
    );
    
    sf::Vector2f viewSize = m_worldView.getSize();
    sf::Vector2f worldPos = transform.transformPoint(
        normalizedPos.x * viewSize.x,
        normalizedPos.y * viewSize.y
    );
    
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
    const WarehouseRenderer::WarehouseInterface* interface = 
        m_warehouseRenderer.getInterfaceAt(worldPos);
    
    if (interface) {
        std::cout << "选中了仓库接口 ID: " << interface->id << std::endl;
        // 创建仓库对象
        m_selectedObject = std::make_shared<SimObject>(SimObject::ObjectType::Device, interface->id);
        return;
    }
    
    // 检查是否点击了车辆
    for (const auto& vehicle : m_vehicles) {
        sf::Vector2f position;
        float rotation;
        m_vehicleRenderer.calculatePosition(vehicle, m_trackLength, m_curveRadius, position, rotation);
        
        // 简单的矩形判断（可优化为更精确的碰撞检测）
        float vehicleHalfWidth = 8.0f;  // 像素单位
        float vehicleHalfLength = 20.0f; // 像素单位
        
        // 转换点击位置到车辆局部坐标系
        float angle = -rotation * M_PI / 180.0f;
        sf::Vector2f relativePos = worldPos - position;
        sf::Vector2f localPos(
            relativePos.x * std::cos(angle) - relativePos.y * std::sin(angle),
            relativePos.x * std::sin(angle) + relativePos.y * std::cos(angle)
        );
        
        if (std::abs(localPos.x) < vehicleHalfLength && std::abs(localPos.y) < vehicleHalfWidth) {
            std::cout << "选中了车辆 ID: " << vehicle.id << std::endl;
            m_selectedObject = std::make_shared<SimObject>(SimObject::ObjectType::Vehicle, vehicle.id);
            return;
        }
    }
}

/**
 * @brief 获取当前选中的对象
 * @return 选中对象指针（可能为nullptr）
 */
std::shared_ptr<SimObject> SimulationView::getSelectedObject() const
{
    return m_selectedObject;
}

/**
 * @brief 更新车辆状态
 * @param vehicles 车辆状态列表
 */
void SimulationView::updateVehicles(const std::vector<VehicleState>& vehicles)
{
    m_vehicles = vehicles;
}

/**
 * @brief 更新设备状态
 * @param devices 设备状态列表
 */
void SimulationView::updateDevices(const std::vector<DeviceState>& devices)
{
    m_devices = devices;
    m_warehouseRenderer.updateDeviceStates(devices);
    
    // 转换为WarehouseState并存储（以支持新接口）
    std::vector<WarehouseState> warehouses;
    for (const auto& device : devices) {
        WarehouseState warehouse;
        warehouse.id = device.id;
        warehouse.trackPosition = device.trackPosition;
        warehouse.isInterface = (device.type == DeviceType::INPUT_STATION || 
                               device.type == DeviceType::OUTPUT_STATION);
        warehouse.capacity = device.capacity;
        warehouse.currentLoad = device.currentLoad;
        
        warehouses.push_back(warehouse);
    }
    
    // 存储转换后的仓库状态
    m_warehouses = warehouses;
}