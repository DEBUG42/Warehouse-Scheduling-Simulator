
#include "../Core/Vehicle.hpp"


#include "../Core/Task.hpp"
#include "../Core/Device.hpp"
#include <iostream>
#include <cmath>
#include <limits>

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
        vehicle.max_speed = 8.0 / 3.0;
        vehicle.m_state.position = vehicle.position_m;
        vehicle.m_state.currentSpeed = 0.0;
        vehicle.m_state.motionState = Vehicle::MotionState::Stopped;
        vehicle.m_state.currentTask = nullptr;
        vehicle.m_state.operationTimer=0.0f;

        vehicles.push_back(vehicle);
    }
}
/*
// 更新所有车辆的状态
// 输入: double current_time - 当前时间, double dt - 时间步长
// 输出: 无
void VehicleManager::updateAllVehicles(double current_time, double dt,double timecale) {
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
*/

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
    const float epsilon = 0.55f;  // 防止浮点误差
    const float safe_margin = 0.2f;  // 追尾安全间距

    // 📍 设备位置映射（注意：索引 = device_id）
    float device_position[19] = {
        -1000.0f, 85.9209372261538f, 83.5209372261538f, 79.9209372261538f,
        77.5209372261538f, 73.9209372261538f, 71.5209372261538f, 67.9209372261538f,
        65.5209372261538f, 61.9209372261538f, 59.5209372261538f, 55.9209372261538f,
        53.5209372261538f, 32.000f, 29.000f, 26.000f, 14.0f, 11.000f, 8.000f
    };

    // 📌 处理位置（取模）
    float pos = std::fmod(vehicle->m_state.position, LOOP_LENGTH);
    float front_pos = std::fmod(leadingVehicle->m_state.position, LOOP_LENGTH);

    float dist_to_target = device_position[vehicle->towards_device] - pos;
    if (dist_to_target < 0) dist_to_target += LOOP_LENGTH;

    float dist_to_leading = front_pos - pos;
    if (dist_to_leading < 0) dist_to_leading += LOOP_LENGTH;

    // 更新车辆位置记录
    vehicle->position_m = pos;
    vehicle->velocity_mps = vehicle->m_state.currentSpeed;

    // // ✅ 调试输出（可用宏控制）
    // std::cout << "Vehicle #" << vehicle->id
    //           << " Speed: " << vehicle->m_state.currentSpeed
    //           << " ToDeviceDist: " << dist_to_target << "\n";

    // 🛑 到达目标位置处理
    if (vehicle->m_state.motionState == Vehicle::MotionState::Stopped &&
        std::fabs(dist_to_target) < epsilon) {

        vehicle->m_state.operationTimer += deltaTime;
        if (vehicle->m_state.operationTimer >= vehicle->m_loadTime) {
            vehicle->m_state.motionState = Vehicle::MotionState::Accelerating;
            vehicle->m_state.operationTimer = 0.0f;

            // 🚨 示例逻辑：执行完一个任务后默认前往 device 15
            vehicle->towards_device = 15;
        }
    }

    // 🚧 动态状态切换：先判断是否要减速或加速
    if ((vehicle->m_state.currentSpeed * vehicle->m_state.currentSpeed) / (2 * vehicle->m_acceleration)
         >= (dist_to_leading - vehicle->m_length - safe_margin)) {
        vehicle->m_state.motionState = Vehicle::MotionState::Decelerating;
    }
    else if ((vehicle->m_state.currentSpeed * vehicle->m_state.currentSpeed) / (2 * vehicle->m_acceleration)
             >= dist_to_target) {
        vehicle->m_state.motionState = Vehicle::MotionState::Decelerating;
    }
	// 🚧 动态状态切换：再判断是否要进入弯道,进入则减速
	else if ((pos >= 0.0f && pos <= 40.0f) && 
                 ((40.0f - pos) <= (((vehicle->m_state.currentSpeed) * (vehicle->m_state.currentSpeed)) - 
                                  (vehicle->m_maxCurveSpeed) * (vehicle->m_maxCurveSpeed)) / 
                                  (2 * vehicle->m_acceleration))){
            vehicle->m_state.motionState = Vehicle::MotionState::Decelerating;
        }
    else if ((pos >= 49.5209372261538f && pos <= 89.5209372261538f) &&
                 ((89.5209372261538f - pos) <= (((vehicle->m_state.currentSpeed) * (vehicle->m_state.currentSpeed)) - 
                                                 (vehicle->m_maxCurveSpeed) * (vehicle->m_maxCurveSpeed)) / 
                                                (2 * vehicle->m_acceleration))) {
            vehicle->m_state.motionState = Vehicle::MotionState::Decelerating;
        }
    else {
        // 弯道处理（下方弯道 + 上方弯道）
        bool in_curve1 = (pos > 40.0f && pos < 49.5209f);
        bool in_curve2 = (pos > 89.5209f && pos < LOOP_LENGTH);

        if ((in_curve1 || in_curve2) &&
            vehicle->m_state.currentSpeed > vehicle->m_maxCurveSpeed) {
            vehicle->m_state.motionState = Vehicle::MotionState::Decelerating;
        }
        else {
            vehicle->m_state.motionState = Vehicle::MotionState::Accelerating;
        }
    }


    // 🚦 按状态推进速度
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
            break;
        case Vehicle::MotionState::Stopped:
            break;
    }

    // 🚗 限速（弯道/直道）
    if ((pos >= 0.0f && pos <= 40.0f) || (pos >= 49.5f && pos <= 89.5f)) {
        if (vehicle->m_state.currentSpeed > vehicle->m_maxStraightSpeed)
            vehicle->m_state.currentSpeed = vehicle->m_maxStraightSpeed;
    }
    else {
        if (vehicle->m_state.currentSpeed > vehicle->m_maxCurveSpeed)
            vehicle->m_state.currentSpeed = vehicle->m_maxCurveSpeed;
    }

    // 🔁 更新位置
    vehicle->m_state.position += vehicle->m_state.currentSpeed * deltaTime;
    if (vehicle->m_state.position >= LOOP_LENGTH)
        vehicle->m_state.position -= LOOP_LENGTH;
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
    vehicle.max_speed = vehicle.m_maxStraightSpeed;
    vehicle.m_state.motionState = Vehicle::MotionState::Accelerating;
    vehicle.is_loaded = false;

    std::cout << "[Assign] Vehicle #" << vehicle.id << " → Task #" << task.id << "\n";
}

