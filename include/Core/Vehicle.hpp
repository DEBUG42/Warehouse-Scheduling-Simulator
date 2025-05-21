#pragma once
#include <SFML/Graphics.hpp>
#include <map>
#include <string>     // 为 Vehicle 的 ID (如果需要) 和 Task 的 ID
#include "Task.hpp"   // 假设 Task.hpp 也在 Core 命名空间或会被调整
#include "Device.hpp" // 假设 Device.hpp 也在 Core 命名空间或会被调整
// #include "../Physics/TrackSegment.hpp" // 实际项目中此文件不存在，彻底移除或注释

namespace Core
{

    // 货物信息结构体 (占位符)
    struct CargoInfo
    {
        int materialId = -1;
        int quantity = 0;
        // 可以根据需要添加其他成员，例如货物类型、来源、目的地等
    };

    class Vehicle
    {
    public:
        // 运动状态枚举
        enum class MotionState
        {
            IDLE,         // 空闲，对应旧的 Stopped 但更通用
            MOVING,       // 通用移动状态 (替代 Accelerating, Cruising, Decelerating 的宏观状态)
            ACCELERATING, // 细分状态：加速阶段
            CRUISING,     // 细分状态：匀速阶段
            DECELERATING, // 细分状态：减速阶段
            OPERATING     // 在设备处进行操作（如装卸货）
        };

    private:
        // 固有属性
        const std::string m_id;         // 车辆唯一ID (新增)
        const float m_length;           // 车辆长度（米）
        const float m_maxStraightSpeed; // 直轨最大速度（米/秒）
        const float m_maxCurveSpeed;    // 弯轨最大速度（米/秒）
        const float m_acceleration;     // 加减速度（米/秒²）

        // 动态状态
        struct State
        {
            float position;                    // 轨道位置（0~trackLength）
            float currentSpeed;                // 当前速度（米/秒）
            MotionState motionState;           // 当前运动状态
            const Task *currentTask = nullptr; // 当前执行的任务
            sf::Clock operationTimer;          // 装卸货操作计时器
            CargoInfo cargo;                   // 车辆承载的货物 (新增)
            bool isLoaded = false;             // 载货状态的简单标志
            float batteryLevel = 1.0f;         // 电量 (0.0 - 1.0)
        } m_state;

    public:
        // 构造函数 (新增)
        Vehicle(const std::string &id, float length, float maxStraightSpeed, float maxCurveSpeed, float acceleration)
            : m_id(id),
              m_length(length),
              m_maxStraightSpeed(maxStraightSpeed),
              m_maxCurveSpeed(maxCurveSpeed),
              m_acceleration(acceleration)
        {
            m_state.motionState = MotionState::IDLE;
            m_state.currentSpeed = 0.0f;
            m_state.position = 0.0f;
        }

        // 运动控制
        /**
         * @brief 更新车辆物理状态
         * @param deltaTime 仿真时间增量（秒）
         * @param leadingVehicle 前车对象（可为nullptr）
         * @return 是否发生状态变更（用于触发UI更新）
         */
        // bool updatePhysics(float deltaTime, const Vehicle* leadingVehicle, const TrackSegment& trackInfo); // 原签名
        bool updatePhysics(float deltaTime, const Vehicle *leadingVehicle /*, 其他轨道参数如果需要的话 */); // 修改后的签名，移除了TrackSegment

        // 任务操作
        /**
         * @brief 开始执行任务
         * @param task 任务对象引用
         * @param currentDevice 当前所在设备引用
         * @return 是否成功启动任务
         */
        bool startTask(const Task &task, DeviceBase &currentDevice);

        /**
         * @brief 完成当前任务
         * @param targetDevice 目标设备引用
         */
        void completeTask(DeviceBase &targetDevice);

        // 状态查询 (getter)
        std::string getId() const { return m_id; }
        float getSpeed() const { return m_state.currentSpeed; }
        MotionState getMotionState() const { return m_state.motionState; }
        std::string getCurrentTaskId() const { return m_state.currentTask ? m_state.currentTask->id : ""; } // 假设 Task 有 string id
        bool getIsLoaded() const { return m_state.isLoaded; }
        const CargoInfo &getCargoInfo() const { return m_state.cargo; } // 返回引用
        float getBatteryLevel() const { return m_state.batteryLevel; }
        float getTrackPositionMm() const { return m_state.position; } // 新增 getter for raw track position

        // 状态修改 (setter - 根据需要添加，保持封装性)
        // void setMotionState(MotionState newState) { m_state.motionState = newState; }
    };

} // namespace Core