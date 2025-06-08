#pragma once
#include <string>
#include <map>
#include <queue>
#include <vector>
#include <unordered_map>
#include <stdexcept>
// 前向声明
class DeviceManager;

#include "Device.hpp"
// 定义任务类型枚举，包括入库和出库两种类型
enum TaskType { INBOUND, OUTBOUND };

struct Task {
    int id;                          // 任务唯一编号
    std::string material_id;                // 物料编号
    TaskType type;                   // 入库/出库任务
    int start_device_id;             // 起始设备ID
    int end_device_id;               // 目标设备ID
    
    
    //调度状态
    int assigned_vehicle_id = -1;      // 分配的车辆ID
    bool is_assigned=false;                // 是否已分配

    //时间戳
    double ready_time = -1;
    double assign_time = -1;
    double pick_time = -1;
    double drop_time = -1;
    double complete_time = -1;


};

class TaskManager {
public:
    void loadFromFile(const std::string& filepath);     // 从文件中加载任务信息
    
    friend std::ostream& operator<<(std::ostream& os, Task& task);
    std::vector<Task>& getAllTasks();
    Task& getTaskById(int task_id) {
    for (auto& task : tasks) {
        if (task.id == task_id)
            return task;
    }
    throw std::runtime_error("No task found with the given task_id.");
}

    Task& getTask(int task_id);
    void initializeNextTaskID();
    std::vector<Task*> getReadyTasks(double current_time,  DeviceManager& device_manager);     // 获取当前时间点可调度的任务
    bool allTasksCompleted(); // 是否所有任务都已完成
    void markTaskAssigned(int task_id, int vehicle_id, double assign_time); // 标记任务已分配

    std::string taskTypeToString(TaskType type); // 任务类型转换为字符串
    std::vector<std::string> split(const std::string& s, char delimiter);
    std::unordered_map<int, int> next_task_id; // 起始设备 → 当前待调度任务编号

private:
    std::vector<Task> tasks;
    float last_debug_time = 0;
    friend void testTasks(TaskManager& taskmanager);//用于调试的友元函数
};

