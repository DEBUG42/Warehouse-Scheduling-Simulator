#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include "TrackRenderer.hpp"
#include "DeviceRenderer.hpp"
#include "VehicleRenderer.hpp"
#include "SimObject.hpp"

class TestSimulationEngine;

class SimulationView
{
private:
    // 视图变换参数
    sf::View m_worldView;      // 世界坐标系视图
    sf::View m_uiView;         // UI叠加层视图
    sf::Vector2f m_viewCenter; // 当前视图中心（世界坐标）
    float m_zoomLevel = 1.0f;  // 当前缩放级别

    // 对象渲染器
    TrackRenderer m_trackRenderer;     // 轨道绘制组件
    DeviceRenderer m_deviceRenderer;   // 设备绘制组件
    VehicleRenderer m_vehicleRenderer; // 车辆绘制组件

    // 交互状态
    bool m_isDragging = false;   // 正在拖拽视图标志
    sf::Vector2f m_lastMousePos; // 上一次鼠标位置（屏幕坐标）
    
    // 仿真引擎引用
    TestSimulationEngine *m_engine;

    // 选择系统
    std::shared_ptr<SimObject> m_selectedObject;

public:    /**
     * @brief 初始化仿真视图
     * @param font 字体引用
     * @param engine 仿真引擎引用
     */
    void initialize(sf::Font &font, TestSimulationEngine &engine);

    /**
     * @brief 更新视图变换参数
     * @param deltaTime 帧时间
     */
    void updateViewTransforms(float deltaTime);

    /**
     * @brief 渲染世界场景
     * @param target SFML渲染目标
     */
    void renderWorld(sf::RenderTarget &target);

    /**
     * @brief 处理视图相关输入事件
     * @param event SFML事件对象
     * @param mousePos 鼠标当前位置（屏幕坐标）
     */
    void handleViewEvent(const sf::Event &event, const sf::Vector2f &mousePos);

    /**
     * @brief 更新视口
     * @param viewport 视口矩形
     */
    void updateViewport(const sf::FloatRect &viewport);

    /**
     * @brief 将屏幕坐标转换为世界坐标
     * @param screenPos 屏幕坐标
     * @return 世界坐标
     */
    sf::Vector2f screenToWorld(const sf::Vector2f &screenPos) const;

    /**
     * @brief 选择指定位置的对象
     * @param worldPos 世界坐标位置
     */
    void selectObjectAt(const sf::Vector2f &worldPos);

    /**
     * @brief 获取当前选中的对象
     * @return 选中对象指针（可能为nullptr）
     */
    std::shared_ptr<SimObject> getSelectedObject() const;
};