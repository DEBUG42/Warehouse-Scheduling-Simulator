#include "../Core/Task.hpp"
#include "../Core/Event.hpp"
#include "../Core/Device.hpp"
#include "../Core/Vehicle.hpp"
#include "../Core/Scheduler.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
//辅助函数：从被保护的task容器中获取task对象
std::vector<Task>& TaskManager::getAllTasks() {
    return tasks;
}
// Physics/TaskManager.cpp
// Physics/TaskManager.cpp
Task& TaskManager::getTask(int task_id) {
    // 假设 TaskManager 内部用一个名为 'tasks_' 的 std::vector 存储所有任务
    for (auto& task : tasks) { // 遍历 tasks_ 向量中的每个任务
        if (task.id == task_id) { // 如果找到匹配的 ID
            return task; // 返回该任务的常量引用
        }
    }
    // 如果循环结束仍未找到任务，则返回一个默认的 Task 对象
    // 这是一个安全措施，防止返回无效引用。
    // 你应该确保这个默认任务有一个可识别的无效 ID (例如 -1)
    static Task dummy_task;
    dummy_task.id = -1; // 设置一个无效的任务ID，以便调用方可以检查
    return dummy_task;
}
//辅助函数：将枚举型 TaskType 转换为字符串
std::string TaskManager::taskTypeToString(TaskType type) {
    switch (type) {
        case INBOUND:
            return "INBOUND";
        case OUTBOUND:
            return "OUTBOUND";
        default:
            return "UNKNOWN";
    }
}


// 辅助函数：将字符串按指定分隔符分割
// 输入: 字符串 (const std::string& s), 分隔符 (char delimiter)
// 输出: 分割后的字符串列表 (std::vector<std::string>)
std::vector<std::string> TaskManager::split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

/**
 * @brief 从 CSV 文件中加载任务数据，并初始化任务池
 * @param filepath CSV 文件路径（建议为 UTF-8 编码，无 BOM）
 * @details 
 *   - 文件应包含表头：任务编号,物料编号,任务类型,起始设备编号,目的设备编号
 *   - 任务类型字段为 INBOUND 或 OUTBOUND
 *   - 成功解析的任务将存入 tasks 容器，并初始化 next_task_id 映射
 * @output
 *   - tasks: 所有任务的结构体列表
 *   - next_task_id: 每个起始设备应执行的最小编号任务
 */
void TaskManager::loadFromFile(const std::string& filepath) {
    std::ifstream fin(filepath);
    std::string line;

    // 检查文件是否成功打开
    if (!fin.is_open()) {
        std::cerr << "[ERROR] Cannot open task file: " << filepath << std::endl;
        return;
    }

    std::getline(fin, line); // 跳过表头

    // 循环读取文件中的每一行，直到文件结束
    while (std::getline(fin, line)) {
        std::stringstream ss(line);
        std::string token;

        Task task;
        std::string type_str;

        // 逐列读取 CSV 数据并赋值给任务结构体
        std::getline(ss, token, ',');
        task.id = std::stoi(token);

        std::getline(ss, task.material_id, ',');

        std::getline(ss, type_str, ',');

        std::getline(ss, token, ',');
        task.start_device_id = std::stoi(token);

        std::getline(ss, token, ',');
        task.end_device_id = std::stoi(token);

        // 根据任务类型字符串设置任务类型
        task.type = (type_str == "INBOUND") ? TaskType::INBOUND : TaskType::OUTBOUND;
        tasks.push_back(task);

        // 初始化或更新 next_task_id 映射
        if (!next_task_id.count(task.start_device_id) || task.id < next_task_id[task.start_device_id]) {
            next_task_id[task.start_device_id] = task.id;
        }
    }

    // 输出加载的任务数量信息    
    std::cout << "[INFO] Loaded " << tasks.size() << " tasks from " << filepath << std::endl;
}

