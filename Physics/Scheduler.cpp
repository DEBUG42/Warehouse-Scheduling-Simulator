#include "../Core/Scheduler.hpp"
#include <iostream>
#include <iomanip>
#include "../Core/Task.hpp"
#include "../Core/Eventqueue.hpp"
#include <SFML/Graphics.hpp>


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
        device_manager_ptr->handleEvent(event);  // 改变 has_goods 等状态

        // 4. 获取更新后的设备状态
        bool after = device_manager_ptr->getDeviceState(event.device_id).has_goods;

        // 5. 日志记录事件本体（时间、类型、设备等）
        logger_ptr->logEvent(event);

        // 6. 若设备状态有变化 → 记录变化日志 + 累积状态时长
        if (before != after) {
            std::string material_id = (event.task_id >= 0)
                ? task_manager_ptr->getTask(event.task_id).material_id
                 : "SYS";

            logger_ptr->logDeviceChange(current_time, event.device_id, material_id, before, after);
            logger_ptr->updateDeviceStates(event.device_id, after, current_time);
        }
    }
}

// 更新系统状态
void Scheduler::updateSystemStates() {
    auto& vehicles = vehicle_manager_ptr->getAllVehicles();
    size_t n = vehicles.size();

    for (size_t i = 0; i < n; ++i) {
        Vehicle* self = &vehicles[i];
        Vehicle* front = &vehicles[(i + n - 1) % n]; // 环形选择前车
        vehicle_manager_ptr->updateVehicle(current_time, dt, self, front);
    }

    device_manager_ptr->update(current_time);
}

// 添加：堆垛机搬运完成事件（出库接口）
void Scheduler::addEventForStackerPut(int device_id, int task_id, double current_time) {
    Event e;
    e.time = current_time + 50.0;
    e.type = EventType::STACKER_PUT_TO_OUT_INTERFACE;
    e.device_id = device_id;
    e.task_id = task_id;
    event_queue_ptr->addEvent(e);
}

// 添加：人工卸货完成事件（出库口）
void Scheduler::addEventForHumanUnload(int device_id, int task_id, double current_time) {
    Event e;
    e.time = current_time + 30.0;
    e.type = EventType::HUMAN_UNLOAD_AT_OUT_PORT;
    e.device_id = device_id;
    e.task_id = task_id;
    event_queue_ptr->addEvent(e);
}

// 添加：叉车完成入库放货
void Scheduler::addEventForForkliftPut(int device_id, int task_id, double current_time) {
    Event e;
    e.time = current_time + 30.0;
    e.type = EventType::FORKLIFT_PUT_TO_IN_PORT;
    e.device_id = device_id;
    e.task_id = task_id;
    event_queue_ptr->addEvent(e);
}

// 添加：堆垛机从接口取货（入库）
void Scheduler::addEventForStackerPick(int device_id, int task_id, double current_time) {
    Event e;
    e.time = current_time + 25.0;
    e.type = EventType::STACKER_PICK_FROM_IN_INTERFACE;
    e.device_id = device_id;
    e.task_id = task_id;
    event_queue_ptr->addEvent(e);
}

// Scheduler.cpp

void Scheduler::tryDispatchTasks() {
    double current_time = this->current_time;

    
    // ✅ 1. 获取所有就绪任务
    std::vector<Task*> ready_tasks = task_manager_ptr->getReadyTasks(current_time, *device_manager_ptr);
    // std::cout << "[Dispatch] " << ready_tasks.size() << " ready tasks at time " << current_time << "\n";
    for (auto* task : ready_tasks) {
        if (task->is_assigned) continue;

        // ✅ 2. 找出当前可用的车辆
        std::vector<Vehicle*> candidates = vehicle_manager_ptr->getAvailableVehicles(*task, current_time);
        if (candidates.empty()) {
            std::cout << "[Dispatch] No vehicle available for Task #" << task->id << " at time " << current_time << "\n";
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
void Scheduler::run(double frequency, float timescale) {
    // dt = 1.0 / frequency * timescale;
    dt = 0.01f;
    current_time = 0.0; 


    sf::Clock updateClock;


    std::cout << "这里表示进入了run循环哦"<< std::endl;
    std::cout << "这里表示所有任务是否已完成"<<task_manager_ptr->allTasksCompleted() << std::endl;

    int step_count = 0;
    while (!task_manager_ptr->allTasksCompleted()) {


        if (updateClock.getElapsedTime().asSeconds() >= dt) {
            // 如果时间已到，立即重置时钟，为下一个 dt 步长计时
            updateClock.restart(); 
            
            // 执行一次模拟步进的逻辑
            processEvents();
            updateSystemStates();
            tryDispatchTasks();

            // 打印模拟时间日志
            // 假设你希望每模拟 1 秒 (即 1.0 / dt 步) 打印一次
            if (step_count % int(1.0 / dt) == 0) { 
                std::cout << "[SimTime] " << std::fixed << std::setprecision(2) << current_time << "s" << std::endl;
                std::cout <<"这里是第一辆车的相关信息"<<vehicle_manager_ptr->getAllVehicles()[0].position_m << std::endl;
                std::cout <<"加速度状态"<< vehicle_manager_ptr->motionStateToString(vehicle_manager_ptr->getAllVehicles()[0].m_state.motionState) <<std::endl;
                std::cout <<"速度状态"<< vehicle_manager_ptr->getAllVehicles()[0].m_state.currentSpeed <<std::endl;
                std::cout <<"当前任务"<< vehicle_manager_ptr->getAllVehicles()[0].m_state.currentTask->id<<std::endl;


                std::cout <<"这里是第二辆车的相关信息"<< vehicle_manager_ptr->getAllVehicles()[1].position_m <<std::endl;
                std::cout <<"加速度状态"<< vehicle_manager_ptr->motionStateToString(vehicle_manager_ptr->getAllVehicles()[1].m_state.motionState) <<std::endl;
                std::cout <<"速度状态"<< vehicle_manager_ptr->getAllVehicles()[1].m_state.currentSpeed <<std::endl;
                std::cout <<"当前任务"<< vehicle_manager_ptr->getAllVehicles()[1].m_state.currentTask->id <<std::endl;
                
                std::cout <<"这里是第三辆车的相关信息"<< vehicle_manager_ptr->getAllVehicles()[2].position_m <<std::endl;
                std::cout <<"加速度状态"<< vehicle_manager_ptr->motionStateToString(vehicle_manager_ptr->getAllVehicles()[2].m_state.motionState) <<std::endl; 
                std::cout <<"速度状态"<< vehicle_manager_ptr->getAllVehicles()[2].m_state.currentSpeed <<std::endl;
                std::cout <<"当前任务"<< vehicle_manager_ptr->getAllVehicles()[2].m_state.currentTask->id <<std::endl;
                
                // std::cout <<"这里是任务的执行情况"
            }

            //

            // 更新模拟时间并增加步数
            current_time += dt;
            step_count++;
        }	}


//    std::cout << "[INFO] Simulation finished at time " << current_time << "s" << std::endl;
}
