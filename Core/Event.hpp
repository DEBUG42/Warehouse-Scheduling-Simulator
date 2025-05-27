#pragma once

enum class EventType {
    DEVICE_BECOMES_EMPTY,
    DEVICE_HAS_GOODS,
    HUMAN_UNLOAD_DONE,
    STACKER_FINISH_LOAD,
    TASK_TIMEOUT,
};

struct Event {
    double time;
    EventType type;
    int device_id;
    int task_id;
};

struct EventCompare {
    bool operator()(Event& a, Event& b) {
        return a.time > b.time;
    }
};

