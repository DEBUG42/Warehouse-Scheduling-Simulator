#include <Task.hpp>
#include <Vehicle.hpp>
#include <Event.hpp>
#include <vector>
#include <fstream>
#include <iostream>
#include <EventQueue.hpp>

class Logger {
public:
    Logger();
    ~Logger();

    void logEvent(Event& e);
    void logTaskAssignment(Task& tasks, Vehicle& vehicles);
    void logSnapshot(double current_time, std::vector<Vehicle>& vehicles,  std::vector<Task>& tasks);

private:
    std::ofstream log_file;
};
Logger::Logger() {
    try {
        log_file.open("simulation_log.txt", std::ios::out);
        if (!log_file.is_open()) {
            throw std::runtime_error("[Logger] Failed to open log file!");
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}


Logger::~Logger() {
    if (log_file.is_open()) {
        log_file.close();
    }
}
//