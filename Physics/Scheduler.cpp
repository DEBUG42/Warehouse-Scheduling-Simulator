#include "../Core/Scheduler.hpp"
#include <iostream>
#include <iomanip>
#include "../Core/Task.hpp"
#include "../Core/Eventqueue.hpp"
#include <SFML/Graphics.hpp>
#define EPSILON 0.0001f
#define PRINT_INTERVAL 1.0f

void Scheduler::bind(TaskManager* tm, VehicleManager* vm, DeviceManager* dm, EventQueue* eq, Logger* lg) {
    task_manager_ptr = tm;
    vehicle_manager_ptr = vm;
    device_manager_ptr= dm;
    event_queue_ptr = eq;
    logger_ptr = lg;
}

void Scheduler::processEvents() {
    // 1. 持续处理当前时间 <= 当前事件时间 的所有事件
    while (!event_queue_ptr->empty() && event_queue_ptr->peek().time <= current_time) {
        Event event = event_queue_ptr->pop();

        // 2. 获取事件触发前的设备状态
        bool before = device_manager_ptr->getDeviceState(event.device_id).has_goods;

        // 3. 调用设备管理器处理事件（设备状态更新）
        // std::cout << "Processing event: "  << " at time " << event.time << std::endl;
        this->handleEvent(event);  // 改变 has_goods 等状态

        // 4. 获取更新后的设备状态
        bool after = device_manager_ptr->getDeviceState(event.device_id).has_goods;

        // 5. 日志记录事件本体（时间、类型、设备等）
        logger_ptr->logEvent(event);

        // 6. 若设备状态有变化 → 记录变化日志 + 累积状态时长
        if (before != after) {
            std::string material_id = (event.task_id >= 0)
                ? task_manager_ptr->getTask(event.task_id).material_id
                 : "SYS";
            std::cout << event.device_id << "号设备是否有货" << device_manager_ptr->getDeviceState(event.device_id).has_goods << std::endl;            
            logger_ptr->logDeviceChange(current_time, event.device_id, material_id, before, after);
            logger_ptr->updateDeviceStates(event.device_id, after, current_time);
        }
    }
}
// 处理传入的事件
// 输入: 事件对象 (const Event& e)
// 输出: 无
void Scheduler::handleEvent(const Event& e) {
    switch (e.type) {
        case EventType::HUMAN_UNLOAD_AT_OUT_PORT:
            // 出库口货物被人工搬空 → 标记为空
            this->device_manager_ptr->getDeviceState(e.device_id).has_goods = false;

            break;

        case EventType::STACKER_PUT_TO_OUT_INTERFACE:
            // 堆垛机已把货物放到接口设备上
            this->device_manager_ptr->getDeviceState(e.device_id).has_goods = true;

            break;

        case EventType::FORKLIFT_PUT_TO_IN_PORT:
            // 入库口叉车放货完成
            this->device_manager_ptr->getDeviceState(e.device_id).has_goods = true;

            break;

        case EventType::STACKER_PICK_FROM_IN_INTERFACE:
            this->device_manager_ptr->getDeviceState(e.device_id).has_goods = false;

            break;

        case EventType::VEHICLE_PICK_UP_GOODS: {
            // 车辆从设备取货 → 设备变为空，车辆变为已装载
            auto& device = this->device_manager_ptr->getDeviceState(e.device_id);
            device.has_goods = false;
            device.is_reserved = false;
            
            Task& task = this->task_manager_ptr->getTask(e.task_id);
            
            Vehicle& vehicle = this->vehicle_manager_ptr->getVehicleByTaskId(e.task_id);           
            vehicle.is_loaded = true;
            vehicle.towards_device = task.end_device_id;



            vehicle.m_state.motionState = Vehicle::MotionState::Accelerating;
            std::cout << "[Event] VEHICLE_PICK_UP_GOODS: Vehicle #" << vehicle.id 
                      << " picked up goods from Device #" << e.device_id << std::endl;
            break;
        }

        case EventType::VEHICLE_PUT_DOWN_GOODS: {
            // 车辆向设备放货 → 设备变有货，车辆清空任务
            auto& device = this->device_manager_ptr->getDeviceState(e.device_id);
            device.has_goods = true;
            device.is_reserved = false;

            Vehicle& vehicle = this->vehicle_manager_ptr->getVehicleByTaskId(e.task_id);
            vehicle.is_loaded = false;
            vehicle.m_state.currentTask = nullptr;
            vehicle.towards_device = -1;
            vehicle.m_state.motionState = Vehicle::MotionState::Accelerating;

            Task& task = this->task_manager_ptr->getTask(e.task_id);
            task.complete_time = this->current_time;

            std::cout << "[Event] VEHICLE_PUT_DOWN_GOODS: Vehicle #" << vehicle.id 
                      << " dropped goods at Device #" << e.device_id 
                      << " → Task #" << e.task_id << " completed.\n";
            break;
        }

        default:
            break;
    }
}

