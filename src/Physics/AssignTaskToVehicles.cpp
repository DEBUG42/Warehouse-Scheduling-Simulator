#include "../Core/Vehicle.hpp"

#include "../Core/Task.hpp"
#include "../Core/Device.hpp"
#include <iostream>
#include <cmath>
#include <limits>

constexpr double LOOP_LENGTH = 99.47787445225672; // 环道总长度（可调）

// 初始化指定数量的车辆
// 输入: int count - 车辆数量
// 输出: 无
void VehicleManager::initializeVehicles(int count)
{
    vehicles.clear();
    for (int i = 0; i < count; ++i)
    {
        Vehicle vehicle;
        vehicle.id = i;
        vehicle.position_m = 32.000 - 0.002 * i - vehicle.m_length * i;
        vehicle.next_available_time = 0.0;
        vehicle.is_executing = false;
        vehicle.is_loaded = false;
        vehicle.towards_device = 0;
        vehicle.velocity_mps = 0.0;
        vehicle.max_speed = 8.0 / 3.0;

        vehicle.m_state.position = vehicle.position_m;
        vehicle.m_state.currentSpeed = 0.0;
        vehicle.m_state.motionState = Vehicle::MotionState::Stopped;
        vehicle.m_state.currentTask = nullptr;
        vehicle.m_state.operationTimer = 0.0f;

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

std::string VehicleManager::motionStateToString(Vehicle::MotionState state)
{
    switch (state)
    {
    case Vehicle::MotionState::Accelerating:
        return "Accelerating";
    case Vehicle::MotionState::Cruising:
        return "Cruising";
    case Vehicle::MotionState::Decelerating:
        return "Decelerating";
    case Vehicle::MotionState::Stopped:
        return "Stopped";
    default:
        return "Unknown";
    }
}

void VehicleManager::updateVehicle(float current_time, float deltaTime, Vehicle *vehicle, Vehicle *leadingVehicle)
{

    // 前车与后车相对距离
    float distance;
    float epsilon = 0.05f; // 防止浮点数误差
    float device_position[19] = {
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
    float VehiclePosition = std::fmod(vehicle->m_state.position, LOOP_LENGTH);
    float LeadingVehiclePosition = std::fmod(leadingVehicle->m_state.position, LOOP_LENGTH);

    // 对接任务的接口
    vehicle->position_m = VehiclePosition;
    vehicle->velocity_mps = vehicle->m_state.currentSpeed;

    // 前车与后车相对距离
    distance = LeadingVehiclePosition - VehiclePosition;
    if (distance < 0.f)
    {
        distance += 99.47787445225672;
    }
    // 防止浮点数误差
    epsilon = 0.05f;

    vehicle->m_state.operationTimer = 0.0f;

    // 到车库停车的判断和处理
    if (vehicle->m_state.motionState == Vehicle::MotionState::Stopped && fabs(VehiclePosition - device_position[vehicle->towards_device]) < epsilon)
    {
        if (vehicle->m_state.currentTask)
        {
            vehicle->m_state.operationTimer += deltaTime;
        }

        if (vehicle->m_state.operationTimer >= vehicle->m_loadTime)
        {
            vehicle->m_state.motionState = Vehicle::MotionState::Accelerating;
            vehicle->m_state.operationTimer = 0.0f;
        }
    }
    // 防碰撞减速
    else if (((vehicle->m_state.currentSpeed) * (vehicle->m_state.currentSpeed) / (2 * vehicle->m_acceleration)) <= (distance + vehicle->m_length + 0.002))
    {
        vehicle->m_state.motionState = Vehicle::MotionState::Decelerating;
    }
    // 到车库提前减速
    else if ((vehicle->m_state.currentSpeed) * (vehicle->m_state.currentSpeed) / (2 * vehicle->m_acceleration) <= ((device_position[vehicle->towards_device] - VehiclePosition)))
    {
        vehicle->m_state.motionState = Vehicle::MotionState::Decelerating;
    }
    // 弯道减速
    // 处理下面那个弯道的减速
    else if ((VehiclePosition >= 0.f) && (VehiclePosition <= 40.0f))
    {
        if ((40.0f - VehiclePosition) <= (((vehicle->m_state.currentSpeed) * (vehicle->m_state.currentSpeed)) - (vehicle->m_maxCurveSpeed) * (vehicle->m_maxCurveSpeed)) / (2 * vehicle->m_acceleration))
        {
            vehicle->m_state.motionState = Vehicle::MotionState::Decelerating;
        }
    }
    // 处理上面那个弯道的减速
    else if ((VehiclePosition >= 49.5209372261538) && (VehiclePosition <= 89.5209372261538))
    {
        if ((87.835981634f - VehiclePosition) <= (((vehicle->m_state.currentSpeed) * (vehicle->m_state.currentSpeed)) - (vehicle->m_maxCurveSpeed) * (vehicle->m_maxCurveSpeed)) / (2 * vehicle->m_acceleration))
        {
            vehicle->m_state.motionState = Vehicle::MotionState::Decelerating;
        }
    }
    // 不减速即设定为加速，更快运动
    //	else{
    //		vehicle->m_state.motionState = Vehicle::MotionState::Accelerating;
    //	}
    if (vehicle->m_state.motionState == Vehicle::MotionState::Stopped)
    {
        if (!vehicle->is_loaded)
        {
            std::cout << "[Vehicle] #" << vehicle->id << " picked at device " << vehicle->m_state.currentTask->start_device_id << "\n";
            vehicle->is_loaded = true;
            vehicle->towards_device = vehicle->m_state.currentTask->end_device_id;
            vehicle->target_position = device_position[vehicle->towards_device];
            vehicle->m_state.motionState = Vehicle::MotionState::Accelerating;
        }
        else
        {
            std::cout << "[Vehicle] #" << vehicle->id << " dropped at device " << vehicle->m_state.currentTask->end_device_id << "\n";
            vehicle->m_state.currentTask->complete_time = current_time;
            vehicle->m_state.currentTask = nullptr;
            vehicle->is_loaded = false;
            vehicle->towards_device = 0;
            vehicle->m_state.currentSpeed = 0.0;
            vehicle->m_state.motionState = Vehicle::MotionState::Stopped;
        }
    }
    // 根据状态确定下一步的操作
    switch (vehicle->m_state.motionState)
    {
    case Vehicle::MotionState::Accelerating:
        // 判断上一辆车的距离
        vehicle->m_state.currentSpeed += vehicle->m_acceleration * deltaTime;
        // 直线上且超过最大速度
        if ((VehiclePosition >= 0.f) && (VehiclePosition <= 40.0f) && (vehicle->m_state.currentSpeed > vehicle->m_maxStraightSpeed))
        {
            vehicle->m_state.currentSpeed = vehicle->m_maxStraightSpeed;
        }
        else if ((VehiclePosition >= 49.5209372261538) && (VehiclePosition <= 89.5209372261538) && (vehicle->m_state.currentSpeed > vehicle->m_maxStraightSpeed))
        {
            vehicle->m_state.currentSpeed = vehicle->m_maxStraightSpeed;
        }
        // 弯道上且超过最大速度
        else if ((VehiclePosition > 40.0f) && (VehiclePosition < 49.5209372261538) && (vehicle->m_state.currentSpeed > vehicle->m_maxCurveSpeed))
        {
            vehicle->m_state.currentSpeed = vehicle->m_maxCurveSpeed;
        }
        else if ((VehiclePosition > 89.5209372261538) && (VehiclePosition < 99.47787445225672) && (vehicle->m_state.currentSpeed > vehicle->m_maxCurveSpeed))
        {
            vehicle->m_state.currentSpeed = vehicle->m_maxCurveSpeed;
        }
        break;
    case Vehicle::MotionState::Decelerating:
        vehicle->m_state.currentSpeed -= vehicle->m_acceleration * deltaTime;
        if (vehicle->m_state.currentSpeed < 0.0f)
        {
            vehicle->m_state.currentSpeed = 0.0f;
        }
        if (vehicle->m_state.currentSpeed <= 0.0f)
        {
            vehicle->m_state.currentSpeed = 0.0f;
            vehicle->m_state.motionState = Vehicle::MotionState::Stopped;
        }
        break;
    case Vehicle::MotionState::Cruising:
        break;
    case Vehicle::MotionState::Stopped:
        break;
    default:
        // 处理未知状态
        break;
    }
    // 位置的更新
    vehicle->m_state.position += vehicle->m_state.currentSpeed * deltaTime;
}
// 计算从一个位置到另一个位置的距离
// 输入: double from - 起始位置, double to - 目标位置
// 输出: double - 计算出的距离
double VehicleManager::getDistance(double from, double to)
{
    double d = to - from;
    if (d < 0)
        d += LOOP_LENGTH;
    return d;
}

// 获取当前可用的车辆（即空闲且没有任务的车辆）
// 输入: Task& task - 任务, double current_time - 当前时间
// 输出: std::vector<Vehicle*> - 可用车辆的指针列表
std::vector<Vehicle *> VehicleManager::getAvailableVehicles(Task &task, double current_time)
{
    std::vector<Vehicle *> result;
    for (size_t i = 0; i < vehicles.size(); ++i)
    {
        if (vehicles[i].m_state.motionState == Vehicle::MotionState::Stopped &&
            vehicles[i].m_state.currentTask == nullptr)
        {
            result.push_back(&vehicles[i]);
        }
    }
    return result;
}

// 从候选车辆中选择最佳车辆来执行任务
// 输入: Task& task - 任务, std::vector<Vehicle*>& candidates - 候选车辆列表, double current_time - 当前时间
// 输出: Vehicle* - 最佳车辆的指针
Vehicle *VehicleManager::selectBestVehicle(Task &task, std::vector<Vehicle *> &candidates, double current_time)
{
    double best_time = std::numeric_limits<double>::max();
    Vehicle *best_vehicle = nullptr;
    float device_position[19] = {
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

    for (auto *vehicle : candidates)
    {
        double dist = getDistance(vehicle->position_m, pickup_pos);
        double est_time = std::sqrt(2 * dist / vehicle->m_acceleration); // ✅ 用 m_acceleration
        if (est_time < best_time)
        {
            best_time = est_time;
            best_vehicle = vehicle;
        }
    }

    return best_vehicle;
}

// 将任务分配给指定的车辆
// 输入: Vehicle& vehicle - 车辆, Task& task - 任务, double current_time - 当前时间
// 输出: 无
void VehicleManager::applyTaskToVehicle(Vehicle &vehicle, Task &task, double current_time)
{
    float device_position[19] = {
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
    vehicle.m_state.currentTask = &task;
    vehicle.towards_device = task.start_device_id;
    vehicle.target_position = device_position[task.start_device_id];
    vehicle.velocity_mps = 0.0;
    vehicle.max_speed = vehicle.m_maxStraightSpeed;
    vehicle.m_state.motionState = Vehicle::MotionState::Accelerating;
    vehicle.is_loaded = false;

    std::cout << "[Assign] Vehicle #" << vehicle.id << " → Task #" << task.id << "\n";
}

// 获取所有车辆
// 输入: 无
// 输出: std::vector<Vehicle>& - 所有车辆的引用列表
std::vector<Vehicle> &VehicleManager::getVehicles()
{
    return vehicles;
}

// 获取所有车辆（const版本）
// 输入: 无
// 输出: const std::vector<Vehicle>& - 所有车辆的常量引用列表
const std::vector<Vehicle> &VehicleManager::getVehicles() const
{
    return vehicles;
}
