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

