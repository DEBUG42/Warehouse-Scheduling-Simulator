#include "../Core/Logger.hpp"
#include "../Core/Vehicle.hpp"
#include "../Core/Task.hpp"
#include "../Core/Event.hpp"
#include <vector>
#include <fstream>
#include <iostream>

Logger::Logger()
{
    try
    {
        log_file.open("simulation_log.txt", std::ios::out);
        if (!log_file.is_open())
        {
            throw std::runtime_error("[Logger] Failed to open log file!");
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }
}

Logger::~Logger()
{
    if (log_file.is_open())
    {
        log_file.close();
    }
}

void Logger::logEvent(Event &e)
{
    log_file << "[Event] Time: " << e.time
             << ", Type: " << static_cast<int>(e.type)
             << ", Device: " << e.device_id
             << ", Task: " << e.task_id << "\n";
}

void Logger::logTaskAssignment(Task &task, Vehicle &vehicle)
{
    log_file << "[Assign] Time: " << task.assign_time
             << ", Task #" << task.id
             << " assigned to Vehicle #" << vehicle.id << "\n";
}

void Logger::logSnapshot(double time, std::vector<Vehicle> &vehicles, std::vector<Task> &tasks)
{
    log_file << "[Snapshot] Time: " << time << "\n";

    for (auto &vehicle : vehicles)
    {
        log_file << "Vehicle #" << vehicle.id
                 << " | Pos: " << vehicle.position_m
                 << " | Vel: " << vehicle.velocity_mps
                 << " | State: " << VehicleManager::motionStateToString(vehicle.m_state.motionState)
                 << " | Loaded: " << vehicle.is_loaded
                 << "\n";
    }

    for (auto &task : tasks)
    {
        log_file << "Task #" << task.id
                 << " | Assigned: " << task.is_assigned
                 << " | Completed: " << (task.complete_time >= 0)
                 << "\n";
    }
    log_file << "------------------------------\n";
}
