
#include "../Core/Vehicle.hpp"


#include "../Core/Task.hpp"
#include "../Core/Device.hpp"
#include <iostream>
#include <cmath>
#include <limits>

#define EPSILON 0.0001f
#define PRINT_INTERVAL 1.0f

constexpr double LOOP_LENGTH = 99.47787445225672;  // 环道总长度（可调）

// 获取所有车辆
// 输入: 无
// 输出: std::vector<Vehicle>& - 所有车辆的引用列表
std::vector<Vehicle>& VehicleManager::getAllVehicles()  {
    return vehicles;
}


// 初始化指定数量的车辆
// 输入: int count - 车辆数量
// 输出: 无
void VehicleManager::initializeVehicles(int count) {
    vehicles.clear();
    for (int i = 0; i < count; ++i) {
        Vehicle vehicle;
        vehicle.id = i;
		vehicle.m_length = 2.0;
        vehicle.position_m = 26.000-0.2*i-vehicle.m_length*i;
		vehicle.m_maxCurveSpeed = 2.0 / 3.0;
		vehicle.m_maxStraightSpeed = 8.0 / 3.0;
		vehicle.m_acceleration = 0.5;
        vehicle.next_available_time = 0.0;
		vehicle.m_loadTime=7.5;
        vehicle.is_executing = false;
        vehicle.is_loaded = false;
        vehicle.towards_device =0;
        vehicle.velocity_mps = 0.0;
        vehicle.m_state.position = vehicle.position_m;
        vehicle.m_state.currentSpeed = 0.0;
        vehicle.m_state.motionState = Vehicle::MotionState::Stopped;
        vehicle.m_state.currentTask = nullptr;
        vehicle.m_state.operationTimer=0.0f;

        vehicles.push_back(vehicle);
    }
}

std::string VehicleManager::motionStateToString(Vehicle::MotionState state) {
    switch(state) {
        case Vehicle::MotionState::Accelerating: return "Accelerating";
        case Vehicle::MotionState::Cruising: return "Cruising";
        case Vehicle::MotionState::Decelerating: return "Decelerating";
        case Vehicle::MotionState::Stopped: return "Stopped";
        default: return "Unknown";
    }
}

