#pragma once
#include <SFML/Graphics.hpp>
#include <map>
#include <string> // For Task ID
#include "Device.hpp"

namespace Core
{
    // DeviceBase已在Device.hpp中namespace Core内声明
    enum class TaskType
    {
        OUTPUT, // 出库
        INPUT,  // 入库
        MOVE    // 移库 (可选)
        // CHARGE // 充电任务 (可选)
    };

    struct Task
    {
        std::string id;    // 任务唯一编号 (改为string)
        TaskType type;     // 任务类型
        int materialId;    // 物料编号
        int startDeviceId; // 起始设备ID
        int endDeviceId;   // 目标设备ID
        // sf::Time createTime;          // 任务创建时间 (可以考虑使用 std::chrono)
        // sf::Time startTime;           // 实际开始时间
        // sf::Time completeTime;        // 完成时间
        std::string assignedVehicleId; // 分配的车辆ID (改为string, 空表示未分配)

        // 构造函数 (示例)
        Task(const std::string &_id, TaskType _type, int _materialId, int _startDev, int _endDev)
            : id(_id), type(_type), materialId(_materialId),
              startDeviceId(_startDev), endDeviceId(_endDev), assignedVehicleId("") {}

        /**
         * @brief 验证任务设备兼容性
         * @param devices 设备映射表 (假设 DeviceBase 也在 Core 命名空间)
         * @return 是否合法任务路径
         */
        bool validate(const std::map<int, Core::DeviceBase *> &devices) const;
    };

} // namespace Core