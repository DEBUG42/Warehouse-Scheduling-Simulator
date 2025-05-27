// Scheduler 类（主循环控制器）
#pragma once
#include "Task.hpp"
#include "Vehicle.hpp"
#include "Device.hpp"
#include "Logger.hpp"
#include "EventQueue.hpp"

// 定义调度器类
class Scheduler {
public:
    // 主入口函数，运行调度器循环
    // 输入: 运行频率 (double frequency)，默认为 100.0
    // 输出: 无
    void run(double frequency = 100.0);

private:
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
};

// 处理事件队列中的事件
// 输入: 无
// 输出: 无
void Scheduler::processEvents() {
    while (!event_queue.empty() && event_queue.peek().time <= current_time) {
        Event event = event_queue.pop();

        // 调用设备管理器处理事件（改变 has_goods、is_transferring 等）
        device_manager.handleEvent(event);

        // 记录事件日志
        logger.logEvent(event);
    }
}

// 推进车辆与设备状态
// 输入: 无
// 输出: 无
void Scheduler::updateSystemStates() {
    // 推进车辆位置、状态、完成判断等（支持加减速模型）
    
	std::vector<Vehicle*> vehicle_list = vehicle_manager.getVehicles();
	vehicle_manager.updateVehicle(Timescale,current_time,dt, vehicle_list[0],vehicle_list[2]);
	vehicle_manager.updateVehicle(Timescale,current_time,dt, vehicle_list[1],vehicle_list[0]);
	vehicle_manager.updateVehicle(Timescale,current_time,dt, vehicle_list[2],vehicle_list[1]);
	
    // 检查设备超时释放、状态推进等
    device_manager.update(current_time);
}
