#include "../Core/Device.hpp"
#include "../Core/Event.hpp"
#include <iostream>

// 定义一个常量函数以获取指定设备的状态
// 输入: 设备ID (int device_id)
// 输出: 设备状态引用 (const DeviceState&)
const DeviceState& DeviceManager::getState(int device_id) const {
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
    // 获取指定ID的设备状态
    auto& state = devices[e.device_id];

    // 根据事件类型处理事件
    switch (e.type) {
        // 设备变为空时的处理
        case EventType::DEVICE_BECOMES_EMPTY:
            state.has_goods = false;
            state.is_transferring = false;
            std::cout << "[Event] Device " << e.device_id << " became EMPTY (task " << e.task_id << ")\n";
            break;

        // 设备中有货物时的处理
        case EventType::DEVICE_HAS_GOODS:
            state.has_goods = true;
            state.is_transferring = false;
            std::cout << "[Event] Device " << e.device_id << " now HAS GOODS (task " << e.task_id << ")\n";
            break;

        // 人类卸货完成或堆垛机完成加载时的处理
        case EventType::HUMAN_UNLOAD_DONE:
        case EventType::STACKER_FINISH_LOAD:
            state.is_transferring = false;
            std::cout << "[Event] Transfer done at device " << e.device_id << " (task " << e.task_id << ")\n";
            break;

        // 未知事件的处理
        default:
            std::cout << "[Event] Unknown event for device " << e.device_id << "\n";
            break;
    }
}
