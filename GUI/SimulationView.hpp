//和track相关的没有测试 在注释掉track之后可以正常运行
#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include <iostream>
#include "TrackRenderer.hpp"
#include "DeviceRenderer.hpp"
#include "VehicleRenderer.hpp"
#include "../Core/Device.hpp"
class SimulationView {
private:
    // 视图变换参数
    sf::View m_worldView;              // 世界坐标系视图
    sf::View m_uiView;                 // UI叠加层视图
    sf::Vector2f m_viewCenter;         // 当前视图中心（世界坐标）
    float m_zoomLevel = 1.0f;          // 当前缩放级别
    
    // 对象渲染器
    //TrackRenderer m_trackRenderer;     // 轨道绘制组件
    DeviceRenderer m_deviceRenderer;   // 设备绘制组件
    VehicleRenderer m_vehicleRenderer; // 车辆绘制组件
    
    // 交互状态
    bool m_isDragging = false;         // 正在拖拽视图标志
    sf::Vector2f m_lastMousePos;       // 上一次鼠标位置（屏幕坐标）

    std::vector<DeviceBase>m_devices;   // 设备列表
    std::vector<Vehicle> m_vehicles;    // 车辆列表

    sf::Texture m_trackTexture;   // 背景纹理
    sf::Shader m_trackShader;     // 背景着色器

public:

    SimulationView();

    /**
     * @brief 更新视图变换参数
     * @param deltaTime 帧时间
     */
    void updateViewTransforms(float deltaTime);

    /**
     * @brief 渲染世界场景
     * @param target SFML渲染目标
     */
    void renderWorld(sf::RenderTarget& target);

    /**
     * @brief 处理视图相关输入事件
     * @param event SFML事件对象
     * @param mousePos 鼠标当前位置（屏幕坐标）
     */
    void handleViewEvent(const sf::Event& event, const sf::Vector2f& mousePos);
};