void VehicleManager::updateVehicle(float current_time, float deltaTime, Vehicle* vehicle, Vehicle* leadingVehicle) {
    const float LOOP_LENGTH = 99.47787445225672f;

    updateKinematics(vehicle, deltaTime, LOOP_LENGTH);

    if (shouldDecelerateForCollision(vehicle, leadingVehicle, LOOP_LENGTH)) {
        vehicle->m_state.motionState = Vehicle::MotionState::Decelerating;
    }
    else if (shouldDecelerateForCurve(vehicle)) {
        vehicle->m_state.motionState = Vehicle::MotionState::Decelerating;
    }
    else {
        vehicle->m_state.motionState = Vehicle::MotionState::Accelerating;
    }

    limitSpeed(vehicle);
    checkAndHandleArrival(vehicle, current_time);
    printVehicleDebugInfo(vehicle, current_time);
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
    for (size_t i = 0; i < vehicles.size(); ++i) {
        if (vehicles[i].m_state.motionState == Vehicle::MotionState::Stopped &&
            vehicles[i].m_state.currentTask == nullptr) {
            result.push_back(&vehicles[i]);
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
	float device_position[19]={
        -1,
		85.9209372261538,
		83.5209372261538,
		79.9209372261538,
		77.5209372261538,
		73.9209372261538,
		71.5209372261538,
		67.9209372261538,
		65.5209372261538,
		61.9209372261538,
		59.5209372261538,
		55.9209372261538,
		53.5209372261538,
		32.000,
		29.000,
		26.000,
		14.000,
		11.000,
		8.000,
	};
    double pickup_pos = device_position[task.start_device_id];

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
//测试后可用
void VehicleManager::applyTaskToVehicle(Vehicle& vehicle, Task& task, double current_time) {
		float device_position[19]={
		-1000.0f,
		85.9209372261538,
		83.5209372261538,
		79.9209372261538,
		77.5209372261538,
		73.9209372261538,
		71.5209372261538,
		67.9209372261538,
		65.5209372261538,
		61.9209372261538,
		59.5209372261538,
		55.9209372261538,
		53.5209372261538,
		32.000,
		29.000,
		26.000,
		14.000,
		11.000,
		8.000,
	};
    vehicle.m_state.currentTask = &task;//把task对象传入currentTask
    vehicle.towards_device = task.start_device_id;
    vehicle.target_position = device_position[task.start_device_id];
    vehicle.velocity_mps = 0.0;
    vehicle.m_state.motionState = Vehicle::MotionState::Accelerating;
    vehicle.is_loaded = false;

    std::cout << "[Assign] Vehicle #" << vehicle.id << " → Task #" << task.id << "\n";
}

void VehicleManager::updateKinematics(Vehicle* vehicle, float deltaTime, float LOOP_LENGTH) {
    switch (vehicle->m_state.motionState) {
        case Vehicle::MotionState::Accelerating:
            vehicle->m_state.currentSpeed += vehicle->m_acceleration * deltaTime;
            break;
        case Vehicle::MotionState::Decelerating:
            vehicle->m_state.currentSpeed -= vehicle->m_acceleration * deltaTime;
            if (vehicle->m_state.currentSpeed <= 0.0f) {
                vehicle->m_state.currentSpeed = 0.0f;
                vehicle->m_state.motionState = Vehicle::MotionState::Stopped;
            }
            break;
        case Vehicle::MotionState::Cruising:
        case Vehicle::MotionState::Stopped:
            break;
    }
    vehicle->m_state.position += vehicle->m_state.currentSpeed * deltaTime;
    if (vehicle->m_state.position >= LOOP_LENGTH)
        vehicle->m_state.position -= LOOP_LENGTH;
    vehicle->position_m = vehicle->m_state.position;
    vehicle->velocity_mps = vehicle->m_state.currentSpeed;
}
bool VehicleManager::shouldDecelerateForCurve(Vehicle* vehicle) {
    float pos = vehicle->position_m;
    float v = vehicle->m_state.currentSpeed;
    float vc = vehicle->m_maxCurveSpeed;
    float a = vehicle->m_acceleration;

    if ((pos >= 0.0f && pos <= 40.0f) && ((40.0f - pos) <= ((v * v - vc * vc) / (2 * a))))
        return true;
    if ((pos >= 49.5f && pos <= 89.5f) && ((89.5f - pos) <= ((v * v - vc * vc) / (2 * a))))
        return true;
    return false;
}

bool VehicleManager::shouldDecelerateForCollision(Vehicle* vehicle, Vehicle* leadingVehicle, float LOOP_LENGTH) {
    const float safe_margin = 0.2f;
    float pos = std::fmod(vehicle->m_state.position, LOOP_LENGTH);
    float front_pos = std::fmod(leadingVehicle->m_state.position, LOOP_LENGTH);

    float dist = front_pos - pos;
    if (dist < 0) dist += LOOP_LENGTH;

    float braking_dist = (vehicle->m_state.currentSpeed * vehicle->m_state.currentSpeed) / (2 * vehicle->m_acceleration);
    return braking_dist >= (dist - vehicle->m_length - safe_margin);
}
void VehicleManager::limitSpeed(Vehicle* vehicle) {
    float pos = vehicle->position_m;
    float& v = vehicle->m_state.currentSpeed;

    if ((pos >= 0.0f && pos <= 40.0f) || (pos >= 49.5f && pos <= 89.5f)) {
        if (v > vehicle->m_maxStraightSpeed)
            v = vehicle->m_maxStraightSpeed;
    } else {
        if (v > vehicle->m_maxCurveSpeed)
            v = vehicle->m_maxCurveSpeed;
    }
}
void VehicleManager::checkAndHandleArrival(Vehicle* vehicle, float current_time) {
    Task* task = vehicle->m_state.currentTask;
    if (!task) return;

    float pos = vehicle->position_m;
    float device_pos = getDevicePosition(task->start_device_id);
    float dist = fabs(pos - device_pos);
    if (dist > 0.5f) return;
    if (vehicle->m_state.currentSpeed > 1e-2f) return;

    if (vehicle->m_state.motionState == Vehicle::MotionState::Stopped) {
        if (!vehicle->is_loaded) {
            std::cout << "[Vehicle] #" << vehicle->id << " picked at device " << task->start_device_id << "\n";
            vehicle->is_loaded = true;
            vehicle->towards_device = task->end_device_id;
        } else {
            std::cout << "[Vehicle] #" << vehicle->id << " dropped at device " << task->end_device_id << "\n";
            task->complete_time = current_time;
            vehicle->m_state.currentTask = nullptr;
            vehicle->is_loaded = false;
            vehicle->towards_device = -1;
        }
    }
}
void VehicleManager::printVehicleDebugInfo(Vehicle* vehicle, float current_time) {

    if (current_time >= vehicle->last_debug_time + PRINT_INTERVAL - EPSILON) {
        std::cout << "[Debug] 车#" << vehicle->id
                  << " 速度: " << vehicle->m_state.currentSpeed
                  << " 加减速状态: " << motionStateToString(vehicle->m_state.motionState)
                  << " 位置: " << vehicle->position_m << "\n";
        vehicle->last_debug_time = current_time;
    }
}
float VehicleManager::getDevicePosition(int device_id) {
    static float device_position[19] = {
        -1, 85.92f, 83.52f, 79.92f, 77.52f, 73.92f, 71.52f,
        67.92f, 65.52f, 61.92f, 59.52f, 55.92f, 53.52f,
        32.00f, 29.00f, 26.00f, 14.00f, 11.00f, 8.00f
    };
    return device_position[device_id];
}
