#include <SFML/Graphics.hpp>
#include <map>
#include "Task.hpp"
#include "Device.hpp"

class Vehicle {
public:
    // 运动状态枚举
    enum class MotionState {
        Accelerating,  // 加速阶段
        Cruising,      // 匀速阶段
        Decelerating,  // 减速阶段
        Stopped        // 静止状态
    };

private:
    // 固有属性
    const float m_length;            // 车辆长度（米）
    const float m_maxStraightSpeed;   // 直轨最大速度（米/秒）
    const float m_maxCurveSpeed;     // 弯轨最大速度（米/秒）
    const float m_acceleration;       // 加减速度（米/秒²）
    
    // 动态状态
    struct {
        float position;               // 轨道位置（0~trackLength）
        float currentSpeed;           // 当前速度（米/秒）
        MotionState motionState;      // 当前运动状态
        const Task* currentTask = nullptr; // 当前执行的任务
        sf::Clock operationTimer;     // 装卸货操作计时器
    } m_state;

public:
    // 运动控制
    /**
     * @brief 更新车辆物理状态
     * @param deltaTime 仿真时间增量（秒）
     * @param leadingVehicle 前车对象（可为nullptr）
     * @param trackInfo 当前轨道段信息（直轨/弯轨）
     * @return 是否发生状态变更（用于触发UI更新）
     */
    bool updatePhysics(float deltaTime, const Vehicle* leadingVehicle, const TrackSegment& trackInfo);
    
    // 任务操作
    /**
     * @brief 开始执行任务
     * @param task 任务对象引用
     * @param currentDevice 当前所在设备引用
     * @return 是否成功启动任务
     */
    bool startTask(const Task& task, DeviceBase& currentDevice);
    
    /**
     * @brief 完成当前任务
     * @param targetDevice 目标设备引用
     */
    void completeTask(DeviceBase& targetDevice);
    
    // 状态查询
    /**
     * @brief 获取车辆当前位置（轨道坐标系）
     * @return 标准化位置（0.0~1.0对应轨道周长）
     */
    float getNormalizedPosition() const;
    
    /**
     * @brief 获取当前载货状态
     * @return 货物信息结构体（包含物料编号等）
     */
    CargoInfo getCargoInfo() const;
};