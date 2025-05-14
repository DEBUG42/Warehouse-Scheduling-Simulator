#pragma once
#include "../Core/Vehicle.hpp"
#include "../Core/Device.hpp"

// 基础模拟对象类，用于对象选择系统
class SimObject
{
public:
    enum class ObjectType
    {
        Vehicle,
        Device
    };

private:
    ObjectType m_type;
    int m_id;

public:
    SimObject(ObjectType type, int id) : m_type(type), m_id(id) {}
    virtual ~SimObject() = default;

    ObjectType getType() const { return m_type; }
    int getId() const { return m_id; }
};

// 车辆状态信息结构
struct VehicleState
{
    int id;
    float position;                   // 轨道位置
    float speed;                      // 当前速度
    Vehicle::MotionState motionState; // 运动状态
    bool isLoaded;                    // 载货状态
    int currentTaskId;                // 当前任务ID（-1表示无任务）
};

// 设备状态信息结构体
struct DeviceState
{
    int id;
    DeviceType type;
    DeviceStatus status;
    int queuedTaskCount;      // 队列任务数
    float processingProgress; // 处理进度 (0.0-1.0)
};
