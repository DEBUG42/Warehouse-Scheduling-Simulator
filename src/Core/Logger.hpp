#pragma once
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "Task.hpp"
#include "Vehicle.hpp"
#include "Event.hpp"

// 设备使用统计结构
struct DeviceStats {
    double total_has_goods_time = 0.0;
    double total_empty_time = 0.0;
    double last_state_change_time = 0.0;
    bool last_has_goods = false;
};

class Logger {
public:
    Logger();
    ~Logger();
    void logEvent(const Event& e) ;

    // void logTaskAssignment(const Task& task, const Vehicle& vehicle);

    // ✅ 记录任务调度日志
    void logTaskExecution(const Task& task, const Vehicle& vehicle);

    // ✅ 记录设备状态变化日志（有无货变化）
    void logDeviceChange(double time, int device_id, const std::string& material_id, bool before, bool after);

    // ✅ 在设备状态变更时更新统计
    void updateDeviceStates(int device_id, bool new_has_goods, double current_time);

    // ✅ 仿真结束时输出设备使用时间统计
    void logSummary(double simulation_end_time);

private:
    std::ofstream log_file_event;
    std::ofstream log_file_task;
    std::ofstream log_file_device;
    std::unordered_map<int, DeviceStats> device_stats;
};
