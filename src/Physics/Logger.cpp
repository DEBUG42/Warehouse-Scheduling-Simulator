#include "../Core/Logger.hpp"

Logger::Logger() {
    log_file_task.open("TaskExeLog.txt", std::ios::out);
    if (!log_file_task.is_open()) {
        std::cerr << "[Logger] Failed to open TaskExeLog.txt!" << std::endl;
    } else {
        log_file_task << "TaskID\tMaterialID\tType\tStartDevice\tEndDevice\tAssignTime\tVehicleID\tPickDone\tDropDone\tFinalTime\n";
    }

    log_file_device.open("DeviceStateLog.txt", std::ios::out);
    if (!log_file_device.is_open()) {
        std::cerr << "[Logger] Failed to open DeviceStateLog.txt!" << std::endl;
    } else {
        log_file_device << "Time\tDeviceID\tMaterialID\tState\n";
    }
}

Logger::~Logger() {
    if (log_file_task.is_open()) log_file_task.close();
    if (log_file_device.is_open()) log_file_device.close();
}

//记录事件本体日志
void Logger::logEvent(const Event& e) {
    log_file_event << "[Event] Time: " << e.time
                   << ", Type: " << static_cast<int>(e.type)
                   << ", Device: " << e.device_id
                   << ", Task: " << e.task_id << "\n";
}

// ✅ 记录任务完成日志
void Logger::logTaskExecution(Task& task, Vehicle& vehicle) {
    log_file_task << task.id << "\t"
                  << task.material_id << "\t"
                  << (task.type == TaskType::INBOUND ? "INBOUND" : "OUTBOUND") << "\t"
                  << task.start_device_id << "\t"
                  << task.end_device_id << "\t"
                  << task.assign_time << "\t"
                  << vehicle.id << "\t"
                  << task.pick_time << "\t"
                  << task.drop_time << "\t"
                  << task.complete_time << "\n";
}

// ✅ 记录设备状态变化（人或堆垛机操作）
void Logger::logDeviceChange(double time, int device_id, const std::string& material_id, bool before, bool after) {
    std::string change;
    if (!before && after) change = "NO_GOODS->HAS_GOODS";
    else if (before && !after) change = "HAS_GOODS->NO_GOODS";
    else return;  // 无状态变化

    log_file_device << time << "\t"
                    << device_id << "\t"
                    << material_id << "\t"
                    << change << "\n";
}

// ✅ 累积设备的使用/空闲时间（调度后调用）
void Logger::updateDeviceStates(int device_id, bool new_has_goods, double current_time) {
    auto& stats = device_stats[device_id];

    double duration = current_time - stats.last_state_change_time;

    if (stats.last_has_goods) stats.total_has_goods_time += duration;
    else stats.total_empty_time += duration;

    stats.last_has_goods = new_has_goods;
    stats.last_state_change_time = current_time;
}

// ✅ 输出设备的最终使用情况（仿真结束）
void Logger::logSummary(double simulation_end_time) {
    std::ofstream summary("DeviceUsageSummary.txt");
    summary << "DeviceID\tHasGoodsTime\tEmptyTime\n";
    for (const auto& [id, stats] : device_stats) {
        double last_duration = simulation_end_time - stats.last_state_change_time;
        double total_has = stats.total_has_goods_time;
        double total_empty = stats.total_empty_time;

        if (stats.last_has_goods) total_has += last_duration;
        else total_empty += last_duration;

        summary << id << "\t" << total_has << "\t" << total_empty << "\n";
    }
    summary.close();
}
