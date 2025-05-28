#pragma once

namespace gui
{

    // 简单的模拟定义，用于演示程序
    enum class VehicleStatus
    {
        Idle,
        Moving,
        Loading,
        Unloading
    };

    enum class DeviceType
    {
        Storage,
        Workstation,
        Conveyor
    };

    enum class DeviceStatus
    {
        Idle,
        Working,
        Error
    };

    enum class ObjectType
    {
        Vehicle,
        Device
    };

    // 基础对象结构，用于ObjectInspector演示
    struct SimObject
    {
        ObjectType type;
        std::string id;

        // 根据type，使用不同的状态信息
        union
        {
            VehicleStatus vehicleStatus;
            DeviceStatus deviceStatus;
        };

        SimObject(ObjectType t, const std::string &objId) : type(t), id(objId)
        {
            if (type == ObjectType::Vehicle)
            {
                vehicleStatus = VehicleStatus::Idle;
            }
            else
            {
                deviceStatus = DeviceStatus::Idle;
            }
        }
    };

} // namespace gui
