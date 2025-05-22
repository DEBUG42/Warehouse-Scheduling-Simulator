void Scheduler::run(double frequency) {
    dt = 1.0 / frequency;

    while (!task_manager.allTasksCompleted()) {

        // Step 1: 执行所有当前时刻应触发的事件
        while (!event_queue.empty() && event_queue.peek().time <= current_time) {
            auto event = event_queue.pop();
            device_manager.handleEvent(event); // 转发给设备管理器
            logger.logEvent(event);
        }

        // Step 2: 推进车辆状态（位置、速度、碰撞检测）
        car_manager.updateAllCars(current_time, dt);

        // Step 3: 更新设备状态（搬运计时等）
        device_manager.update(current_time);

        // Step 4: 调度就绪任务
        tryDispatchTasks();

        // Step 5: 可视化或记录当前帧（如每0.1s记录一次）
        logger.logSnapshot(current_time, car_manager.getCars(), task_manager.getTasks());

        current_time += dt;
    }
}


int main() {
    Scheduler scheduler;
    scheduler.run();
    return 0;
}
