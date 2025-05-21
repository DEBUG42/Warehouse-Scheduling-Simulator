#include <vector>
#include <../Core/Vehicle.hpp>
#include <cmath>
// 三辆车的运动控制

class VehicleStateMachine {
public:
    // 当前小车
    Vehicle* vehicle;
    // 当前主小车相对原点位置
    float VehiclePosition;
    // 前车相对原点位置
    float LeadingVehiclePosition;
    // 前车与后车相对距离
    float distance;

public:
    /*
     * @brief 构造函数
     * @param v 车辆对象
     */
    VehicleStateMachine(Vehicle* v) : vehicle(v) {}

    /*
     * @brief 更新车辆状态机
     * @param deltaTime 时间步长
     * @param leadingVehicle 前车对象
     */
    void update(float deltaTime, Vehicle* leadingVehicle) {
        // 当前主小车相对原点位置
        VehiclePosition = std::fmod(vehicle->m_state.position, 95.671963268f);
        // leadingVehicle相对原点位置
        LeadingVehiclePosition = std::fmod(leadingVehicle->m_state.position, 95.671963268f);
        // 前车与后车相对距离
        distance = LeadingVehiclePosition - VehiclePosition;
        if (distance < 0.f) {
            distance += 95.671963268f; // 如果距离为负数，说明前车在后车后面一圈，需要加上一圈的长度
        }

        // 到车库的减速
        if (vehicle->m_state.motionState == Vehicle::MotionState::Stopped && VehiclePosition == 车库位置) {
            if (vehicle->m_state.operationTimer.getElapsedTime().asSeconds() == 0.0f)
                vehicle->m_state.operationTimer.restart();

            if (vehicle->m_state.operationTimer.getElapsedTime().asSeconds() >= vehicle->m_loadTime) {
                vehicle->m_state.motionState = Vehicle::MotionState::Accelerating;
                vehicle->m_state.currentSpeed = 0.0f;
                vehicle->m_state.operationTimer.restart();
            }
        }
        // 防碰撞减速
        else if (((vehicle->m_state.currentSpeed) * (vehicle->m_state.currentSpeed) / (2 * vehicle->m_acceleration)) <= (distance + vehicle->m_length + 0.002)) {
            vehicle->m_state.motionState = Vehicle::MotionState::Decelerating;
            // break; // 这里不应该使用break，因为这不是在switch-case结构中
        }
        else if ((vehicle->m_state.currentSpeed) * (vehicle->m_state.currentSpeed) / (2 * vehicle->m_acceleration) <= (目标车库的位置 - (std::fmod((vehicle->m_state.position), 95.671963268f)))) {
            vehicle->m_state.motionState = Vehicle::MotionState::Decelerating;
            // break; // 同上，不应该使用break
        }

        // 弯道减速	
        // 处理下面那个弯道的减速
        else if ((VehiclePosition >= 0.f) && (VehiclePosition <= 40.0f)) {
            if ((40.0f - VehiclePosition) <= ((vehicle->m_state.currentSpeed) * (vehicle->m_state.currentSpeed) - (vehicle->m_maxCurveSpeed) * (vehicle->m_maxCurveSpeed)) / (2 * vehicle->m_acceleration)) {
                vehicle->m_state.motionState = Vehicle::MotionState::Decelerating;
                // break; // 同上，不应该使用break
            }
        }
        // 处理上面那个弯道的减速
        else if ((VehiclePosition > 40.0f) && (VehiclePosition <= 95.671963268f)) {
            if ((VehiclePosition - 40.0f) >= ((vehicle->m_state.currentSpeed) * (vehicle->m_state.currentSpeed) - (vehicle->m_maxCurveSpeed) * (vehicle->m_maxCurveSpeed)) / (2 * vehicle->m_acceleration)) {
                vehicle->m_state.motionState = Vehicle::MotionState::Decelerating;
                // break; // 同上，不应该使用break
            }
        }
        else {
            vehicle->m_state.motionState = Vehicle::MotionState::Accelerating;
        }

        switch (vehicle->m_state.motionState) {
            case Vehicle::MotionState::Accelerating:
                // 判断上一辆车的距离
                accelerate(deltaTime);
                break;
            case Vehicle::MotionState::Decelerating:
                decelerate(deltaTime);
                if (vehicle->m_state.currentSpeed <= 0.0f) {
                    vehicle->m_state.currentSpeed = 0.0f;
                    vehicle->m_state.motionState = Vehicle::MotionState::Stopped;
                }
                break;
            case Vehicle::MotionState::Cruising:
                // 巡航状态下的处理逻辑
                break;
            case Vehicle::MotionState::Stopped:
                // 停止状态下的处理逻辑
                break;
            default:
                // 处理未知状态
                break;
        }
    }

private:
    /*
     * @brief 加速处理
     * @param deltaTime 时间步长
     */
    void accelerate(float deltaTime) {
        vehicle->m_state.currentSpeed += vehicle->m_acceleration * deltaTime;
        if (vehicle->m_state.currentSpeed > vehicle->m_maxStraightSpeed) {
            vehicle->m_state.currentSpeed = vehicle->m_maxStraightSpeed; // 限制最大速度
        }
    }