// 更新系统状态
void Scheduler::updateSystemStates(float dt) {
    auto& vehicles = vehicle_manager_ptr->getAllVehicles();
    size_t n = vehicles.size();

    for (size_t i = 0; i < n; ++i) {
        Vehicle* self = &vehicles[i];
        Vehicle* front = &vehicles[(i + n - 1) % n]; // 环形选择前车
        VehicleManager::VehicleUpdateResult result = this->vehicle_manager_ptr->updateVehicle(current_time, dt, self, front);
        if (result.trigger == VehicleEventTrigger::PickUpArrived) {
            addEventForVehiclePickUp(result.device_id, result.task_id, current_time);
    }
        else if (result.trigger == VehicleEventTrigger::PutDownArrived) {
            addEventForVehiclePutDown(result.device_id, result.task_id, current_time);
}
    }

    auto device_events = device_manager_ptr->update(current_time);
    for (const auto& result : device_events) {
        switch (result.trigger) {
            case DeviceManager::DeviceEventTrigger::ForkliftPutToInPort:
                addEventForForkliftPut(result.device_id, result.task_id, current_time);
                // std::cout <<"叉车在入库口放货"<<std::endl;
                break;
            case DeviceManager::DeviceEventTrigger::HumanUnloadAtOutPort:
                addEventForHumanUnload(result.device_id, result.task_id, current_time);
                // std::cout <<"人工在出库口取货"<<std::endl;
                break;
                addEventForStackerPick(result.device_id, result.task_id, current_time);
                // std::cout <<"堆垛机从入库接口取货"<<std::endl;
                break;
                addEventForStackerPut(result.device_id, result.task_id, current_time);
                // std::cout <<"堆垛机在出库接口放货"<<std::endl;
                break;
            default:
                break;
        }
}

}

// 添加：堆垛机在出库口放货（出库接口）
void Scheduler::addEventForStackerPut(int device_id, int task_id, double current_time) {
    Event e;
    e.time = current_time + 50.0;
    e.type = EventType::STACKER_PUT_TO_OUT_INTERFACE;
    e.device_id = device_id;
    e.task_id = task_id;
    event_queue_ptr->addEvent(e);
}

// 添加：人工在出库接口取货（出库口）
void Scheduler::addEventForHumanUnload(int device_id, int task_id, double current_time) {
    Event e;
    e.time = current_time + 30.0;
    e.type = EventType::HUMAN_UNLOAD_AT_OUT_PORT;
    e.device_id = device_id;
    e.task_id = task_id;
    event_queue_ptr->addEvent(e);
}

// 添加：叉车完成入库放货(入库口)
void Scheduler::addEventForForkliftPut(int device_id, int task_id, double current_time) {
    Event e;
    e.time = current_time + 30.0;
    e.type = EventType::FORKLIFT_PUT_TO_IN_PORT;
    e.device_id = device_id;
    e.task_id = task_id;
    event_queue_ptr->addEvent(e);
}

