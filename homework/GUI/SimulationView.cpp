#include "SimulationView.hpp"
#include "TestSimulationEngine.hpp"
#include <iostream>
#include <cmath>

void SimulationView::initialize(sf::Font &font, TestSimulationEngine &engine)
{
    // 初始化视图
    m_worldView.reset(sf::FloatRect(-400, -300, 800, 600));
    m_uiView.reset(sf::FloatRect(0, 0, 800, 600));
    m_viewCenter = sf::Vector2f(0, 0);
    m_zoomLevel = 1.0f;

    // 初始化渲染器
    m_trackRenderer.generateGeometry(engine.getTrackLength(), engine.getCurveRadius());
    
    try {
        // 尝试加载资源，处理可能出现的异常
        m_deviceRenderer.loadResources("resources/icons/");
    } catch (const std::exception& e) {
        std::cerr << "加载设备图标资源失败: " << e.what() << std::endl;
        // 尝试备用路径
        try {
            m_deviceRenderer.loadResources("GUI/resources/icons/");
        } catch (...) {
            std::cerr << "备用路径资源加载也失败" << std::endl;
        }
    }

    // 保存引擎引用
    m_engine = &engine;

    // 初始化选择系统
    m_selectedObject = nullptr;
}

void SimulationView::updateViewTransforms(float deltaTime)
{
    // 如果有动画或其他需要更新的视图变换，在这里处理
    // 例如：平滑移动视图中心点等

    // 更新视图
    m_worldView.setCenter(m_viewCenter);
    float zoomFactor = std::pow(0.8f, m_zoomLevel); // 缩放因子指数变化
    m_worldView.setSize(800 * zoomFactor, 600 * zoomFactor);
}

void SimulationView::renderWorld(sf::RenderTarget &target)
{
    // 设置世界坐标系视图
    sf::View previousView = target.getView();
    target.setView(m_worldView);

    // 绘制轨道
    target.draw(m_trackRenderer);

    // 绘制设备
    // 假设我们有一个设备位置的列表
    for (const auto &devicePair : m_engine->getDevices())
    {
        int deviceId = devicePair.first;
        const DeviceState &deviceState = m_engine->getDeviceState(deviceId);
        sf::Vector2f position = m_engine->getDevicePosition(deviceId);
        m_deviceRenderer.renderDevice(target, deviceState, position);
    }    // 绘制车辆
    const std::vector<VehicleState>& vehicles = m_engine->getVehicles();
    for (const auto& vehicle : vehicles)
    {
        // 计算车辆位置和旋转角度
        float angle = vehicle.position * 2.0f * M_PI;
        float radius = m_engine->getTrackRadius();
        float x = radius * std::cos(angle);
        float y = radius * std::sin(angle) * 0.7f;
        
        sf::Vector2f position(x, y);
        float rotation = angle * 180.0f / M_PI + 90.0f;
        
        m_vehicleRenderer.renderVehicle(target, vehicle, position, rotation);
    }

    // 恢复之前的视图
    target.setView(previousView);
}

void SimulationView::handleViewEvent(const sf::Event &event, const sf::Vector2f &mousePos)
{
    // 处理鼠标移动事件（拖拽视图）
    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Right)
    {
        m_isDragging = true;
        m_lastMousePos = mousePos;
    }
    else if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Right)
    {
        m_isDragging = false;
    }
    else if (event.type == sf::Event::MouseMoved && m_isDragging)
    {
        // 计算差值并更新视图中心
        sf::Vector2f delta = m_lastMousePos - mousePos;
        float zoomFactor = std::pow(0.8f, m_zoomLevel); // 与缩放级别相匹配
        m_viewCenter += delta * zoomFactor;
        m_lastMousePos = mousePos;
    }
    // 处理鼠标滚轮事件（缩放视图）
    else if (event.type == sf::Event::MouseWheelScrolled)
    {
        if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel)
        {
            // 更新缩放级别
            m_zoomLevel += event.mouseWheelScroll.delta > 0 ? -0.5f : 0.5f;

            // 限制缩放范围
            if (m_zoomLevel < -5.0f)
                m_zoomLevel = -5.0f;
            if (m_zoomLevel > 5.0f)
                m_zoomLevel = 5.0f;
        }
    }
    // 处理对象选择事件（左键点击）
    else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
    {
        sf::Vector2f worldPos = screenToWorld(mousePos);
        selectObjectAt(worldPos);
    }
}

void SimulationView::updateViewport(const sf::FloatRect &viewport)
{
    m_worldView.setViewport(viewport);
    m_uiView.setViewport(viewport);
}

sf::Vector2f SimulationView::screenToWorld(const sf::Vector2f &screenPos) const
{
    // 将屏幕坐标转换为世界坐标
    return sf::Vector2f(
        m_worldView.getCenter().x + (screenPos.x - 400) * (m_worldView.getSize().x / 800),
        m_worldView.getCenter().y + (screenPos.y - 300) * (m_worldView.getSize().y / 600));
}

void SimulationView::selectObjectAt(const sf::Vector2f &worldPos)
{
    // 实现对象选择逻辑
    // 首先检查是否点击了车辆
    const std::vector<VehicleState>& vehicles = m_engine->getVehicles();
    for (const auto& vehicle : vehicles)
    {
        // 计算车辆位置
        float angle = vehicle.position * 2.0f * M_PI;
        float radius = m_engine->getTrackRadius();
        float x = radius * std::cos(angle);
        float y = radius * std::sin(angle) * 0.7f;
        
        sf::Vector2f vehiclePos(x, y);
        float distance = std::sqrt(std::pow(vehiclePos.x - worldPos.x, 2) +
                                   std::pow(vehiclePos.y - worldPos.y, 2));

        // 如果点击位置在车辆半径内（假设半径为10个单位）
        if (distance < 10.0f)
        {
            // 创建车辆模拟对象
            m_selectedObject = std::make_shared<SimObject>(
                SimObject::ObjectType::Vehicle,
                vehicle.id);
            return;
        }
    }

    // 然后检查是否点击了设备
    for (const auto &devicePair : m_engine->getDevices())
    {
        int deviceId = devicePair.first;
        sf::Vector2f devicePos = m_engine->getDevicePosition(deviceId);
        float distance = std::sqrt(std::pow(devicePos.x - worldPos.x, 2) +
                                   std::pow(devicePos.y - worldPos.y, 2));

        // 如果点击位置在设备半径内（假设半径为15个单位）
        if (distance < 15.0f)
        {
            // 创建设备模拟对象
            m_selectedObject = std::make_shared<SimObject>(
                SimObject::ObjectType::Device,
                deviceId);
            return;
        }
    }

    // 如果点击的是空白区域，取消选择
    m_selectedObject = nullptr;
}

std::shared_ptr<SimObject> SimulationView::getSelectedObject() const
{
    return m_selectedObject;
}