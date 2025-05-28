#pragma once
class Vehicle;
#include "Task.hpp"
#include "Vehicle.hpp"
#include "Event.hpp"
#include <vector>
#include <fstream>
#include <iostream>
#include "EventQueue.hpp"

class Logger
{
public:
    Logger();
    ~Logger();

    void logEvent(Event &e);
    void logTaskAssignment(Task &tasks, Vehicle &vehicles);
    void logSnapshot(double current_time, std::vector<Vehicle> &vehicles, std::vector<Task> &tasks);

private:
    std::ofstream log_file;
};

//