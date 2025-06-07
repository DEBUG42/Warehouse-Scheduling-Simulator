#pragma once

enum class EventType {
    // 出库相关事件
    STACKER_PUT_TO_OUT_INTERFACE,    // 出库接口堆垛（50s）
    HUMAN_UNLOAD_AT_OUT_PORT,        // 出库口人工卸货（30s）

    // 入库相关事件
    FORKLIFT_PUT_TO_IN_PORT,         // 入库口放置（30s）
    STACKER_PICK_FROM_IN_INTERFACE,  // 入库接口取垛（25s）

    // 任务调度相关
    DEVICE_BECOMES_EMPTY,            // 某设备变为空（通用触发）
    DEVICE_HAS_GOODS,                // 某设备变为有货
    TASK_TIMEOUT,                     // 任务过期未完成

    //车辆与设备交互事件
    VEHICLE_PICK_UP_GOODS,           // 车辆取货
    VEHICLE_PUT_DOWN_GOODS          // 车辆放下货
};

// 事件对象：由调度器推动处理
struct Event {
    double time;         // 事件触发时间
    EventType type;      // 事件类型
    int device_id;       // 涉及的设备 ID
    int task_id;         // 关联任务 ID（如无任务可设为 -1）
};

// 用于 priority_queue 排序（小顶堆：时间早的事件优先）
struct EventCompare {
    bool operator()(const Event& a, const Event& b) const {
        return a.time > b.time;
    }
};