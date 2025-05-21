#pragma once
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
	
    // 构造函数
	/**
	 * @brief 构造函数
	 * @param position
	 * @param currentSpeed
	 * @param motionState
	 **/		
	InitVehicle(float position, float currentSpeed, MotionState motionStateposition){
		Vehicle.m_state.position = position;
		Vehicle.m_state.currentSpeed = currentSpeed;
		Vehicle.m_state.motionState = motionState;
		Vehicle.m_state.currentTask = nullptr;
		Vehicle.m_state.operationTimer.restart();
	}

public:
    // 固有属性
    float m_length=0.02;            // 车辆长度（米）
    float m_maxStraightSpeed=(8/3)*TimeScale();   // 直轨最大速度（米/秒）
    float m_maxCurveSpeed=(2/3)*TimeScale();     // 弯轨最大速度（米/秒）
    float m_acceleration=0.5*TimeScale()*TimeScale();       // 加减速度（米/秒²）
	float m_loadTime=7.5/TimeScale(); // 装卸货时间（秒）

    // 动态状态
    struct {
        float position;               // 轨道位置（0~trackLength）
        float currentSpeed;           // 当前速度（米/秒）
        MotionState motionState;      // 当前运动状态
        const Task* currentTask = nullptr; // 当前执行的任务
        sf::Clock operationTimer;     // 装卸货操作计时器
    } m_state;


}
