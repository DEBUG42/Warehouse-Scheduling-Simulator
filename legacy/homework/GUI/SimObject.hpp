#pragma once
#include <SFML/Graphics.hpp>

/**
 * @brief 车辆状态结构体
 * 
 * 表示仿真系统中车辆的状态信息
 */
struct VehicleState {
    int id;                // 车辆ID
    float trackPosition;   // 轨道位置（毫米），从轨道起点开始的位置
    float speed;           // 当前速度（毫米/秒）
    bool isLoaded;         // 是否载货
    int currentTaskId;     // 当前任务ID，-1表示无任务
    
    VehicleState() : id(0), trackPosition(0.0f), speed(0.0f), 
                     isLoaded(false), currentTaskId(-1) {}
};

/**
 * @brief 仓库/接口设备状态结构体
 * 
 * 表示轨道旁的仓库或接口设备的状态信息
 */
struct WarehouseState {
    int id;                // 仓库/接口设备ID
    float trackPosition;   // 轨道位置（毫米）
    bool isInterface;      // 是否是接口设备（true）还是仓库（false）
    int capacity;          // 容量
    int currentLoad;       // 当前负载
    
    WarehouseState() : id(0), trackPosition(0.0f), isInterface(false),
                       capacity(0), currentLoad(0) {}
};
