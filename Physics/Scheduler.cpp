#include "../Core/Scheduler.hpp"
#include <iostream>
#include <iomanip>
#include "../Core/Task.hpp"
#include "../Core/Eventqueue.hpp"
// 处理事件队列中的事件
void Scheduler::processEvents() {
    while (!event_queue.empty() && event_queue.peek().time <= current_time) {
        Event event = event_queue.pop();
        device_manager.handleEvent(event);
        logger.logEvent(event);
    }
}

// 主运行循环
void Scheduler::run(double frequency, float timescale) {
    dt = 1.0 / frequency * timescale;
    current_time = 0.0; 
    // std::cout << "这里表示所有任务是否已完成"<<task_manager.allTasksCompleted() << std::endl;
//  std::cout << "[INFO] Starting simulation at " << frequency << "Hz..." << std::endl;

    int step_count = 0;
    while (!task_manager.allTasksCompleted()) {
        processEvents();
        updateSystemStates();
        tryDispatchTasks();

        if (step_count % int(frequency) == 0) {
            std::cout << "[SimTime] " << std::fixed << std::setprecision(2) << current_time << "s" << std::endl;
        }

        current_time += dt;
        step_count++;
    }

//    std::cout << "[INFO] Simulation finished at time " << current_time << "s" << std::endl;
}

// 更新系统状态
void Scheduler::updateSystemStates() {
    auto& vehicle_list = vehicle_manager.getVehicles();
    if(vehicle_list.size() >= 3) {
        vehicle_manager.updateVehicle(current_time, dt, &vehicle_list[0], &vehicle_list[2]);
        vehicle_manager.updateVehicle(current_time, dt, &vehicle_list[1], &vehicle_list[0]);
        vehicle_manager.updateVehicle(current_time, dt, &vehicle_list[2], &vehicle_list[1]);
    }
    device_manager.update(current_time);
}

// 添加：堆垛机搬运完成事件（出库接口）
void Scheduler::addEventForStackerPut(int device_id, int task_id, double current_time) {
    Event e;
    e.time = current_time + 50.0;
    e.type = EventType::STACKER_PUT_TO_OUT_INTERFACE;
    e.device_id = device_id;
    e.task_id = task_id;
    event_queue.addEvent(e);
}

// 添加：人工卸货完成事件（出库口）
void Scheduler::addEventForHumanUnload(int device_id, int task_id, double current_time) {
    Event e;
    e.time = current_time + 30.0;
    e.type = EventType::HUMAN_UNLOAD_AT_OUT_PORT;
    e.device_id = device_id;
    e.task_id = task_id;
    event_queue.addEvent(e);
}

// 添加：叉车完成入库放货
void Scheduler::addEventForForkliftPut(int device_id, int task_id, double current_time) {
    Event e;
    e.time = current_time + 30.0;
    e.type = EventType::FORKLIFT_PUT_TO_IN_PORT;
    e.device_id = device_id;
    e.task_id = task_id;
    event_queue.addEvent(e);
}

// 添加：堆垛机从接口取货（入库）
void Scheduler::addEventForStackerPick(int device_id, int task_id, double current_time) {
    Event e;
    e.time = current_time + 25.0;
    e.type = EventType::STACKER_PICK_FROM_IN_INTERFACE;
    e.device_id = device_id;
    e.task_id = task_id;
    event_queue.addEvent(e);
}
