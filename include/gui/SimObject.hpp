#pragma once
#include <string>
#include <SFML/Graphics.hpp>
#include "../Core/Vehicle.hpp"

// 基础模拟对象类，用于对象选择系统
class SimObject
{
public:
    enum class ObjectType
    {
        Vehicle, // 车辆
        Device,  // 设备
        Track    // 轨道
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
    int id;                           // 车辆ID
    float trackPosition;              // 轨道位置（毫米，从起点开始计算）
    float speed;                      // 当前速度（米/秒）
    bool isLoaded;                    // 载货状态（true=载货，false=空载）
    int currentTaskId;                // 当前任务ID（-1表示无任务）
    Vehicle::MotionState motionState; // 运动状态
    sf::Vector2f worldPosition;       // 世界坐标位置

    // 构造函数，设置默认值
    VehicleState() : id(0), trackPosition(0.0f), speed(0.0f),
                     isLoaded(false), currentTaskId(-1),
                     motionState(Vehicle::MotionState::IDLE),
                     worldPosition(0.0f, 0.0f) {}
};
