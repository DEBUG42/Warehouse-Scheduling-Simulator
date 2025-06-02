#include "../Core/EventQueue.hpp"
#include <iostream>

void EventQueue::addEvent(const Event& e) {
    queue.push(e);
}

Event EventQueue::peek() {
    return queue.top();
}

Event EventQueue::pop() {
    Event e = queue.top();
    queue.pop();
    return e;
}

bool EventQueue::empty() {
    return queue.empty();
}

bool EventQueue::hasEvent(double current_time) {
    return !queue.empty() && queue.top().time <= current_time;
}

void EventQueue::initializeInitialEvents() {
    std::vector<int> out_interfaces = {2, 4, 6, 8, 10, 12};
    std::vector<int> in_ports       = {16, 17, 18};

    // 1. 出库接口设备 → 50 秒后由堆垛机上货
    for (int device_id : out_interfaces) {
        Event e;
        EventQueue event_queue;
        e.time = 50.0;
        e.type = EventType::STACKER_PUT_TO_OUT_INTERFACE;
        e.device_id = device_id;
        e.task_id = -1;  // 非任务触发
        event_queue.addEvent(e);

        std::cout << "[InitEvent] Scheduled STACKER_PUT_TO_OUT_INTERFACE at device " << device_id << " @ 50s\n";
    }

    // 2. 入库口 → 30 秒后由人工叉车上货
    for (int device_id : in_ports) {
        Event e;
        EventQueue event_queue;
        e.time = 30.0;
        e.type = EventType::FORKLIFT_PUT_TO_IN_PORT;
        e.device_id = device_id;
        e.task_id = -1;
        event_queue.addEvent(e);

        std::cout << "[InitEvent] Scheduled FORKLIFT_PUT_TO_IN_PORT at device " << device_id << " @ 30s\n";
    }
}
