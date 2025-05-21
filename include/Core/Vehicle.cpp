#include "Vehicle.hpp"
#include <cmath>

namespace Core
{

    bool Vehicle::updatePhysics(float deltaTime, const Vehicle *leadingVehicle, const Physics::TrackSegment &trackInfo)
    {
        bool stateChanged = false;
        float targetSpeed = m_maxStraightSpeed;

        // 根据轨道类型调整目标速度
        if (trackInfo.getType() == Physics::TrackSegment::Type::CURVE)
        {
            targetSpeed = std::min(m_maxCurveSpeed, targetSpeed);
        }

        // 检查前车
        if (leadingVehicle)
        {
            float safeDistance = m_length * 2.0f; // 安全距离为车长的2倍
            float distanceToLeading = leadingVehicle->getTrackPositionMm() - m_state.position;

            if (distanceToLeading < safeDistance)
            {
                targetSpeed = std::min(targetSpeed, leadingVehicle->getSpeed());
            }
        }

        // 根据当前状态更新速度
        switch (m_state.motionState)
        {
        case MotionState::IDLE:
            if (targetSpeed > 0)
            {
                m_state.motionState = MotionState::ACCELERATING;
                stateChanged = true;
            }
            break;

        case MotionState::ACCELERATING:
            m_state.currentSpeed += m_acceleration * deltaTime;
            if (m_state.currentSpeed >= targetSpeed)
            {
                m_state.currentSpeed = targetSpeed;
                m_state.motionState = MotionState::CRUISING;
                stateChanged = true;
            }
            break;

        case MotionState::CRUISING:
            if (m_state.currentSpeed > targetSpeed)
            {
                m_state.motionState = MotionState::DECELERATING;
                stateChanged = true;
            }
            break;

        case MotionState::DECELERATING:
            m_state.currentSpeed -= m_acceleration * deltaTime;
            if (m_state.currentSpeed <= targetSpeed)
            {
                m_state.currentSpeed = targetSpeed;
                m_state.motionState = MotionState::CRUISING;
                stateChanged = true;
            }
            break;

        case MotionState::OPERATING:
            // 在操作状态下不更新速度
            break;
        }

        // 更新位置
        m_state.position += m_state.currentSpeed * deltaTime;

        return stateChanged;
    }

} // namespace Core