    /*
     * @brief 减速处理
     * @param deltaTime 时间步长
     */
    void decelerate(float deltaTime) {
        vehicle->m_state.currentSpeed -= vehicle->m_acceleration * deltaTime;
        if (vehicle->m_state.currentSpeed < 0.0f) {
            vehicle->m_state.currentSpeed = 0.0f; // 速度不能为负数
        }
    }

};

class MotionController3 {
public:
    Vehicle myvehicle[3];
    VehicleStateMachine vsm1, vsm2, vsm3;

    /*
     * @brief 构造函数，初始化三辆车及其状态机
     */
    MotionController3() {
        myvehicle[0] = Vehicle(26.000f, 0.0f, Vehicle::MotionState::Accelerating);
        myvehicle[1] = Vehicle(23.800f, 0.0f, Vehicle::MotionState::Accelerating);
        myvehicle[2] = Vehicle(21.600f, 0.0f, Vehicle::MotionState::Accelerating);

        vsm1 = VehicleStateMachine(&myvehicle[0]);
        vsm2 = VehicleStateMachine(&myvehicle[1]);
        vsm3 = VehicleStateMachine(&myvehicle[2]);
    }

    /*
     * @brief 更新三辆车的状态
     */
    void updateVehicles() {
        vsm1.update(1.0f / 60.0f, &myvehicle[2]); // 每辆车的更新逻辑中，传入前车作为参数
        vsm2.update(1.0f / 60.0f, &myvehicle[0]);
        vsm3.update(1.0f / 60.0f, &myvehicle[1]);
    }
};

class MotionController5 {
public:
    Vehicle myvehicle[5];
    VehicleStateMachine vsm1, vsm2, vsm3, vsm4, vsm5;

    /*
     * @brief 构造函数，初始化五辆车及其状态机
     */
    MotionController5() {
        myvehicle[0] = Vehicle(26.000f, 0.0f, Vehicle::MotionState::Accelerating);
        myvehicle[1] = Vehicle(23.800f, 0.0f, Vehicle::MotionState::Accelerating);
        myvehicle[2] = Vehicle(21.600f, 0.0f, Vehicle::MotionState::Accelerating);
        myvehicle[3] = Vehicle(19.400f, 0.0f, Vehicle::MotionState::Accelerating);
        myvehicle[4] = Vehicle(17.200f, 0.0f, Vehicle::MotionState::Accelerating);

        vsm1 = VehicleStateMachine(&myvehicle[0]);
        vsm2 = VehicleStateMachine(&myvehicle[1]);
        vsm3 = VehicleStateMachine(&myvehicle[2]);
        vsm4 = VehicleStateMachine(&myvehicle[3]);
        vsm5 = VehicleStateMachine(&myvehicle[4]);
    }

    /*
     * @brief 更新五辆车的状态
     */
    void updateVehicles() {
        vsm1.update(1.0f / 60.0f, &myvehicle[4]);
        vsm2.update(1.0f / 60.0f, &myvehicle[0]);
        vsm3.update(1.0f / 60.0f, &myvehicle[1]);
        vsm4.update(1.0f / 60.0f, &myvehicle[2]);
        vsm5.update(1.0f / 60.0f, &myvehicle[3]);
    }
};

class MotionController7 {
public:
    Vehicle myvehicle[7];
    VehicleStateMachine vsm1, vsm2, vsm3, vsm4, vsm5, vsm6, vsm7;

    /*
     * @brief 构造函数，初始化七辆车及其状态机
     */
    MotionController7() {
        myvehicle[0] = Vehicle(26.000f, 0.0f, Vehicle::MotionState::Accelerating);
        myvehicle[1] = Vehicle(23.800f, 0.0f, Vehicle::MotionState::Accelerating);
        myvehicle[2] = Vehicle(21.600f, 0.0f, Vehicle::MotionState::Accelerating);
        myvehicle[3] = Vehicle(19.400f, 0.0f, Vehicle::MotionState::Accelerating);
        myvehicle[4] = Vehicle(17.200f, 0.0f, Vehicle::MotionState::Accelerating);
        myvehicle[5] = Vehicle(15.000f, 0.0f, Vehicle::MotionState::Accelerating);
        myvehicle[6] = Vehicle(12.800f, 0.0f, Vehicle::MotionState::Accelerating);

        vsm1 = VehicleStateMachine(&myvehicle[0]);
        vsm2 = VehicleStateMachine(&myvehicle[1]);
        vsm3 = VehicleStateMachine(&myvehicle[2]);
        vsm4 = VehicleStateMachine(&myvehicle[3]);
        vsm5 = VehicleStateMachine(&myvehicle[4]);
        vsm6 = VehicleStateMachine(&myvehicle[5]);
        vsm7 = VehicleStateMachine(&myvehicle[6]);
    }

    /*
     * @brief 更新七辆车的状态
     */
    void updateVehicles() {
        vsm1.update(1.0f / 60.0f, &myvehicle[6]);
        vsm2.update(1.0f / 60.0f, &myvehicle[0]);
        vsm3.update(1.0f / 60.0f, &myvehicle[1]);
        vsm4.update(1.0f / 60.0f, &myvehicle[2]);
        vsm5.update(1.0f / 60.0f, &myvehicle[3]);
        vsm6.update(1.0f / 60.0f, &myvehicle[4]);
        vsm7.update(1.0f / 60.0f, &myvehicle[5]);
    }
};
