#include "Task.hpp"

namespace Core
{

    bool Task::validate(const std::map<int, DeviceBase *> &devices) const
    {
        // 检查起始设备和目标设备是否存在
        auto startIt = devices.find(startDeviceId);
        auto endIt = devices.find(endDeviceId);

        if (startIt == devices.end() || endIt == devices.end())
        {
            return false;
        }

        // 获取设备类型
        DeviceType startType = startIt->second->getCoreType();
        DeviceType endType = endIt->second->getCoreType();

        // 根据任务类型验证设备类型
        switch (type)
        {
        case TaskType::INPUT:
            // 入库任务：起始设备必须是入库接口，目标设备必须是入库作业口
            return startType == DeviceType::StorageIn &&
                   endType == DeviceType::WorkstationIn;

        case TaskType::OUTPUT:
            // 出库任务：起始设备必须是出库作业口，目标设备必须是出库接口
            return startType == DeviceType::WorkstationOut &&
                   endType == DeviceType::StorageOut;

        case TaskType::MOVE:
            // 移库任务：起始设备和目标设备都必须是入库接口
            return startType == DeviceType::StorageIn &&
                   endType == DeviceType::StorageIn;

        default:
            return false;
        }
    }

} // namespace Core