#include "../Core/Scheduler.hpp"
#include <iostream>
#include <iomanip>

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

//    std::cout << "[INFO] Starting simulation at " << frequency << "Hz..." << std::endl;

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
