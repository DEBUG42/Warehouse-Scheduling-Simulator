#pragma once

// 设备类型枚举
enum class DeviceType
{
    INPUT_STATION,    // 入库口
    OUTPUT_STATION,   // 出库口
    STORAGE_STATION,  // 仓储区
    WORK_STATION      // 工作站
};

// 设备状态枚举
enum class DeviceStatus
{
    IDLE,       // 空闲
    BUSY,       // 忙碌
    FAULT       // 故障
};

// 设备状态信息结构
struct DeviceState
{
    int id;                 // 设备ID
    DeviceType type;        // 设备类型
    float trackPosition;    // 轨道位置（毫米）
    DeviceStatus status;    // 设备状态
    int capacity;           // 最大容量
    int currentLoad;        // 当前负载
    int materialId;         // 当前处理的物料ID（-1表示无物料）
    
    // 构造函数，设置默认值
    DeviceState() : id(0), type(DeviceType::STORAGE_STATION), 
                    trackPosition(0.0f), status(DeviceStatus::IDLE),
                    capacity(1), currentLoad(0), materialId(-1) {}
};
