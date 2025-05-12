#include <SFML/Graphics.hpp>
#include <map>
#include "Device.hpp"

enum class TaskType {
    output,
    input
};
struct Task {
    int taskId;                     // 任务唯一编号
    TaskType type;                   // 入库/出库任务
    int materialId;                  // 物料编号
    int startDeviceId;               // 起始设备ID
    int endDeviceId;                 // 目标设备ID
    sf::Time createTime;             // 任务创建时间
    sf::Time startTime;              // 实际开始时间
    sf::Time completeTime;           // 完成时间
    int assignedVehicleId = -1;      // 分配的车辆ID
    
    /**
     * @brief 验证任务设备兼容性
     * @param devices 设备映射表
     * @return 是否合法任务路径
     */
    bool validate(const std::map<int, DeviceBase*>& devices) const;
};