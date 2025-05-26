#include "Physics/CollisionDetector.hpp"
#include "Physics/TaskManager.cpp"
#include "Physics/DeviceManager.cpp"
#include "Physics/AssignTasksToVehicles.cpp"
#include "Core/Scheduler.hpp"


/**
 * @brief 仿真主控循环，以固定频率推进（默认100Hz）
 * @param frequency 仿真频率（单位：Hz）
 */
void Scheduler::run(double frequency) {
    dt = 1.0 / frequency;
    current_time = 0.0;

    std::cout << "[INFO] Starting simulation at " << frequency << "Hz..." << std::endl;

    int step_count = 0;
    while (!task_manager.allTasksCompleted()) {

        // Step 1: 处理当前帧的所有事件（由 EventQueue 控制）
        processEvents();

        // Step 2: 推进车辆 + 设备状态
        updateSystemStates();

        // Step 3: 尝试调度任务
        tryDispatchTasks();

        // Step 4: 每秒打印一次仿真时间
        if (step_count % int(frequency) == 0) {
            std::cout << "[SimTime] " << std::fixed << std::setprecision(2) << current_time << "s" << std::endl;
        }

        current_time += dt;
        step_count++;
    }

    std::cout << "[INFO] Simulation finished at time " << current_time << "s" << std::endl;
}


int main() {
    Scheduler scheduler;
    scheduler.run();
    return 0;
}
