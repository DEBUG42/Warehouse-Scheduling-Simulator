//CarManager 类（车辆推进器 + 调度支持）
#pragma once
#include <SFML/Graphics.hpp>
#include <map>
#include "Task.hpp"
#include "Device.hpp"
#include <vector>


class Vehicle {
public:
    // 运动状态枚举
    enum class MotionState {
        Accelerating,  // 加速阶段
        Cruising,      // 匀速阶段
        Decelerating,  // 减速阶段
        Stopped        // 静止状态
    };
	
    // 构造函数
	/**
	 * @brief 构造函数
	 * @param position
	 * @param currentSpeed
	 * @param motionState
	 **/		
	Vehicle(float position, float currentSpeed, MotionState motionStateposition){
		Vehicle.m_state.position = position;
		Vehicle.m_state.currentSpeed = currentSpeed;
		Vehicle.m_state.motionState = motionState;
		Vehicle.m_state.currentTask = nullptr;
		Vehicle.m_state.operationTimer.restart();
	}

public:
    // 固有属性
    int id;                      // 车辆编号
    float m_length = 0.02;            // 车辆长度（米）
    float m_maxStraightSpeed = (8 / 3) * TimeScale();   // 直轨最大速度（米/秒）
    float m_maxCurveSpeed = (2 / 3) * TimeScale();     // 弯轨最大速度（米/秒）
    float m_acceleration = 0.5 * TimeScale() * TimeScale();       // 加减速度（米/秒²）

    int towards_device;          // 要前往的设备编号
    float position_m;           // 在轨道上的位置
    float  next_available_time;         // 下一次空车时间
    bool is_executing;           // 是否正在执行任务
    bool is_loaded;              // 是否装载货物

    // 动态状态
    struct {
        float position;               // 轨道位置（0~trackLength）
        float currentSpeed;           // 当前速度（米/秒）
        MotionState motionState;      // 当前运动状态
        const Task* currentTask = nullptr; // 当前执行的任务
        float operationTimer;     // 装卸货操作计时器
    } m_state;


    Vehicle(int id, int current_device, double position_m, double available_time, bool is_executing, bool is_loaded,
            float position, float currentSpeed, MotionState motionState) :
            id(id), current_device(current_device), position_m(position_m), available_time(available_time),
            is_executing(is_executing), is_loaded(is_loaded) {
        m_state.position = position;
        m_state.currentSpeed = currentSpeed;
        m_state.motionState = motionState;
        m_state.currentTask = nullptr;
        m_state.operationTimer.restart();
    }
};


//储存函数的车辆管理类
class VehicleManager {
public:
    void initializeCars(int count);
    void updateAllCars(double current_time, double dt); // 高频推进车辆
    std::vector<Car*> getAvailableCars(const Task& task, double current_time);
    Car* selectBestCar(const Task& task, const std::vector<Car*>&, double current_time);
    void applyTaskToCar(Car& car, const Task& task, double current_time);

private:
    std::vector<Car> cars;
    void applyMotion(Car& car, double dt); // 推进位置+速度
};
