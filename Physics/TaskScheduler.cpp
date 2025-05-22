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

// 记录任务执行信息，便于评估系统效率。
procedure logTaskAssignment(task, car){
    log_line ← join([
        task.id,
        task.material_id,
        task.type,               // 可以是 "入库" 或 "出库"
        task.start_device,
        task.end_device,
        task.assign_time,
        car.id,
        task.pick_time,
        task.drop_time,
        task.complete_time
    ], sep = '\t')

    writeToFile("TaskExeLog.txt", log_line)
}

void Scheduler::tryDispatchTasks() {
    auto ready_tasks = task_manager.getReadyTasks(current_time, device_manager);

    for (auto* task : ready_tasks) {
        if (task->is_assigned) continue;

        auto candidates = car_manager.getAvailableCars(*task, current_time);
        if (candidates.empty()) continue;

        Car* best = car_manager.selectBestCar(*task, candidates, current_time);
        if (!best) continue;

        car_manager.applyTaskToCar(*best, *task, current_time);
        task_manager.markTaskAssigned(task->id, best->id, current_time);
        device_manager.reserve(task->start_device, task->id, current_time + 5); // 5秒为占用示意
        logger.logTaskAssignment(*task, *best);

        // 如果有延迟事件，如人工卸货，放入 event_queue
        if (task->type == TaskType::OUTBOUND) {
            event_queue.addEvent(Event{
                .time = current_time + 30.0,
                .type = EventType::DEVICE_BECOMES_EMPTY,
                .device_id = task->end_device,
                .task_id = task->id
            });
        }
    }
}
