#include "../Core/Logger.hpp"
void Logger::logEvent(Event& e) {
    log_file << "[Event] Time: " << e.time
             << ", Type: " << static_cast<int>(e.type)
             << ", Device: " << e.device_id
             << ", Task: " << e.task_id << "\n";
}

void Logger::logTaskAssignment(Task& tasks, Vehicle& vehicles) {
    log_file << "[Assign] Time: " << tasks.assign_time
             << ", Task #" << tasks.id
             << " assigned to Vehicle #" << vehicles.id << "\n";
}

void Logger::logSnapshot(double time, std::vector<Vehicle>& vehicles, std::vector<Task>& tasks) {
    log_file << "[Snapshot] Time: " << time << "\n";

    for (auto& vehicle : vehicles) {
        log_file << "Vehicle #" << vehicle.id
                 << " | Pos: " << vehicle.position_m
                 << " | Vel: " << vehicle.velocity_mps
                 << " | State: " << VehicleManager::motionStateToString(vehicle.m_state.motionState)              
                 << " | Loaded: " << vehicle.is_loaded
                 << "\n";
    }

    for (auto& task : tasks) {
        log_file << "Task #" << task.id
                 << " | Assigned: " << task.is_assigned
                 << " | Completed: " << (task.complete_time >= 0)
                 << "\n";
    }
    log_file << "------------------------------\n";
}
