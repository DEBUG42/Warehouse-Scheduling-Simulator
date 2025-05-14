#pragma once
#include <SFML/Graphics.hpp>
#include <map>
#include <queue>
#include "Task.hpp"
enum class TaskType {
    output,
    input
};

// 设备类型枚举
enum class DeviceType {
    StorageIn,      // 入库接口设备（1,3,5,7,9,11）
    StorageOut,     // 出库接口设备（2,4,6,8,10,12）
    WorkstationIn,  // 入库作业口（16,17,18）
    WorkstationOut  // 出库作业口（13,14,15）
};

enum class DeviceStatus {
    working,        // 工作中
    idle,           // 空闲中
    preparing,      // 准备中
};

// 设备基类
class DeviceBase {
protected:
    const int m_id;                   // 设备唯一标识
                // 当前状态
//    std::queue<Task> m_taskQueue;     // 任务等待队列
    sf::Clock m_processingTimer;      // 处理计时器（用于堆垛机/人工操作）
    
public:
    DeviceBase(int id, DeviceType type);
    DeviceStatus m_status;
    /**
     * @brief 更新设备状态
     * @param deltaTime 仿真时间增量（秒）
     * @return 是否有状态变更（如完成货物处理）
     */
    virtual bool update(float deltaTime) = 0;
    
    /**
     * @brief 添加新任务到队列
     * @param task 任务对象
     */
//     void enqueueTask(const Task& task);
    
    // 其他公共接口...
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
// 入库接口设备特化
class StorageInDevice : public DeviceBase {
private:
    bool m_readyForUnload;           // 是否允许卸货
public:
    StorageInDevice(int id);
    
    bool update(float deltaTime) override;
    
    /**
     * @brief 通知堆垛机完成取货
     * @param success 是否成功取货
     */
    void notifyCargoPickup(bool success);
};

// 出库作业口特化
class WorkstationOutDevice : public DeviceBase {
public:
    WorkstationOutDevice(int id);
    
    bool update(float deltaTime) override;
    
    /**
     * @brief 人工卸货完成回调
     */
    void notifyManualUnloadComplete();
};

