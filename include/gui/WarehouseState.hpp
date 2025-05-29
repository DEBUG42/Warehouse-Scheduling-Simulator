#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

namespace gui
{
    /**
     * @brief 仓库状态信息
     *
     * 包含仓库的基本信息和统计数据
     */
    struct WarehouseState
    {
        // 基本信息
        std::string name;
        sf::Vector2f dimensions; // 仓库尺寸
        int totalStorageSlots;
        int occupiedSlots;

        // 运行状态
        bool isOperational;
        float operationTime; // 运行时间

        // 统计信息
        int totalTasksCompleted;
        int totalTasksFailed;
        float averageTaskTime;
        float efficiency; // 效率百分比

        /**
         * @brief 默认构造函数
         */
        WarehouseState()
            : name("Warehouse"), dimensions(1000.0f, 1000.0f),
              totalStorageSlots(100), occupiedSlots(0),
              isOperational(true), operationTime(0.0f),
              totalTasksCompleted(0), totalTasksFailed(0),
              averageTaskTime(0.0f), efficiency(100.0f)
        {
        }

        /**
         * @brief 计算占用率
         */
        float getOccupancyRate() const
        {
            if (totalStorageSlots == 0)
                return 0.0f;
            return static_cast<float>(occupiedSlots) / totalStorageSlots * 100.0f;
        }

        /**
         * @brief 获取状态描述
         */
        std::string getStatusDescription() const
        {
            if (!isOperational)
                return "Offline";
            if (occupiedSlots >= totalStorageSlots)
                return "Full";
            if (occupiedSlots == 0)
                return "Empty";
            return "Operational";
        }
    };
} // namespace gui
