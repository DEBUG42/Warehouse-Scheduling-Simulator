#pragma once
#include <SFML/Graphics.hpp>
#include "SimObject.hpp"

/**
 * @brief 设备类型转换函数，将新的仓库类型转换为旧的设备类型
 * @param isInterface 是否为接口设备
 * @param isInput 是否为输入设备
 * @return 设备类型枚举
 */
inline DeviceType warehouseTypeToDeviceType(bool isInterface, bool isInput)
{
    if (isInterface)
    {
        return isInput ? DeviceType::INPUT_STATION : DeviceType::OUTPUT_STATION;
    }
    else
    {
        return isInput ? DeviceType::STORAGE_STATION : DeviceType::WORK_STATION;
    }
}

/**
 * @brief 设备状态转换函数，将新的仓库状态转换为旧的设备状态
 * @param warehouse 仓库状态
 * @param isInput 是否为输入设备
 * @return 设备状态
 */
inline DeviceState warehouseToDeviceState(const WarehouseState &warehouse, bool isInput)
{
    DeviceState device;
    device.id = warehouse.id;
    device.trackPosition = warehouse.trackPosition;
    device.type = warehouseTypeToDeviceType(warehouse.isInterface, isInput);
    device.status = (warehouse.currentLoad < warehouse.capacity) ? DeviceStatus::IDLE : DeviceStatus::BUSY;
    device.capacity = warehouse.capacity;
    device.currentLoad = warehouse.currentLoad;
    device.materialId = (device.currentLoad > 0) ? 1 : -1; // 简单处理，实际中应有具体物料ID
    return device;
}
