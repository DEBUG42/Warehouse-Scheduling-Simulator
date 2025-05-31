#include "../Core/Scheduler.hpp"
#include <iostream>
#include <iomanip>
#include "../Core/Task.hpp"
#include "../Core/Eventqueue.hpp"

void Scheduler::processEvents() {
    // 1. 持续处理当前时间 <= 当前事件时间 的所有事件
    while (!event_queue.empty() && event_queue.peek().time <= current_time) {
        Event event = event_queue.pop();

        // 2. 获取事件触发前的设备状态
        bool before = device_manager.getDeviceState(event.device_id).has_goods;

        // 3. 调用设备管理器处理事件（设备状态更新）
        device_manager.handleEvent(event);  // 改变 has_goods 等状态

        // 4. 获取更新后的设备状态
        bool after = device_manager.getDeviceState(event.device_id).has_goods;

        // 5. 日志记录事件本体（时间、类型、设备等）
        logger.logEvent(event);

        // 6. 若设备状态有变化 → 记录变化日志 + 累积状态时长
        if (before != after) {
            std::string material_id = (event.task_id >= 0)
                ? task_manager.getTask(event.task_id).material_id
                : "SYS";

            logger.logDeviceChange(current_time, event.device_id, material_id, before, after);
            logger.updateDeviceStates(event.device_id, after, current_time);
        }
    }
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

// Scheduler.cpp

void Scheduler::tryDispatchTasks() {
    double current_time = this->current_time;

    // 1. 获取就绪任务（准备好，设备空闲）
    std::vector<Task*> ready_tasks = task_manager.getReadyTasks(current_time, device_manager);

    for (auto* task : ready_tasks) {
        if (task->is_assigned) continue;

        // 2. 获取当前可用的车辆（Stopped 且未分配任务）
        std::vector<Vehicle*> candidates = vehicle_manager.getAvailableVehicles(*task, current_time);
        if (candidates.empty()) continue;

        // 3. 选择最近完成任务的车辆（最早到达起点）
        Vehicle* best_car = vehicle_manager.selectBestVehicle(*task, candidates, current_time);
        if (!best_car) continue;

        // 4. 将任务分配给车辆
        vehicle_manager.applyTaskToVehicle(*best_car, *task, current_time);

        // 5. 更新任务分配状态
        task->is_assigned = true;
        task->assigned_vehicle_id = best_car->id;
        task->assign_time = current_time;

        // 6. 锁定设备（防止任务重入）
        device_manager.reserve(*task, current_time);

        // 7. 写任务调度日志
        logger.logTaskExecution(*task, *best_car);
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
