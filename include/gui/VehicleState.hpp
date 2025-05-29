#pragma once
#include "../src/Core/Vehicle.hpp"
#include <SFML/Graphics.hpp>
#include <string>

namespace gui
{
    /**
     * @brief GUI层的车辆状态表示
     *
     * 封装Core::Vehicle的信息用于GUI显示
     * 提供GUI友好的接口和数据格式
     */
    class VehicleState
    {
    public:
        int id;
        sf::Vector2f position;
        float speed;
        float acceleration;
        Vehicle::VehicleState state;
        Vehicle::MotionState motionState;
        std::string statusText;
        bool isSelected;

        // 运动历史记录
        struct MovementEvent
        {
            float timestamp;
            float speed;
            float acceleration;
            std::string description;
        };
        std::vector<MovementEvent> movementHistory;

        /**
         * @brief 默认构造函数
         */
        VehicleState()
            : id(-1), position(0.0f, 0.0f), speed(0.0f), acceleration(0.0f),
              state(Vehicle::VehicleState::IDLE), motionState(Vehicle::MotionState::Stopped),
              statusText("Unknown"), isSelected(false)
        {
        }

        /**
         * @brief 从Core::Vehicle构造
         * @param coreVehicle Core层的车辆对象
         */
        VehicleState(const Vehicle &coreVehicle)
            : id(coreVehicle.getId()),
              position(coreVehicle.getPosition().x, coreVehicle.getPosition().y),
              speed(coreVehicle.getSpeed()),
              acceleration(coreVehicle.getAcceleration()),
              state(coreVehicle.getVehicleState()),
              motionState(coreVehicle.getMotionState()),
              isSelected(false)
        {
            updateStatusText();
        }

        /**
         * @brief 更新状态文本描述
         */
        void updateStatusText()
        {
            switch (state)
            {
            case Vehicle::VehicleState::IDLE:
                statusText = "Idle";
                break;
            case Vehicle::VehicleState::MOVING_TO_PICK:
                statusText = "Moving to Pick";
                break;
            case Vehicle::VehicleState::PICKING:
                statusText = "Picking";
                break;
            case Vehicle::VehicleState::MOVING_TO_DROP:
                statusText = "Moving to Drop";
                break;
            case Vehicle::VehicleState::DROPPING:
                statusText = "Dropping";
                break;
            default:
                statusText = "Unknown";
                break;
            }

            // 添加运动状态
            switch (motionState)
            {
            case Vehicle::MotionState::Accelerating:
                statusText += " (Accelerating)";
                break;
            case Vehicle::MotionState::Decelerating:
                statusText += " (Decelerating)";
                break;
            case Vehicle::MotionState::Cruising:
                statusText += " (Cruising)";
                break;
            case Vehicle::MotionState::Stopped:
                statusText += " (Stopped)";
                break;
            }
        }

        /**
         * @brief 添加运动事件到历史记录
         * @param timestamp 时间戳
         * @param description 事件描述
         */
        void addMovementEvent(float timestamp, const std::string &description)
        {
            MovementEvent event;
            event.timestamp = timestamp;
            event.speed = speed;
            event.acceleration = acceleration;
            event.description = description;

            movementHistory.push_back(event);

            // 限制历史记录长度
            if (movementHistory.size() > 50)
            {
                movementHistory.erase(movementHistory.begin());
            }
        }

        /**
         * @brief 获取最近的运动事件
         * @param count 获取的事件数量
         * @return 最近的运动事件列表
         */
        std::vector<MovementEvent> getRecentEvents(int count = 10) const
        {
            std::vector<MovementEvent> recent;
            int start = std::max(0, static_cast<int>(movementHistory.size()) - count);

            for (int i = start; i < static_cast<int>(movementHistory.size()); ++i)
            {
                recent.push_back(movementHistory[i]);
            }

            return recent;
        }
    };
} // namespace gui
