#pragma once
#include <SFML/Graphics.hpp>
#include <string>            // 需要包含 <string> 以使用 std::to_string
#include "Core/Device.hpp"   // 包含核心设备头文件
#include "gui/SimObject.hpp" // 确保 SimObject 被包含

// 将GUI相关的枚举和辅助函数放入 gui 命名空间
namespace gui
{

    // 设备类型枚举
    enum class DeviceType
    {
        INPUT_STATION,   // 入库口
        OUTPUT_STATION,  // 出库口
        STORAGE_STATION, // 仓储区
        WORK_STATION,    // 工作站
        CHARGER,         // 充电桩
        // Core 类型的映射目标
        CORE_STORAGE_IN,
        CORE_STORAGE_OUT,
        CORE_WORKSTATION_IN,
        CORE_WORKSTATION_OUT,
        UNKNOWN_DEVICE_TYPE // 未知类型
    };

    // 设备状态枚举
    enum class DeviceStatus
    {
        IDLE,
        BUSY,
        WORKING,
        OFFLINE,
        CHARGING_VEHICLE,
        ERROR_STATUS,
        UNKNOWN_DEVICE_STATUS
    };

    // 将 Core::DeviceType 转换为 gui::DeviceType
    inline gui::DeviceType coreToGuiDeviceType(Core::DeviceType coreType)
    {
        switch (coreType)
        {
        case Core::DeviceType::StorageIn:
            return gui::DeviceType::CORE_STORAGE_IN;
        case Core::DeviceType::StorageOut:
            return gui::DeviceType::CORE_STORAGE_OUT;
        case Core::DeviceType::WorkstationIn:
            return gui::DeviceType::CORE_WORKSTATION_IN;
        case Core::DeviceType::WorkstationOut:
            return gui::DeviceType::CORE_WORKSTATION_OUT;
        default:
            return gui::DeviceType::UNKNOWN_DEVICE_TYPE;
        }
    }

    // 将 Core::DeviceStatus 转换为 gui::DeviceStatus
    inline gui::DeviceStatus coreToGuiDeviceStatus(Core::DeviceStatus coreStatus)
    {
        switch (coreStatus)
        {
        case Core::DeviceStatus::idle:
            return gui::DeviceStatus::IDLE;
        case Core::DeviceStatus::working:
            return gui::DeviceStatus::WORKING;
        default:
            return gui::DeviceStatus::UNKNOWN_DEVICE_STATUS;
        }
    }

    // 设备状态信息结构
    struct DeviceState : public SimObject
    {
        gui::DeviceType deviceType; // 使用 gui::DeviceType
        gui::DeviceStatus status;   // 使用 gui::DeviceStatus
        int capacity;
        int currentLoad;
        int materialId;
        float processingProgress;
        int queuedTaskCount;
        std::string boundVehicleId;

        // 默认构造函数
        DeviceState() : SimObject(SimObjectType::Device, ""),
                        deviceType(gui::DeviceType::UNKNOWN_DEVICE_TYPE),
                        status(gui::DeviceStatus::UNKNOWN_DEVICE_STATUS),
                        capacity(1), currentLoad(0), materialId(-1),
                        processingProgress(0.0f), queuedTaskCount(0)
        {
        }

        // 用于测试和手动创建的构造函数
        DeviceState(const std::string &_id, const sf::Vector2f &_pos,
                    gui::DeviceType _deviceType, gui::DeviceStatus _status, const std::string &_boundVehicleId = "")
            : SimObject(SimObjectType::Device, _id, _pos),
              deviceType(_deviceType),
              status(_status),
              boundVehicleId(_boundVehicleId),
              capacity(1), currentLoad(0), materialId(-1),
              processingProgress(0.0f), queuedTaskCount(0) {}

        // 新增构造函数：从 Core::DeviceBase 构建
        DeviceState(const Core::DeviceBase &coreDevice, const sf::Vector2f &_pos)
            : SimObject(SimObjectType::Device, std::to_string(coreDevice.getId()), _pos),
              deviceType(coreToGuiDeviceType(coreDevice.getCoreType())),
              status(coreToGuiDeviceStatus(coreDevice.getCoreStatus())),
              capacity(coreDevice.getCapacity()),
              currentLoad(coreDevice.getCurrentLoad()),
              materialId(coreDevice.getCurrentMaterialId()),
              processingProgress(coreDevice.getProcessingProgress()),
              queuedTaskCount(static_cast<int>(coreDevice.getQueuedTaskCount())),
              boundVehicleId("") // 核心设备目前没有绑定车辆ID的直接概念，暂留空
        {
        }
    };

} // namespace gui
