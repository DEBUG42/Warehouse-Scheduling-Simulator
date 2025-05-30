#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include <functional>
#include "TrackRenderer.hpp"
// #include "DeviceRenderer.hpp" // Removed
#include "VehicleRenderer.hpp"
#include "WarehouseRenderer.hpp"
#include "SimulationInterface.hpp"
#include "../src/Core/Vehicle.hpp"
#include "../src/Core/Device.hpp"

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
    sf::Vector2f m_worldOriginOffsetPx;        // Offset of the track's (0,0) from the view's (0,0)    // 选择系统
    std::shared_ptr<Vehicle> m_selectedObject; // 存储的状态数据
    std::vector<Vehicle *> m_vehicles;         // 车辆状态
    std::vector<DeviceBase *> m_devices;       // 设备状态（使用Core设备指针）// 视图控制参数
    sf::Vector2f m_unzoomedWorldViewSize;      // 世界视图在 m_zoomLevel = 0 时的基础大小    // 回调函数
    std::function<void(int)> m_onVehicleSelected; // 车辆选择回调    // 显示控制变量
    bool m_showGrid = false;       // 显示网格
    bool m_showWarehouses = true;  // 显示仓库
    bool m_showVehicles = true;    // 显示车辆
    bool m_showDebugInfo = false;  // 显示调试信息

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
    void selectObjectAt(const sf::Vector2f &worldPos); /**
                                                        * @brief 获取当前选中的对象
                                                        * @return 选中对象指针（可能为nullptr）
                                                        */
    std::shared_ptr<Vehicle> getSelectedObject() const;

    /**
     * @brief 更新车辆状态
     * @param vehicles 车辆状态列表
     */
    void updateVehicles(const std::vector<Vehicle *> &vehicles); /**
                                                                  * @brief 更新设备状态
                                                                  * @param devices 设备状态列表
                                                                  */
    void updateDevices(const std::vector<DeviceBase *> &devices);

    /**
     * @brief 设置车辆选择回调
     * @param callback 车辆选择回调函数，参数为车辆ID，-1表示取消选择
     */
    void setVehicleSelectedCallback(std::function<void(int)> callback);    // 调整视图大小
    void resize(unsigned int width, unsigned int height);    // 显示控制方法
    void setShowGrid(bool show);
    bool getShowGrid() const;
    void setShowWarehouses(bool show);
    bool getShowWarehouses() const;
    void setShowVehicles(bool show);
    bool getShowVehicles() const;
    void setShowDebugInfo(bool show);    bool getShowDebugInfo() const;

    /**
     * @brief 获取轨道渲染器引用，用于访问轨道几何信息
     * @return TrackRenderer 的引用
     */
    const TrackRenderer& getTrackRenderer() const;

private:
    /**
     * @brief 选择指定位置的车辆
     * @param worldPos 世界坐标位置
     */
    void selectVehicleAt(const sf::Vector2f &worldPos);

    /**
     * @brief 渲染网格
     * @param target SFML渲染目标
     */
    void renderGrid(sf::RenderTarget &target);

    /**
     * @brief 渲染路径原点标记
     * @param target SFML渲染目标
     */
    void renderPathOriginMarker(sf::RenderTarget &target);    /**
     * @brief 渲染UI叠加层
     * @param target SFML渲染目标
     */
    void renderUIOverlay(sf::RenderTarget &target);

    /**
     * @brief 渲染调试信息
     * @param target SFML渲染目标
     */
    void renderDebugInfo(sf::RenderTarget &target);
};