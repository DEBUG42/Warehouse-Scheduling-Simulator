// Scheduler 类（主循环控制器）
#pragma once
#include "Task.hpp"
#include "Device.hpp"
#include "Logger.hpp"
#include "EventQueue.hpp"
#include "Vehicle.hpp"

// 定义调度器类
class Scheduler {
public:
    // 主入口函数，运行调度器循环
    // 输入: 运行频率 (double frequency)，默认为 100.0
    // 输出: 无
    void run(double frequency = 100.0,float timescale=1.0);

public:
    float current_time = 0.0;       // 当前时间
    float dt = 1.0 / 100.0;         // 时间步长

    EventQueue event_queue;        // 事件队列
    TaskManager task_manager;      // 任务管理器
    VehicleManager vehicle_manager;      // 车辆管理器
    DeviceManager device_manager;    // 设备管理器
    Logger logger;                 // 日志记录器

    // 主循环阶段
    // 处理事件队列中的事件
    // 输入: 无
    // 输出: 无
    void processEvents();

    // 推进车辆与设备状态
    // 输入: 无
    // 输出: 无
    void updateSystemStates();

    // 尝试分派任务给可用的车辆
    // 输入: 无（依赖于类的成员变量）
    // 输出: 无
    void tryDispatchTasks();

    void addEventForStackerPut(int device_id, int task_id, double current_time);

    void addEventForHumanUnload(int device_id, int task_id, double current_time);

    void addEventForForkliftPut(int device_id, int task_id, double current_time);

    void addEventForStackerPick(int device_id, int task_id, double current_time);


};


