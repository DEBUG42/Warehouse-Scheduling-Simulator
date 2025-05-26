#include "../Core/Vehicle.hpp"
#include "../Core/Task.hpp"
#include "../Core/Device.hpp"
#include <iostream>
#include <cmath>
#include <limits>

constexpr double LOOP_LENGTH = 100.0;  // 环道总长度（可调）

// 初始化指定数量的车辆
// 输入: int count - 车辆数量
// 输出: 无
void VehicleManager::initializeVehicles(int count) {
    vehicles.clear();
    for (int i = 0; i < count; ++i) {
        Vehicle vehicle;
        vehicle.id = i;
        vehicle.position_m = (LOOP_LENGTH / count) * i;
        vehicle.next_available_time = 0.0;
        vehicle.is_executing = false;
        vehicle.is_loaded = false;
        vehicle.towards_device = -1;
        vehicle.velocity_mps = 0.0;
        vehicle.max_speed = 8.0 / 3.0;

        vehicle.m_state.position = vehicle.position_m;
        vehicle.m_state.currentSpeed = 0.0;
        vehicle.m_state.motionState = Vehicle::MotionState::Stopped;
        vehicle.m_state.currentTask = nullptr;
        vehicle.m_state.operationTimer.restart();

        vehicles.push_back(vehicle);
    }
}

// 更新所有车辆的状态
// 输入: double current_time - 当前时间, double dt - 时间步长
// 输出: 无
void VehicleManager::updateAllVehicles(double current_time, double dt) {
    for (auto& vehicle : vehicles) {
        if (!vehicle.m_state.currentTask) continue;

        float& pos = vehicle.position_m;
        float& v = vehicle.velocity_mps;
        double a = vehicle.m_acceleration;
        double vmax = vehicle.max_speed;

        double dist_to_target = getDistance(pos, vehicle.target_position);

        switch (vehicle.m_state.motionState) {
        case Vehicle::MotionState::Accelerating:
            v += a * dt;
            if (v >= vmax) {
                v = vmax;
                vehicle.m_state.motionState = Vehicle::MotionState::Cruising;
            }
            break;

        case Vehicle::MotionState::Cruising:
            if (v * v / (2 * a) >= dist_to_target) {
                vehicle.m_state.motionState = Vehicle::MotionState::Decelerating;
            }
            break;

        case Vehicle::MotionState::Decelerating:
            v -= a * dt;
            if (v <= 0) {
                v = 0;
                vehicle.m_state.motionState = Vehicle::MotionState::Stopped;
            }
            break;

        case Vehicle::MotionState::Stopped:
        default:
            break;
        }

        pos += v * dt;
        if (pos > LOOP_LENGTH) pos -= LOOP_LENGTH;
        if (pos < 0) pos += LOOP_LENGTH;

        dist_to_target = getDistance(pos, vehicle.target_position);
        if (dist_to_target < 0.05 && v < 1e-2) {
            auto& task = *vehicle.m_state.currentTask;

            if (vehicle.m_state.motionState == Vehicle::MotionState::Stopped) {
                if (!vehicle.is_loaded) {
                    std::cout << "[Vehicle] #" << vehicle.id << " picked at device " << task.start_device_id << "\n";
                    vehicle.is_loaded = true;
                    vehicle.towards_device = task.end_device_id;
                    vehicle.target_position = task.end_device_id * 5.0;
                    vehicle.m_state.motionState = Vehicle::MotionState::Accelerating;
                } else {
                    std::cout << "[Vehicle] #" << vehicle.id << " dropped at device " << task.end_device_id << "\n";
                    task.complete_time = current_time;
                    vehicle.m_state.currentTask = nullptr;
                    vehicle.is_loaded = false;
                    vehicle.towards_device = -1;
                    vehicle.velocity_mps = 0.0;
                    vehicle.m_state.motionState = Vehicle::MotionState::Stopped;
                }
            }
        }
    }
}

// 计算从一个位置到另一个位置的距离
// 输入: double from - 起始位置, double to - 目标位置
// 输出: double - 计算出的距离
double VehicleManager::getDistance(double from, double to) {
    double d = to - from;
    if (d < 0) d += LOOP_LENGTH;
    return d;
}

// 获取当前可用的车辆（即空闲且没有任务的车辆）
// 输入: Task& task - 任务, double current_time - 当前时间
// 输出: std::vector<Vehicle*> - 可用车辆的指针列表
std::vector<Vehicle*> VehicleManager::getAvailableVehicles(Task& task, double current_time) {
    std::vector<Vehicle*> result;
    for (auto& vehicle : vehicles) {
        // ✅ 判断是否空闲
        if (vehicle.m_state.motionState == Vehicle::MotionState::Stopped &&
            vehicle.m_state.currentTask == nullptr) {
            result.push_back(&vehicle);
        }
    }
    return result;
}

// 从候选车辆中选择最佳车辆来执行任务
// 输入: Task& task - 任务, std::vector<Vehicle*>& candidates - 候选车辆列表, double current_time - 当前时间
// 输出: Vehicle* - 最佳车辆的指针
Vehicle* VehicleManager::selectBestVehicle(Task& task, std::vector<Vehicle*>& candidates, double current_time) {
    double best_time = std::numeric_limits<double>::max();
    Vehicle* best_vehicle = nullptr;

    double pickup_pos = task.start_device_id * 5.0;

    for (auto* vehicle : candidates) {
        double dist = getDistance(vehicle->position_m, pickup_pos);
        double est_time = std::sqrt(2 * dist / vehicle->m_acceleration); // ✅ 用 m_acceleration
        if (est_time < best_time) {
            best_time = est_time;
            best_vehicle = vehicle;
        }
    }

    return best_vehicle;
}

// 将任务分配给指定的车辆
// 输入: Vehicle& vehicle - 车辆, Task& task - 任务, double current_time - 当前时间
// 输出: 无
void VehicleManager::applyTaskToVehicle(Vehicle& vehicle, Task& task, double current_time) {
    vehicle.m_state.currentTask = &task;                      // ✅ 修正为 m_state.currentTask
    vehicle.towards_device = task.start_device_id;
    vehicle.target_position = task.start_device_id * 5.0;
    vehicle.velocity_mps = 0.0;
    vehicle.max_speed = vehicle.m_maxStraightSpeed;
    vehicle.m_state.motionState = Vehicle::MotionState::Accelerating;
    vehicle.is_loaded = false;

    std::cout << "[Assign] Vehicle #" << vehicle.id << " → Task #" << task.id << "\n";
}

// 获取所有车辆
// 输入: 无
// 输出: std::vector<Vehicle>& - 所有车辆的引用列表
std::vector<Vehicle>& VehicleManager::getVehicles()  {
    return vehicles;
}
