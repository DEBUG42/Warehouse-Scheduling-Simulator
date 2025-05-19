#pragma once
#include <string>

// 基础模拟对象类，用于对象选择系统
class SimObject
{
public:
    enum class ObjectType
    {
        Vehicle,  // 车辆
        Warehouse // 仓库/接口设备
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
    int id;              // 车辆ID
    float trackPosition; // 轨道位置（毫米，从起点开始计算）
    float speed;         // 当前速度（毫米/秒）
    bool isLoaded;       // 载货状态（true=载货，false=空载）
    int currentTaskId;   // 当前任务ID（-1表示无任务）

    // 构造函数，设置默认值
    VehicleState() : id(0), trackPosition(0.0f), speed(0.0f),
                     isLoaded(false), currentTaskId(-1) {}
};

// 仓库/接口设备状态信息结构体
struct WarehouseState
{
    int id;              // 仓库/接口ID
    float trackPosition; // 轨道位置（毫米，从起点开始计算）
    bool isInterface;    // 是否为接口设备（true=接口，false=仓库）
    int capacity;        // 最大容量
    int currentLoad;     // 当前负载

    // 构造函数，设置默认值
    WarehouseState() : id(0), trackPosition(0.0f), isInterface(false),
                       capacity(0), currentLoad(0) {}
};