// 添加：堆垛机从入库接口取货（入库接口）
void Scheduler::addEventForStackerPick(int device_id, int task_id, double current_time) {
    Event e;
    e.time = current_time + 25.0;
    e.type = EventType::STACKER_PICK_FROM_IN_INTERFACE;
    e.device_id = device_id;
    e.task_id = task_id;
    event_queue_ptr->addEvent(e);
}
void Scheduler::addEventForVehiclePickUp(int device_id, int task_id, double current_time) {
    Event e;
    e.time = current_time + 7.5;  // 稍后立即触发，也可以根据需要设定延迟
    e.type = EventType::VEHICLE_PICK_UP_GOODS;
    e.device_id = device_id;
    e.task_id = task_id;
    this->event_queue_ptr->addEvent(e);

    std::cout << "[Event] Scheduled VEHICLE_PICK_UP_GOODS for Task #" 
              << task_id << " at Device #" << device_id 
              << " (TriggerTime: " << e.time << ")\n";
}
void Scheduler::addEventForVehiclePutDown(int device_id, int task_id, double current_time) {
    Event e;
    e.time = current_time + 7.5;  // 稍后立即触发，也可以加延迟
    e.type = EventType::VEHICLE_PUT_DOWN_GOODS;
    e.device_id = device_id;
    e.task_id = task_id;
    this->event_queue_ptr->addEvent(e);

    std::cout << "[Event] Scheduled VEHICLE_PUT_DOWN_GOODS for Task #" 
              << task_id << " at Device #" << device_id 
              << " (TriggerTime: " << e.time << ")\n";
}


void Scheduler::tryDispatchTasks() {
    double current_time = this->current_time;

    
    // ✅ 1. 获取所有就绪任务
    std::vector<Task*> ready_tasks = task_manager_ptr->getReadyTasks(current_time, *device_manager_ptr);
    // std::cout << "[Dispatch] " << ready_tasks.size() << " ready tasks at time " << current_time << "\n";
    for (auto* task : ready_tasks) {
        if (task->is_assigned) continue;

        // ✅ 2. 找出当前可用的车辆(测试后可用)
        std::vector<Vehicle*> candidates = vehicle_manager_ptr->getAvailableVehicles(*task, current_time);
        if (candidates.empty()) {
            // std::cout << "[Dispatch] No vehicle available for Task #" << task->id << " at time " << current_time << "\n";
            continue;
        }
        else {
            std::cout << "[Dispatch] " << candidates.size() << " vehicles available for Task #" << task->id << " at time " << current_time << "\n";
        }
        // ✅ 3. 从可选车辆中选择最优
        Vehicle* best_vehicle = vehicle_manager_ptr->selectBestVehicle(*task, candidates, current_time);
        if (!best_vehicle) {
            std::cout << "[Dispatch] No best vehicle selected for Task #" << task->id << "\n";
            continue;
        }

        // ✅ 4. 分配任务
        vehicle_manager_ptr->applyTaskToVehicle(*best_vehicle, *task, current_time);

        // ✅ 5. 更新任务状态
        task->is_assigned = true;
        task->assigned_vehicle_id = best_vehicle->id;
        task->assign_time = current_time;

        // ✅ 6. 锁定设备，防止设备被重复分配
        device_manager_ptr->reserve(*task, current_time);

        // // ✅ 7. 写入日志
        // logger_manager->logTaskExecution(const Task& task, const Vehicle& vehicle);

        // std::cout << "[Dispatch] Assigned Task #" << task->id 
        //           << " to Vehicle #" << best_vehicle->id 
        //           << " at time " << current_time << "\n";
    }
}

// 主运行循环
void Scheduler::run(float deltatime) {
    if (!task_manager_ptr->allTasksCompleted()) {

            // 执行一次模拟步进的逻辑
            processEvents();
            updateSystemStates(deltatime);
            tryDispatchTasks();

            // 打印模拟时间日志
            // 假设你希望每模拟 1 秒 (即 1.0 / dt 步) 打印一次
                 // 判断是否到了新的秒刻点，并且需要进行输出
    if (current_time >= this->last_debug_time + PRINT_INTERVAL - EPSILON) {
                std::cout << "[SimTime] " << std::fixed << std::setprecision(2) << current_time << "s" << std::endl;

        // 更新这辆车上一次输出调试信息的时间
        this->last_debug_time = current_time;
    }
                
            // 更新模拟时间
            current_time += deltatime;

        	}

}
