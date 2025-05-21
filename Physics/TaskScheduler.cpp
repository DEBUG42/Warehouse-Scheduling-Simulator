#include "TaskScheduler.hpp"
#include <fstream>
#include <sstream>
#include <iostream>


// 辅助函数：将字符串按指定分隔符分割
std::vector<std::string> split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}


// 功能: 从指定的文件路径加载任务。
// 输入:
// filename: 任务文件的路径，字符串类型。
// 输出:
// 返回 bool 值，表示任务是否成功加载。
// true: 任务文件成功打开并解析，所有任务已加载到 task_queue_map 中。
// false: 文件无法打开或解析过程中出现错误。
bool TaskScheduler::loadTasks(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return false;
    }

    std::string line;
    // 假设第一行总是标题行，读取并跳过它
    if (std::getline(file, line)) {
        // 可以选择在这里打印标题行，确认是否正确读取
        // std::cout << "Header line: " << line << std::endl;
        // 如果第一行不是你期望的标题行，你可能需要更复杂的逻辑来判断是否跳过
    } else {
        std::cerr << "File is empty or failed to read header: " << filename << std::endl;
        return false;
    }


    while (std::getline(file, line)) { // 按行读取文件内容
        if (line.empty()) { // 跳过空行
            continue;
        }

        // 使用逗号 ',' 作为分隔符来分割当前行
        std::vector<std::string> fields = split(line, ',');

        // 检查字段数量是否符合预期 (任务ID, 物料ID, 任务类型, 起始设备, 目的设备 - 共5个字段)
        if (fields.size() != 5) {
            std::cerr << "Failed to parse line (incorrect number of fields): " << line << std::endl;
            return false;
        }

        Task task;
        std::string type_str;

        try {
            // 将分割后的字符串转换为对应的类型
            task.id = std::stoi(fields[0]);
            task.material_id = fields[1];
            type_str = fields[2]; // 任务类型字符串
            task.start_device = std::stoi(fields[3]);
            task.end_device = std::stoi(fields[4]);
        } catch (const std::exception& e) {
            std::cerr << "Error converting data in line: " << line << " - " << e.what() << std::endl;
            return false;
        }


        // 根据任务类型字符串设置任务类型枚举
        if (type_str == "INBOUND") {
            task.type = INBOUND;
        } else if (type_str == "OUTBOUND") {
            task.type = OUTBOUND;
        } else {
            std::cerr << "Invalid task type: " << type_str << " in line: " << line << std::endl;
            return false;
        }

        // 将任务添加到对应设备的任务队列中
        task_queue_map[task.start_device].push(task);
    }

    file.close(); // 关闭文件
    return true;  // 成功加载任务，返回true
}

// 检查指定设备ID是否有待处理的任务
bool TaskScheduler::hasTasks(int device_id) const {
    // 检查map中是否存在该设备ID，并且其任务队列不为空
    auto it = task_queue_map.find(device_id);
    return it != task_queue_map.end() && !it->second.empty();
}

// 功能: 获取指定设备的下一个任务。
// 输入:
// device_id: 设备的唯一标识符，整数类型。
// 输出:
// 返回 Task 结构体，表示该设备的下一个任务。
// 如果指定设备没有任务，抛出 std::runtime_error 异常，异常信息为 "No tasks available for this device."
Task TaskScheduler::getNextTask(int device_id) {
    // 获取指定设备的任务队列
    auto it = task_queue_map.find(device_id);
    if (it != task_queue_map.end() && !it->second.empty()) {
        Task task = it->second.front(); // 获取队列中的第一个任务
        it->second.pop(); // 移除队列中的第一个任务
        return task; // 返回该任务
    }

    // 如果没有任务，抛出异常或返回默认构造的任务
    throw std::runtime_error("No tasks available for this device.");
}

procedure onEvent(current_time):

    updateAllDeviceStates(current_time)
    updateAllCarStates(current_time)

    ready_tasks ← filterReadyTasks(all_tasks, device_states)

    for task in ready_tasks:
        if not task.is_assigned:
            candidate_cars ← filterAvailableCars(task, all_cars, current_time)
            if candidate_cars is not empty:
                best_car ← selectCarWithEarliestFinish(task, candidate_cars, current_time)
                assignTaskToCar(task, best_car, current_time)
                logTaskAssignment(task, best_car)



function isTaskReady(task, current_time):

    if task.id != next_task_id[task.start_device]:
        return false  // 顺序不对

    if task.type == 出库:
        if not device_states[task.start_device].has_goods:
            return false
        if not device_states[task.end_device].is_empty:
            return false

    else if task.type == 入库:
        if not device_states[task.start_device].has_goods:
            return false
        if not device_states[task.end_device].is_empty:
            return false

    return true


function filterReadyTasks(all_tasks, device_states):

    ready_list ← []

    for task in all_tasks:
        if not task.is_assigned and isTaskReady(task, current_time):
            ready_list.append(task)

    return ready_list



// | 模块                 | 输入                  | 输出            | 作用           |
// | ------------------ | ------------------- | ------------- | ------------ |
// | `onEvent`          | 当前时间、车辆状态、任务列表、设备状态 | 分配任务、更新车/设备状态 | 触发调度过程       |
// | `isTaskReady`      | 单个任务、设备状态、当前时间      | True/False    | 判断任务是否满足执行条件 |
// | `filterReadyTasks` | 所有任务、设备状态           | 就绪任务列表        | 从任务池中筛选就绪任务  |

