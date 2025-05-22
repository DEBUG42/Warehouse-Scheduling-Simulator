#pragma once
#include <SFML/Graphics.hpp>
#include "SimObject.hpp"

/**
 * @brief 设备类型转换函数，将新的仓库类型转换为旧的设备类型
 * @param isInterface 是否为接口设备
 * @param isInput 是否为输入设备
 * @return 设备类型枚举
 */
inline gui::DeviceType warehouseTypeToDeviceType(bool isInterface, bool isInput)
{
    if (isInterface)
    {
        return isInput ? gui::DeviceType::INPUT_STATION : gui::DeviceType::OUTPUT_STATION;
    }
    else
    {
        return isInput ? gui::DeviceType::STORAGE_STATION : gui::DeviceType::WORK_STATION;
    }
}
