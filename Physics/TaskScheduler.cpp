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

//功能：触发整个调度系统的事件驱动过程。    
void TaskScheduler::onEvent(sf::Time current_time) {
    // Step 1: 更新所有设备和车辆状态（如搬运完成、货物被取走等）

    updateAllDeviceStates(current_time);
    updateAllCarStates(current_time);

    // Step 2: 从任务池中筛选出当前“就绪”任务（满足起点/终点状态、顺序依赖等）
    ready_tasks ← filterReadyTasks(all_tasks, device_states, current_time)

    // Step 3: 遍历就绪任务，为每个任务选择合适车辆
    for task in ready_tasks:
        if not task.is_assigned:

            // 筛选当前能够执行该任务的车辆
            candidate_cars ← filterAvailableCars(task, all_cars, current_time)

            if candidate_cars is not empty:

                // 从候选车辆中选择一个最早完成任务的车辆
                best_car ← selectCarWithEarliestFinish(task, candidate_cars, current_time)

                if best_car ≠ null:

                    // Step 4: 正式分配任务给车辆
                    assignTaskToCar(task, best_car, current_time)

                    // 更新车辆可用时间、当前位置、状态（改为“执行中”）
                    updateCarStatus(best_car, task, current_time)

                    // 更新设备预约状态（起点设备标记“将被取货”，终点设备标记“将被放货”）
                    reserveDeviceForTask(task, current_time)

                    // 标记任务为“已分配”
                    task.is_assigned ← true

                    // Step 5: 写入调度日志（TaskExeLog、DeviceStateLog）
                    logTaskAssignment(task, best_car)
}

// 功能:判断任务是否可调度
bool isTaskReady(task, current_time){

    // Step 1: 顺序约束 —— 同一个起始设备上的任务必须按编号顺序执行
    if task.id ≠ next_task_id[task.start_device]:
        return false  // 起始设备当前不能跳过前面的任务

    // Step 2: 设备状态检查
    start = device_states[task.start_device]
    end   = device_states[task.end_device]

    if task.type == 出库:

        // 起点必须为“有货、非搬运中、允许小车取货”状态
        if not start.has_goods or start.is_transferring:
            return false

        // 终点（出库口）必须为空闲（无货、未被占用）
        if not end.is_empty or end.is_waiting_unload:
            return false

    else if task.type == 入库:

        // 起点（入库口）必须有货且未被其他任务占用
        if not start.has_goods or start.is_transferring:
            return false

        // 终点（入库接口）必须为空（即未被货物占据）
        if not end.is_empty or end.is_reserved:
            return false

    // Step 3: 时间条件（任务准备时间 ≤ 当前时间）
    if task.ready_time > current_time:
        return false

    return true  // 该任务满足调度条件
}

function filterReadyTasks(all_tasks, device_states, current_time, next_task_id){
    ready_list ← []

    for task in all_tasks:

        // 1. 已经被分配过的任务跳过
        if task.is_assigned:
            continue

        // 2. 是否满足就绪条件（顺序约束 + 起止设备状态 + 时间）
        if isTaskReady(task, current_time, device_states, next_task_id):

            // 3. 加入就绪任务池
            ready_list.append(task)

    return ready_list
}




// | 模块                 | 输入                  | 输出            | 作用           |
// | ------------------ | ------------------- | ------------- | ------------ |
// | `onEvent`          | 当前时间、车辆状态、任务列表、设备状态 | 分配任务、更新车/设备状态 | 触发调度过程       |
// | `isTaskReady`      | 单个任务、设备状态、当前时间      | True/False    | 判断任务是否满足执行条件 |
// | `filterReadyTasks` | 所有任务、设备状态           | 就绪任务列表        | 从任务池中筛选就绪任务  |

