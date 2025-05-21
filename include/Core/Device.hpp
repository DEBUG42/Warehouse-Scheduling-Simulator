#pragma once
#include <SFML/Graphics.hpp>
#include <map>
#include <queue>
#include <memory>

// 设备类型枚举 (由 gui/DeviceState.hpp 提供，此处注释以避免多重定义)
/*
enum class DeviceType {
    StorageIn,      // 入库接口设备（1,3,5,7,9,11）
    StorageOut,     // 出库接口设备（2,4,6,8,10,12）
    WorkstationIn,  // 入库作业口（16,17,18）
    WorkstationOut  // 出库作业口（13,14,15）
};
*/

// 设备状态枚举 (由 gui/DeviceState.hpp 提供，此处注释以避免多重定义)
/*
enum class DeviceStatus {
    working,        // 工作中
    idle,           // 空闲中
};
*/

// 将核心层特定的枚举放入 Core 命名空间
namespace Core
{
    struct Task; // 前向声明，便于智能指针解耦
    enum class DeviceType
    {
        StorageIn,     // 入库接口设备（1,3,5,7,9,11）
        StorageOut,    // 出库接口设备（2,4,6,8,10,12）
        WorkstationIn, // 入库作业口（16,17,18）
        WorkstationOut // 出库作业口（13,14,15）
    };

    enum class DeviceStatus
    {
        working, // 工作中
        idle,    // 空闲中
    };

    // 设备基类
    class DeviceBase
    {
    protected:
        const int m_id;                                // 设备唯一标识
        const DeviceType m_type;                       // 设备类型 (使用 Core::DeviceType)
        DeviceStatus m_status;                         // 当前状态 (使用 Core::DeviceStatus)
        std::queue<std::shared_ptr<Task>> m_taskQueue; // 任务等待队列（智能指针解耦）
        std::shared_ptr<Task> m_currentTask;           // 当前处理的任务 (智能指针)
        sf::Clock m_processingTimer;                   // 处理计时器（用于堆垛机/人工操作）

    public:
        DeviceBase(int id, DeviceType type) // 使用 Core::DeviceType
            : m_id(id), m_type(type), m_status(DeviceStatus::idle), m_currentTask(nullptr)
        {
        }

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
        // void enqueueTask(const Task& task);

        // 其他公共接口...
        int getId() const { return m_id; }
        DeviceType getCoreType() const { return m_type; }       // 返回 Core::DeviceType
        DeviceStatus getCoreStatus() const { return m_status; } // 返回 Core::DeviceStatus
        size_t getQueuedTaskCount() const { return m_taskQueue.size(); }

        virtual float getProcessingProgress() const
        {
            if (m_status == DeviceStatus::working && m_currentTask)
            {
                return 0.5f; // 占位符：返回50%进度
            }
            return 0.0f;
        }

        virtual int getCurrentMaterialId() const
        {
            if (m_status == DeviceStatus::working && m_currentTask)
            {
                return -1; // 占位符：没有物料ID
            }
            return -1;
        }

        virtual int getCapacity() const
        {
            return 1; // 默认容量为1
        }

        virtual int getCurrentLoad() const
        {
            return (m_status == DeviceStatus::working && m_currentTask) ? 1 : 0; // 使用 Core::DeviceStatus
        }
    };

    // 入库接口设备特化
    class StorageInDevice : public DeviceBase
    {
    private:
        bool m_readyForUnload; // 是否允许卸货
    public:
        StorageInDevice(int id) : DeviceBase(id, DeviceType::StorageIn), m_readyForUnload(false) {}

        bool update(float deltaTime) override;

        /**
         * @brief 通知堆垛机完成取货
         * @param success 是否成功取货
         */
        void notifyCargoPickup(bool success);
    };

    // 出库作业口特化
    class WorkstationOutDevice : public DeviceBase
    {
    public:
        WorkstationOutDevice(int id) : DeviceBase(id, DeviceType::WorkstationOut) {}

        bool update(float deltaTime) override;

        /**
         * @brief 人工卸货完成回调
         */
        void notifyManualUnloadComplete();
    };
} // namespace Core

// 前向声明 GUI 定义的枚举，如果 Core 内部确实需要直接使用它们
// 但更好的做法是 Core::DeviceBase 构造函数等依然使用其内部概念，在转换到 GUI 时再映射
// enum class DeviceType; // 来自 gui/DeviceState.hpp
// enum class DeviceStatus; // 来自 gui/DeviceState.hpp