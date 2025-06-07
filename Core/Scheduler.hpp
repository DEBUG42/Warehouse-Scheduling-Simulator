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

    float current_time = 0.0;       // 当前时间      
    float last_debug_time = -1;     // 时间步长
    EventQueue* event_queue_ptr;        // 事件队列
    TaskManager* task_manager_ptr;      // 任务管理器
    VehicleManager* vehicle_manager_ptr;      // 车辆管理器
    DeviceManager* device_manager_ptr;    // 设备管理器
    Logger* logger_ptr;                 // 日志记录器
   
    void bind(TaskManager* tm, VehicleManager* vm, DeviceManager* dm, EventQueue* eq, Logger* lg);

    
    void run(float deltatime);

    void processEvents();

    void handleEvent(const Event& e);

    void updateSystemStates(float deltatime);

    // 尝试分派任务给可用的车辆
    // 输入: 无（依赖于类的成员变量）
    // 输出: 无
    void tryDispatchTasks();

    void addEventForStackerPut(int device_id, int task_id, double current_time);

    void addEventForHumanUnload(int device_id, int task_id, double current_time);

    void addEventForForkliftPut(int device_id, int task_id, double current_time);

    void addEventForStackerPick(int device_id, int task_id, double current_time);
    void addEventForVehiclePickUp(int device_id, int task_id, double current_time) ;

    void addEventForVehiclePutDown(int device_id, int task_id, double current_time) ;

    void vehiclePickUpgoods(int vehicle_id,int device_id,int task_id,double current_time);

    void vehicleUnloadgoods(int vehicle_id,int device_id,int task_id,double current_time);

};