// 获取当前时间点下所有可以立即执行的任务。
// 一个任务被认为是“准备好执行”的，如果它满足以下所有条件：
// 1. 尚未被分配（即没有人或设备在处理它）。
// 2. 满足顺序约束（如果存在）。例如，一个任务可能需要前一个任务完成后才能开始。
// 3. 任务的就绪时间 (ready_time) 已经到达或过去。
// 4. 任务所需的起始设备和结束设备目前都未被保留（即空闲）。

// 输入:
//   current_time: 当前仿真的时间戳 (double类型)。
//   device_manager: 设备管理器的常量引用。用于查询设备当前的占用状态。
//
// 输出:
//   std::vector<Task*>: 一个指向所有当前准备好执行任务的指针列表。
//                        返回指针是为了避免复制大型 Task 对象，并允许直接操作原始任务。
std::vector<Task*> TaskManager::getReadyTasks(double current_time, DeviceManager& device_manager) {
    std::vector<Task*> ready;

    for (Task& task : tasks) {
        if (task.is_assigned) continue;

        // 顺序限制
        if (task.id != next_task_id[task.start_device_id]) continue;

        if (task.ready_time > current_time) continue;

        const auto& dev_start = device_manager.getDeviceState(task.start_device_id);
        const auto& dev_end = device_manager.getDeviceState(task.end_device_id);

        if (dev_start.is_reserved || dev_end.is_reserved || !dev_start.has_goods)
            continue;

        ready.push_back(&task);
    }
    return ready;
}

// 检查所有任务是否已经完成
// 输入: 无
// 输出: 布尔值，指示所有任务是否已完成 (bool)
// TaskManager.cpp 中实现
bool TaskManager::allTasksCompleted() {
    for (const auto& task : tasks) {
        // 任务未被分配，或者未完成，都不算完成
        if (!task.is_assigned || task.complete_time < 0.0) {
            return false;
        }
    }
    return true;
}


// 标记指定的任务已分配给车辆，并记录分配时间和车辆ID
// 输入: 任务ID (int task_id), 车辆ID (int vehicle_id), 分配时间 (double assign_time)
// 输出: 无
void TaskManager::markTaskAssigned(int task_id, int vehicle_id, double assign_time) {
    // 标记指定的任务已分配给车辆，并记录分配时间和车辆ID
    for (auto& task : tasks) {
        if (task.id == task_id) {
            task.is_assigned = true;
            task.assigned_vehicle_id = vehicle_id;
            task.assign_time = assign_time;

            // 更新起始设备的 next_task_id
            int& cur = next_task_id[task.start_device_id];
            if (task.id == cur) {
                ++cur;
            }
            return;
        }
    }
}



// // 尝试分派任务给可用的车辆
// // 输入: 无（依赖于类的成员变量）
// // 输出: 无
// void Scheduler::tryDispatchTasks() {
//     auto ready_tasks = task_manager.getReadyTasks(current_time, device_manager);

//     for (Task* task : ready_tasks) {
//         if (task->is_assigned) continue;

//         auto candidates = vehicle_manager.getAvailableVehicles(*task, current_time);
//         if (candidates.empty()) continue;

//         Vehicle* best_vehicle = vehicle_manager.selectBestVehicle(*task, candidates, current_time);
//         if (!best_vehicle) continue;

//         // 分配任务
//         vehicle_manager.applyTaskToVehicle(*best_vehicle, *task, current_time);
//         task_manager.markTaskAssigned(task->id, best_vehicle->id, current_time);
//         device_manager.reserve(task->start_device_id, task->id, current_time + 5.0);  // 假设锁定5s

//         // 如果任务为出库任务，添加“人工卸货完成”延迟事件
//         if (task->type == TaskType::OUTBOUND) {
//             Event e {
//                 .time = current_time + 30.0,
//                 .type = EventType::DEVICE_BECOMES_EMPTY,
//                 .device_id = task->end_device_id,
//                 .task_id = task->id
//             };
//             event_queue.addEvent(e);
//         }

//         // 日志记录任务分配情况
//         logger.logTaskAssignment(*task, *best_vehicle);
//     }
// }
