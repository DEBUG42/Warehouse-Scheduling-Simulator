// #include "Physics/TaskManager.hpp"
// #include "Physics/DeviceManager.hpp"
// #include "Physics/AssignTaskToVehicles.hpp"
#include "Core/Logger.hpp"
#include "Core/Scheduler.hpp"
#include <iostream>
#include <cassert>
#include "Core/Task.hpp"


int main() {
    //这里是在实例化对象
    EventQueue eq;
    double now = 0.0;
    Scheduler scheduler;
    TaskManager task_manager;
    EventQueue eventqueue;
    scheduler.vehicle_manager->initializeVehicles(3);
    scheduler.device_manager->initializeDevices();
    auto& vehicles = scheduler.vehicle_manager->getVehicles();
    //while(1){
    // scheduler.run(100);

// 正确获取三辆车的信息
if (vehicles.size() >= 3) {
if (vehicles[0].m_state.motionState==Vehicle::MotionState::Stopped)
    std::cout << "第一辆车停车" << std::endl;

} 
    //这里是在测试读取文件是否正常
    task_manager.loadFromFile("tasks.csv");


    //这里是在验证访问容器函数是否正常
    std::vector<Task>& tasks = task_manager.getAllTasks();

    

    //这里是在测试任务容器是否已经赋值
    std::cout << "第7个任务的终止设备编号："<< tasks[6].end_device_id << std::endl;
    std::cout << "第8个任务的起始设备编号："<< tasks[7].start_device_id << std::endl;
    std::cout << "第9个任务的终止设备编号："<< tasks[8].material_id << std::endl;
    std::cout << "第10个任务的出入库任务类型：" << task_manager.taskTypeToString(tasks[9].type) << std::endl;


    //这里是在测试是否还有可用任务
    std::cout << "这里表示所有任务是否已完成"<<task_manager.allTasksCompleted() << std::endl;


//     //这里是在测试事件系统是否可以正常工作
//     // 模拟三个事件插入
//     eq.addEvent({now + 10, EventType::STACKER_PUT_TO_OUT_INTERFACE, 2, 1});
//     eq.addEvent({now + 5, EventType::HUMAN_UNLOAD_AT_OUT_PORT, 13, 1});
//     eq.addEvent({now + 15, EventType::FORKLIFT_PUT_TO_IN_PORT, 16, 2});

//     std::cout << "=== [事件系统测试开始] ===\n";

//     for (double t = 0; t <= 20.0; t += 1.0) {
//         std::cout << "[Time] 当前时间: " << t << "s\n";

//         while (!eq.empty() && eq.peek().time <= t) {
//             Event e = eq.pop();
//             std::cout << "  >> 触发事件：时间=" << e.time
//                       << "，类型=" << static_cast<int>(e.type)
//                       << "，设备=" << e.device_id
//                       << "，任务=" << e.task_id << "\n";
//         }
//     }

//     std::cout << "=== [事件系统测试结束] ===\n";

    return 0;
}


