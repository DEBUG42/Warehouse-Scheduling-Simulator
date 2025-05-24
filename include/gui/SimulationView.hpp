#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include "TrackRenderer.hpp"
// #include "DeviceRenderer.hpp" // Removed
#include "VehicleRenderer.hpp"
#include "WarehouseRenderer.hpp"
#include "SimObject.hpp"
#include "DeviceState.hpp"
#include "SimulationInterface.hpp"

/**
 * @brief 仿真场景视图类
 *
 * 负责管理和渲染整个仿真场景，包括轨道、设备和车辆
 * 实现视图变换控制（平移、缩放）和对象选择功能
 * 作为GUI系统中负责场景显示的核心组件与仿真引擎交互
 */
class SimulationView
{
private:
    // 视图变换参数
    sf::View m_worldView;      // 世界坐标系视图
    sf::View m_uiView;         // UI叠加层视图
    sf::Vector2f m_viewCenter; // 当前视图中心（世界坐标）
    float m_zoomLevel = 1.0f;  // 当前缩放级别

    // 对象渲染器
    TrackRenderer m_trackRenderer; // 轨道绘制组件
    // DeviceRenderer m_deviceRenderer;       // 设备绘制组件（旧版） -> REMOVED
    VehicleRenderer m_vehicleRenderer;     // 车辆绘制组件
    WarehouseRenderer m_warehouseRenderer; // 仓库绘制组件（新版，已整合设备渲染）

    // 交互状态
    bool m_isDragging = false;   // 正在拖拽视图标志
    sf::Vector2f m_lastMousePos; // 上一次鼠标位置（屏幕坐标）

    // 仿真引擎引用
    std::shared_ptr<SimulationInterface> m_simInterface; // 新版接口

    // 轨道参数
    float m_trackLength = 40000.0f; // 单个直轨段长度(mm)，根据文档. Used by VehicleRenderer too.
    float m_curveRadius = 2500.0f;  // 弯道半径(mm)，根据文档. Used by VehicleRenderer too.

    // Define the visual origin for the track and warehouses within the view
    // This is where the (0,0) of your track's coordinate system will be placed in the world view.
    sf::Vector2f m_worldOriginOffsetPx; // Offset of the track's (0,0) from the view's (0,0)

    // 选择系统
    std::shared_ptr<gui::SimObject> m_selectedObject; // 存储的状态数据
    std::vector<gui::VehicleState> m_vehicles;        // 车辆状态
    std::vector<gui::DeviceState> m_devices;          // 设备状态（旧接口）

    // 视图控制参数
    sf::Vector2f m_unzoomedWorldViewSize; // 世界视图在 m_zoomLevel = 0 时的基础大小

public:
    /**
     * @brief 构造函数，传入全局字体，确保VehicleRenderer等成员能正确初始化
     */
    SimulationView(sf::Font &font);

    /**
     * @brief 初始化仿真视图（使用新版接口）
     * @param font 字体引用
     * @param simInterface 仿真接口
     * @param initialViewSize The initial size of the viewport for this view in pixels
     */
    void initialize(sf::Font &font, std::shared_ptr<SimulationInterface> simInterface, const sf::Vector2f &initialViewSize);

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
    std::shared_ptr<gui::SimObject> getSelectedObject() const;

    /**
     * @brief 更新车辆状态
     * @param vehicles 车辆状态列表
     */
    void updateVehicles(const std::vector<gui::VehicleState> &vehicles);

    /**
     * @brief 更新设备状态
     * @param devices 设备状态列表
     */
    void updateDevices(const std::vector<gui::DeviceState> &devices);

    // 调整视图大小
    void resize(unsigned int width, unsigned int height);

private:
    /**
     * @brief 渲染轨道
     * @param target SFML渲染目标
     */
    void renderTrack(sf::RenderTarget &target);

    /**
     * @brief 渲染仓库/接口设备
     * @param target SFML渲染目标
     */
    void renderWarehouses(sf::RenderTarget &target);

    /**
     * @brief 渲染车辆
     * @param target SFML渲染目标
     */
    void renderVehicles(sf::RenderTarget &target);

    /**
     * @brief 渲染UI层
     * @param target SFML渲染目标
     */
    void renderUI(sf::RenderTarget &target);
};