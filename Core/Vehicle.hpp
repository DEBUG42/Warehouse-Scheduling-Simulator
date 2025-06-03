#pragma once
#include "Device.hpp"
#include <vector>
#include <iostream>
#include <iomanip>
#include <memory>
//#include "Scheduler.hpp"
#include "Task.hpp"
#include "Event.hpp"
#include <SFML/Graphics.hpp>


// 定义车辆类
class Vehicle {
public:

// 定义车辆状态枚举
    enum class VehicleState {
        IDLE,           // 空闲状态
        MOVING_TO_PICK, // 移动到取货位置
        PICKING,        // 取货状态
        MOVING_TO_DROP, // 移动到卸货位置
        DROPPING        // 卸货状态
};
    // 运动状态枚举
    enum class MotionState {
        Accelerating,  // 加速阶段
        Cruising,      // 匀速阶段
        Decelerating,  // 减速阶段
        Stopped        // 静止状态
    };



    // 固有属性
    int id;                      // 车辆编号
    float m_length = 2;            // 车辆长度（米）
    float m_maxStraightSpeed = (8 / 3); // 直轨最大速度（米/秒）
    float m_maxCurveSpeed = (2 / 3) ;     // 弯轨最大速度（米/秒）
    float m_acceleration = 0.5 ;       // 加减速度（米/秒²）
	float m_loadTime=7.5; // 装卸货时间（秒）

    int towards_device;          // 要前往的设备编号
    float position_m;            // 在轨道上的位置
    float next_available_time;   // 下一次空车时间
    bool is_executing;           // 是否正在执行任务
    bool is_loaded;              // 是否装载货物
    float velocity_mps;          // 当前速度（米/秒）
    float max_speed;             // 最大速度（米/秒）
    float target_position;       // 目标位置（米/秒）

    // 动态状态
    struct {
        float position;               // 轨道位置（0~trackLength）
        float currentSpeed;           // 当前速度（米/秒）
        MotionState motionState;      // 当前运动状态
        Task* currentTask = nullptr; // 当前执行的任务
        float operationTimer=0.0f;     // 装卸货操作计时器
    } m_state;
};

// 储存和管理车辆的类
class VehicleManager {
public:
    // 初始化指定数量的车辆
    // 输入: 车辆数量 (int count)
    // 输出: 无
    void initializeVehicles(int count);

    
    static std::string motionStateToString(Vehicle::MotionState state); 

    // 更新所有车辆的状态
    // 输入: 当前时间 (double current_time), 时间步长 (double dt)
    // 输出: 无
    //void updateAllVehicles(double current_time, double dt,);

	//更新单个车辆的状态
	//输入：当前时间（double current_time），时间步长（double dt）时间倍率（Timescal）
	//		当前车辆，前一辆车
    void updateVehicle(float current_time,float deltaTime, Vehicle* vehicle, Vehicle* leadingVehicle);


    // 获取可用于执行指定任务的车辆列表
    // 输入: 任务对象 (const Task& task), 当前时间 (double current_time)
    // 输出: 可用车辆列表 (std::vector<Vehicle*>)
    std::vector<Vehicle*> getAvailableVehicles(Task& task, double current_time);

    // 选择最适合执行指定任务的车辆
    // 输入: 任务对象 (const Task& task), 候选车辆列表 (const std::vector<Vehicle*>& candidates), 当前时间 (double current_time)
    // 输出: 最佳车辆 (Vehicle*)
    Vehicle* selectBestVehicle(Task& task, std::vector<Vehicle*>& candidates, double current_time);

    // 将任务分配给指定的车辆
    // 输入: 车辆对象 (Vehicle& vehicle), 任务对象 (Task& task), 当前时间 (double current_time)
    // 输出: 无
    void applyTaskToVehicle(Vehicle& vehicle, Task& task, double current_time);

    // 返回所有车辆的常量引用
    // 输入: 无
    // 输出: 所有车辆的列表 (const std::vector<Vehicle>&)
    std::vector<Vehicle>& getAllVehicles();

public:
    std::vector<Vehicle> vehicles; // 定义 vehicles 容器
    constexpr static double LOOP_LENGTH = 99.47787445225672;  // 环道总长度（可调）

    // 计算两个位置之间的距离
    // 输入: 起始位置 (double from), 结束位置 (double to)
    // 输出: 距离 (double)
    double getDistance(double from, double to);
};
 //新增：将 MotionState 转换为字符串的辅助函数
// static std::string motionStateToString(Vehicle::MotionState state) {
//     switch (state) {
//         case Vehicle::MotionState::Accelerating: return "Accelerating";
//         case Vehicle::MotionState::Cruising:     return "Cruising";
//         case Vehicle::MotionState::Decelerating: return "Decelerating";
//         case Vehicle::MotionState::Stopped:      return "Stopped";
//         default:                        return "Unknown";
//     }
// }