#pragma once
#include "SimObject.hpp"
#include <vector>
#include <functional>

/**
 * @brief 仿真接口类
 * 
 * 定义前后端通信的接口，包括：
 * 1. 获取车辆数量和状态
 * 2. 获取仓库/接口设备状态
 * 3. 控制仿真速度和暂停
 * 4. 状态变化回调机制
 */
class SimulationInterface {
public:
    // 状态更新回调函数类型定义
    using VehicleUpdateCallback = std::function<void(const std::vector<VehicleState>&)>;
    using WarehouseUpdateCallback = std::function<void(const std::vector<WarehouseState>&)>;
    using SimSpeedUpdateCallback = std::function<void(float)>;
    using SimPauseUpdateCallback = std::function<void(bool)>;
    
    virtual ~SimulationInterface() = default;
    
    /**
     * @brief 获取仿真中的车辆数量
     * @return 车辆数量
     */
    virtual int getVehicleCount() const = 0;
    
    /**
     * @brief 获取所有车辆的状态
     * @return 包含所有车辆状态的向量
     */
    virtual std::vector<VehicleState> getVehicleStates() const = 0;
    
    /**
     * @brief 获取仓库/接口设备数量
     * @return 仓库/接口设备数量
     */
    virtual int getWarehouseCount() const = 0;
    
    /**
     * @brief 获取所有仓库/接口设备状态
     * @return 包含所有仓库/接口设备状态的向量
     */
    virtual std::vector<WarehouseState> getWarehouseStates() const = 0;
    
    /**
     * @brief 获取轨道总长度（毫米）
     * @return 轨道总长度
     */
    virtual float getTrackLength() const = 0;
    
    /**
     * @brief 获取弯道半径（毫米）
     * @return 弯道半径
     */
    virtual float getCurveRadius() const = 0;
    
    /**
     * @brief 获取当前仿真速度倍率
     * @return 仿真速度倍率，1.0表示实时速度
     */
    virtual float getSimulationSpeed() const = 0;
    
    /**
     * @brief 获取仿真暂停状态
     * @return 是否暂停，true表示暂停
     */
    virtual bool isPaused() const = 0;
    
    /**
     * @brief 设置仿真速度倍率
     * @param speed 速度倍率，1.0表示实时速度
     */
    virtual void setSimulationSpeed(float speed) = 0;
    
    /**
     * @brief 设置仿真暂停状态
     * @param paused 是否暂停，true表示暂停
     */
    virtual void setPaused(bool paused) = 0;
    
    /**
     * @brief 设置车辆状态更新回调函数
     * @param callback 回调函数
     */
    virtual void setVehicleUpdateCallback(VehicleUpdateCallback callback) = 0;
    
    /**
     * @brief 设置仓库/接口设备状态更新回调函数
     * @param callback 回调函数
     */
    virtual void setWarehouseUpdateCallback(WarehouseUpdateCallback callback) = 0;
    
    /**
     * @brief 设置仿真速度更新回调函数
     * @param callback 回调函数
     */
    virtual void setSimSpeedUpdateCallback(SimSpeedUpdateCallback callback) = 0;
    
    /**
     * @brief 设置仿真暂停状态更新回调函数
     * @param callback 回调函数
     */
    virtual void setPauseUpdateCallback(SimPauseUpdateCallback callback) = 0;
};
