#include "../Core/Device.hpp"
#include "../Core/Event.hpp"
#include <iostream>

// 定义一个常量函数以获取指定设备的状态
// 输入: 设备ID (int device_id)
// 输出: 设备状态引用 (const DeviceState&)
DeviceState& DeviceManager::getState(int device_id) {
    // 在设备列表中查找指定ID的设备
    auto it = devices.find(device_id);
    // 如果找到，则返回该设备的状态
    if (it != devices.end()) return it->second;

    // 如果未找到，返回一个空的设备状态
    static DeviceState dummy;
    return dummy;
}

// 预约指定设备供任务使用
// 输入: 设备ID (int device_id), 任务ID (int task_id), 预约结束时间 (double until_time)
// 输出: 无
void DeviceManager::reserve(int device_id, int task_id, double until_time) {
    // 获取或创建指定ID的设备状态
    auto& state = devices[device_id];
    // 设置设备为已预约状态
    state.is_reserved = true;
    // 记录预约该设备的任务ID
    state.reserved_by = task_id;
    // 记录预约的结束时间
    state.reserved_until = until_time;

    // 输出预约信息到控制台
    std::cout << "[Reserve] Device " << device_id << " reserved by task " << task_id << " until " << until_time << "s\n";
}

// 释放指定设备的预约
// 输入: 设备ID (int device_id), 任务ID (int task_id)
// 输出: 无
void DeviceManager::release(int device_id, int task_id) {
    // 获取指定ID的设备状态
    auto& state = devices[device_id];
    // 如果当前任务ID与预约该设备的任务ID匹配，则释放设备
    if (state.reserved_by == task_id) {
        state.is_reserved = false;
        state.reserved_by = -1;
        state.reserved_until = 0.0;

        // 输出释放信息到控制台
        std::cout << "[Release] Device " << device_id << " released by task " << task_id << "\n";
    }
}

// 更新所有设备的状态
// 输入: 当前时间 (double current_time)
// 输出: 无
void DeviceManager::update(double current_time) {
    // 遍历所有设备
    for (auto& [id, state] : devices) {
        // 如果设备已预约且当前时间超过预约结束时间，则自动释放设备
        if (state.is_reserved && current_time >= state.reserved_until) {
            state.is_reserved = false;
            state.reserved_by = -1;
            state.reserved_until = 0.0;

            // 输出自动释放信息到控制台
            std::cout << "[Auto-Release] Device " << id << " released at " << current_time << "s\n";
        }
    }
}

// 处理传入的事件
// 输入: 事件对象 (const Event& e)
// 输出: 无
void DeviceManager::handleEvent(const Event& e) {
    switch (e.type) {
        case EventType::HUMAN_UNLOAD_AT_OUT_PORT:
            // 出库口货物被人工搬空 → 标记为空
            DeviceState[e.device_id].has_goods = false;
            break;

        case EventType::STACKER_PUT_TO_OUT_INTERFACE:
            // 堆垛机已把货物放到接口设备上
            DeviceState[e.device_id].has_goods = true;
            break;

        case EventType::FORKLIFT_PUT_TO_IN_PORT:
            // 入库口叉车放货完成
            DeviceState[e.device_id].has_goods = true;
            break;

        case EventType::STACKER_PICK_FROM_IN_INTERFACE:
            // 堆垛机取走入库接口设备货物
            DeviceState[e.device_id].has_goods = false;
            break;

        default:
            break;
    }
}
