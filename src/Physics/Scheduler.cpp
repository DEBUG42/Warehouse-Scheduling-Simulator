#include "../Core/Scheduler.hpp"
#include <iostream>
#include <iomanip>

// 处理事件队列中的事件
void Scheduler::processEvents()
{
    while (!event_queue.empty() && event_queue.peek().time <= current_time)
    {
        Event event = event_queue.pop();
        device_manager.handleEvent(event);
        logger.logEvent(event);
    }
}

// 主运行循环
void Scheduler::run(double frequency, float timescale)
{
    dt = 1.0 / frequency * timescale;
    current_time = 0.0;

    //    std::cout << "[INFO] Starting simulation at " << frequency << "Hz..." << std::endl;

    int step_count = 0;
    while (!task_manager.allTasksCompleted())
    {
        processEvents();
        updateSystemStates();
        tryDispatchTasks();

        if (step_count % int(frequency) == 0)
        {
            std::cout << "[SimTime] " << std::fixed << std::setprecision(2) << current_time << "s" << std::endl;
        }

        current_time += dt;
        step_count++;
    }

    //    std::cout << "[INFO] Simulation finished at time " << current_time << "s" << std::endl;
}

// 更新系统状态
void Scheduler::updateSystemStates()
{
    auto &vehicle_list = vehicle_manager.getVehicles();
    if (vehicle_list.size() >= 3)
    {
        vehicle_manager.updateVehicle(current_time, dt, &vehicle_list[0], &vehicle_list[2]);
        vehicle_manager.updateVehicle(current_time, dt, &vehicle_list[1], &vehicle_list[0]);
        vehicle_manager.updateVehicle(current_time, dt, &vehicle_list[2], &vehicle_list[1]);
    }
    device_manager.update(current_time);
}

// 尝试分派任务
void Scheduler::tryDispatchTasks()
{
    auto ready_tasks = task_manager.getReadyTasks(current_time, device_manager);

    for (Task *task : ready_tasks)
    {
        if (task->is_assigned)
            continue;

        auto candidates = vehicle_manager.getAvailableVehicles(*task, current_time);
        if (candidates.empty())
            continue;

        Vehicle *best_vehicle = vehicle_manager.selectBestVehicle(*task, candidates, current_time);
        if (!best_vehicle)
            continue;

        // 分配任务
        vehicle_manager.applyTaskToVehicle(*best_vehicle, *task, current_time);
        task_manager.markTaskAssigned(task->id, best_vehicle->id, current_time);
        device_manager.reserve(task->start_device_id, task->id, current_time + 5.0); // 假设锁定5s

        // 如果任务为出库任务，添加"人工卸货完成"延迟事件
        if (task->type == TaskType::OUTBOUND)
        {
            Event e{
                .time = current_time + 30.0,
                .type = EventType::DEVICE_BECOMES_EMPTY,
                .device_id = task->end_device_id,
                .task_id = task->id};
            event_queue.addEvent(e);
        }

        // 日志记录任务分配情况
        logger.logTaskAssignment(*task, *best_vehicle);
    }
}

// 获取车辆列表（为GUI提供访问）
const std::vector<Vehicle> &Scheduler::getVehicles() const
{
    return vehicle_manager.getVehicles();
}

// 获取设备列表（为GUI提供访问）
const std::vector<DeviceBase> &Scheduler::getDevices() const
{
    return device_manager.getDevices();
}

// 设置仿真模式
void Scheduler::setSimulationMode(SimulationMode mode)
{
    m_currentMode = mode;

    // 根据不同模式配置仿真参数
    switch (mode)
    {
    case SimulationMode::TASK1:
        std::cout << "[Scheduler] 模式切换到 TASK1: 基础任务调度" << std::endl;
        // TODO: 设置 TASK1 特定的调度参数
        break;
    case SimulationMode::TASK2_1:
        std::cout << "[Scheduler] 模式切换到 TASK2.1: 优化调度算法 v1" << std::endl;
        // TODO: 设置 TASK2.1 特定的调度参数
        break;
    case SimulationMode::TASK2_2:
        std::cout << "[Scheduler] 模式切换到 TASK2.2: 优化调度算法 v2" << std::endl;
        // TODO: 设置 TASK2.2 特定的调度参数
        break;
    case SimulationMode::TASK2_3:
        std::cout << "[Scheduler] 模式切换到 TASK2.3: 高级调度算法" << std::endl;
        // TODO: 设置 TASK2.3 特定的调度参数
        break;
    }
}

// 获取当前仿真模式
SimulationMode Scheduler::getCurrentSimulationMode() const
{
    return m_currentMode;
}
