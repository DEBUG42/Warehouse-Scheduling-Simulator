#include "Core/Scheduler.hpp"
#include <iostream>
#include <SFML/Graphics.hpp>
int main() {
    sf::Clock clock;
    std::cout << "====== Warehouse Scheduling Simulator ======\n";

   
    EventQueue event_queue;        // 事件队列
    TaskManager task_manager;      // 任务管理器
    VehicleManager vehicle_manager;      // 车辆管理器
    DeviceManager device_manager;    // 设备管理器
    Logger logger;                 // 日志记录器

        
    Scheduler scheduler;
    scheduler.bind(&task_manager, &vehicle_manager, &device_manager, &event_queue, &logger);

    auto& vehicles = vehicle_manager.getAllVehicles(); 
    vehicle_manager.initializeVehicles(3);
    scheduler.device_manager_ptr->initializeDevices();


    // 正确获取三辆车的信息
    if (vehicles.size() >= 3) {
    if (vehicles[0].m_state.motionState==Vehicle::MotionState::Stopped)
        std::cout << "第一辆车停车" << std::endl;

    } 
        //这里是在测试读取文件是否正常
    task_manager.loadFromFile("tasks.csv");
    task_manager.initializeNextTaskID();


        //这里是在验证访问容器函数是否正常
    std::vector<Task>& tasks = task_manager.getAllTasks();

    std::cout << "任务数量：" << task_manager.getAllTasks().size() << std::endl;
    std::cout << "第10个任务是否已分配？" << tasks[9].is_assigned << std::endl;
    std::cout << "第10个任务的起始设备" << tasks[9].start_device_id << std::endl;


    // 5. 生成初始事件（如堆垛机放货、人工放货）
    scheduler.event_queue_ptr->initializeInitialEvents();
    std::cout << "[Init] Initial events pushed into EventQueue.\n";

    // 6. 启动调度主循环
    std::cout << "[Sim] Simulation started...\n";
    float deltatime;
    while(1){
    deltatime=clock.restart().asSeconds();
    scheduler.run(deltatime);
    }
    std::cout << "[Sim] Simulation finished.\n";

    // 7. 输出设备使用情况

    std::cout << "[Done] Logs saved. Check TaskExeLog.txt and DeviceStateLog.txt.\n";

    return 0;
}
