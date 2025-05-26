#pragma once
#include "Event.hpp"
#include <queue>
#include <vector>

class EventQueue {
public:
    // 添加一个事件到事件队列中
    // 输入: Event& e - 要添加的事件
    // 输出: 无
    void addEvent(Event& e);

    // 检查当前时间是否有事件发生
    // 输入: double current_time - 当前时间
    // 输出: bool - 如果有事件发生返回true，否则返回false
    bool hasEvent(double current_time);

    // 查看事件队列中最先发生的事件（不移除事件）
    // 输入: 无
    // 输出: Event - 最先发生的事件
    Event peek();

    // 移除并返回事件队列中最先发生的事件
    // 输入: 无
    // 输出: Event - 最先发生的事件
    Event pop();

    // 检查事件队列是否为空
    // 输入: 无
    // 输出: bool - 如果事件队列为空返回true，否则返回false
    bool empty();

private:
    // 优先队列，存储事件，按时间排序
    std::priority_queue<Event, std::vector<Event>, EventCompare> queue;
};

// 添加事件到事件队列的实现
// 输入: Event& e - 要添加的事件
// 输出: 无
void EventQueue::addEvent(Event& e) {
    queue.push(e);
}

// 查看事件队列中最先发生的事件（不移除事件）
// 输入: 无
// 输出: Event - 最先发生的事件
Event EventQueue::peek() {
    return queue.top();
}

// 移除并返回事件队列中最先发生的事件
// 输入: 无
// 输出: Event - 最先发生的事件
Event EventQueue::pop() {
    Event e = queue.top();
    queue.pop();
    return e;
}

// 检查事件队列是否为空
// 输入: 无
// 输出: bool - 如果事件队列为空返回true，否则返回false
bool EventQueue::empty() {
    return queue.empty();
}

// 检查当前时间是否有事件发生
// 输入: double current_time - 当前时间
// 输出: bool - 如果有事件发生返回true，否则返回false
bool EventQueue::hasEvent(double current_time) {
    return !queue.empty() && queue.top().time <= current_time;
}
