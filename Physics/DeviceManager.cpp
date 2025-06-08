#include "../Core/Device.hpp"
#include "../Core/Event.hpp"
#include <iostream>


// DeviceManager.cpp
DeviceState& DeviceManager::getDeviceState(int id) {
    auto it = deviceStates.find(id);
    if (it != deviceStates.end()) {
        return it->second;
    } else {
        static DeviceState dummy;  // 防止越界，返回默认状态
        return dummy;
    }
}
DeviceType& DeviceManager::getDeviceType(int id) {
    auto it = deviceType.find(id);
    if (it != deviceType.end()) {
        return it->second;
    } else {
        static DeviceType dummy;  // 防止越界，返回默认状态
        return dummy;
    }
}


//初始化设备
void DeviceManager::initializeDevices() {
    deviceStates.clear();

    for (int id = 1; id <= 18; ++id) {
        DeviceState state;
        state.has_goods = false;
        state.is_reserved = false;
        state.reserved_by = -1;
        state.reserved_until = 0.0;
        state.is_transferring = false;

        deviceStates[id] = state;
    }

    std::cout << "[DeviceManager] Initialized all 18 devices.\n";
}


// 预约指定设备供任务使用
// 输入: 设备ID (int device_id), 任务ID (int task_id), 预约结束时间 (double until_time)
// 输出: 无
void DeviceManager::reserve(const Task& task, double current_time) {
    int id = task.start_device_id;

    DeviceState& state = deviceStates[id];
    state.is_reserved = true;
    state.reserved_by = task.id;

    // 设置一个保守的保留时间（比如 120 秒后释放，后续可用事件机制更新）
    state.reserved_until = current_time + 120.0;

    std::cout << "[Reserve] Device " << id << " reserved by Task #" << task.id << " until " << state.reserved_until << "s\n";
}


// 释放指定设备的预约
// 输入: 设备ID (int device_id), 任务ID (int task_id)
// 输出: 无
void DeviceManager::release(int device_id, int task_id) {
    // 获取指定ID的设备状态
    auto& state = deviceStates[device_id];
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
std::vector<DeviceManager::DeviceUpdateResult> DeviceManager::update(double current_time) {
    std::vector<DeviceManager::DeviceUpdateResult> results;

    for (auto& [id, state] : deviceStates) {
        if (state.is_reserved && current_time >= state.reserved_until) {
            state.is_reserved = false;
            state.reserved_by = -1;
            state.reserved_until = 0.0;

            std::cout << "[Auto-Release] Device #" << id << " released at " << current_time << "s\n";
        }

        // 入库口设备 空闲 → 可以触发叉车放货
        if (deviceType[id] == DeviceType::StorageIn && !state.has_goods && !state.is_reserved && !state.is_event_pending) {
            results.push_back({
                DeviceManager::DeviceEventTrigger::ForkliftPutToInPort,
                id,
                -1
            });
        state.is_event_pending = true; // 标记为已触发
        }

        // 出库作业口 有货 → 可以触发人工卸货
        if (deviceType[id] == DeviceType::WorkstationOut && state.has_goods && !state.is_transferring && !state.is_event_pending) {
            results.push_back({
                DeviceManager::DeviceEventTrigger::HumanUnloadAtOutPort,
                id,
                -1
            });
            state.is_event_pending = true; // 标记为已触发
        }

        // 入库接口设备有货 → 可触发堆垛机取货
        if (deviceType[id] == DeviceType::StorageIn && state.has_goods && !state.is_transferring && !state.is_event_pending) {
            results.push_back({
                DeviceManager::DeviceEventTrigger::StackerPickFromInInterface,
                id,
                -1
            });
            state.is_event_pending = true; // 标记为已触发
        }

        // 出库接口设备为空 → 可触发堆垛机放货
        if (deviceType[id] == DeviceType::StorageOut && !state.has_goods && !state.is_reserved && !state.is_event_pending) {
            results.push_back({
                DeviceManager::DeviceEventTrigger::StackerPutToOutInterface,
                id,
                -1
            });
            state.is_event_pending = true; // 标记为已触发
        }
    }

    return results;
}
