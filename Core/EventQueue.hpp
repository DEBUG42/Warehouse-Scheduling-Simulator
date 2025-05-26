#pragma once
#include "Event.hpp"
#include <queue>
#include <vector>

class EventQueue {
public:
    void addEvent(const Event& e);
    bool hasEvent(double current_time) const;
    Event peek() const;
    Event pop();
    bool empty() const;

private:
    std::priority_queue<Event, std::vector<Event>, EventCompare> queue;
};

#include "EventQueue.hpp"

void EventQueue::addEvent(const Event& e) {
    queue.push(e);
}

Event EventQueue::peek() const {
    return queue.top();
}

Event EventQueue::pop() {
    Event e = queue.top();
    queue.pop();
    return e;
}

bool EventQueue::empty() const {
    return queue.empty();
}

bool EventQueue::hasEvent(double current_time) const {
    return !queue.empty() && queue.top().time <= current_time;
}
