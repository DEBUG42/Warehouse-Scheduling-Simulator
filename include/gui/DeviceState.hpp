#pragma once
#include <SFML/Graphics.hpp>

// 设备类型枚举
enum class DeviceType
{
    INPUT_STATION,   // 入库口
    OUTPUT_STATION,  // 出库口
    STORAGE_STATION, // 仓储区
    WORK_STATION,    // 工作站
    StorageIn,       // 入库接口
    StorageOut,      // 出库接口
    WorkstationIn,   // 工作站入
    WorkstationOut   // 工作站出
};

// 设备状态枚举
enum class DeviceStatus
{
    IDLE,    // 空闲
    BUSY,    // 忙碌
    FAULT,   // 故障
    WORKING, // 工作中
    OFFLINE  // 离线
};

// 设备状态信息结构
struct DeviceState
{
    int id;                   // 设备ID
    DeviceType type;          // 设备类型
    float trackPosition;      // 轨道位置（毫米）
    DeviceStatus status;      // 设备状态
    int capacity;             // 最大容量
    int currentLoad;          // 当前负载
    int materialId;           // 当前处理的物料ID（-1表示无物料）
    float processingProgress; // 处理进度(0-1)
    int queuedTaskCount;      // 等待任务数
    sf::Vector2f position;    // 设备位置

    // 构造函数，设置默认值
    DeviceState() : id(0), type(DeviceType::STORAGE_STATION),
                    trackPosition(0.0f), status(DeviceStatus::IDLE),
                    capacity(1), currentLoad(0), materialId(-1),
                    processingProgress(0.0f), queuedTaskCount(0),
                    position(sf::Vector2f(0.0f, 0.0f)) {}
};
