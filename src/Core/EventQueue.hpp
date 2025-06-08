#pragma once
#include "Event.hpp"
#include <queue>
#include <vector>

class EventQueue {
public:
    void addEvent(const Event& e);         // ✅ 参数加 const
    size_t size();
    bool hasEvent(double current_time);
    Event peek();
    Event pop();
    bool empty();
    void initializeInitialEvents();



    
private:
    std::priority_queue<Event, std::vector<Event>, EventCompare> queue;
};
