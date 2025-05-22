//Scheduler 类（主循环控制器）
class Scheduler {
public:
    void run(double frequency = 100.0); // 主入口

private:
    double current_time = 0.0;
    double dt = 1.0 / 100.0;

    EventQueue event_queue;
    TaskManager task_manager;
    VehicleManager vehicle_manager;
    DeviceManager device_manager;
    Logger logger;

    // 主循环阶段
    void processEvents();               // 处理事件
    void updateSystemStates();         // 推进车辆与设备状态
    void tryDispatchTasks();           // 任务调度
};
