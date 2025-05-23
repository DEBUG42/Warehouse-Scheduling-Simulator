#pragma once
#include <string>
#include <SFML/Graphics.hpp>
#include "Core/Vehicle.hpp" // 确保 Core::Vehicle 被包含

namespace gui
{

    // 基础模拟对象类型枚举
    enum class SimObjectType // 重命名以避免与 SimObject::ObjectType 冲突，并移到命名空间作用域
    {
        Vehicle, // 车辆
        Device,  // 设备
        Track    // 轨道 (未来可能需要)
    };

    // 基础模拟对象类，用于对象选择系统
    class SimObject
    {
    private:
        SimObjectType m_simObjType; // 使用新的枚举名
        std::string m_id;           // ID 改为 std::string
        sf::Vector2f m_position;    // 增加位置成员，所有SimObject都应该有位置

    public:
        SimObject(SimObjectType type, const std::string &id, const sf::Vector2f &pos = sf::Vector2f(0, 0))
            : m_simObjType(type), m_id(id), m_position(pos) {}
        virtual ~SimObject() = default;

        SimObjectType getType() const { return m_simObjType; }
        const std::string &getId() const { return m_id; } // 返回 const 引用
        sf::Vector2f getPosition() const { return m_position; }
        void setPosition(const sf::Vector2f &pos) { m_position = pos; }
    };

    // GUI 特定的车辆状态枚举
    enum class VehicleStatus
    {
        IDLE,
        MOVING_TO_LOAD,
        LOADING,
        MOVING_TO_UNLOAD,
        UNLOADING,
        CHARGING,
        ERROR,
        UNKNOWN
    };

    // 将 Core::MotionState 转换为 gui::VehicleStatus
    inline VehicleStatus coreMotionToGuiStatus(Core::Vehicle::MotionState coreState)
    {
        switch (coreState)
        {
        case Core::Vehicle::MotionState::IDLE:
            return VehicleStatus::IDLE;
        case Core::Vehicle::MotionState::OPERATING: // 核心的 OPERATING 可能需要进一步区分是 LOADING还是UNLOADING
                                                    // 这里暂时映射为 IDLE，具体逻辑可以在 VehicleState 构造函数中细化
            return VehicleStatus::IDLE;             // 或者可以映射到一个通用的 BUSY 状态
        case Core::Vehicle::MotionState::MOVING:
        case Core::Vehicle::MotionState::ACCELERATING:
        case Core::Vehicle::MotionState::CRUISING:
        case Core::Vehicle::MotionState::DECELERATING:
            // 对于移动状态，GUI层面可能需要更详细的信息来区是去装货还是卸货
            // 这通常依赖于当前任务类型，Core::Vehicle 本身可能不直接提供此信息
            // 暂时全部映射为 UNKNOWN，需要在 VehicleState 构造函数中根据任务判断
            return VehicleStatus::UNKNOWN; // 或者一个通用的 MOVING
        default:
            return VehicleStatus::UNKNOWN;
        }
    }

    // 货物信息结构体 (GUI中使用，可以与Core::CargoInfo不同或进行转换)
    struct CargoDisplayInfo
    {
        int materialId = -1;
        int quantity = 0;
        // 根据GUI显示需求，可以添加更多如物料名称、图标等

        CargoDisplayInfo() = default;
        CargoDisplayInfo(const Core::CargoInfo &coreCargo) : materialId(coreCargo.materialId),
                                                             quantity(coreCargo.quantity) {}
    };

    // 车辆状态信息结构 (GUI用)
    struct VehicleState : public SimObject
    {
        // std::string id; // 从 SimObject 继承
        // SimObjectType type = SimObjectType::Vehicle; // SimObject 构造时设置
        sf::Vector2f position;     // 兼容测试代码直接访问
        float rawTrackPositionMm;  // 新增: 车辆在轨道上的原始位置 (mm)
        float speed;               // 当前速度（米/秒）
        gui::VehicleStatus status; // GUI特定的车辆状态
        std::string currentTaskId; // 当前任务ID（空字符串表示无任务）
        bool isLoaded;             // 载货状态
        CargoDisplayInfo cargo;    // 载货详情 (GUI用)
        float batteryLevel;        // 电量 (0.0 - 1.0)

        // 默认构造函数
        VehicleState()
            : SimObject(SimObjectType::Vehicle, ""),
              position(sf::Vector2f(0, 0)), // 先初始化position，因为它在类定义中首先声明
              rawTrackPositionMm(0.0f),
              speed(0.0f),
              status(gui::VehicleStatus::UNKNOWN),
              currentTaskId(""),
              isLoaded(false),
              cargo(),
              batteryLevel(1.0f)
        {
        }

        // 测试/手动创建用构造函数
        VehicleState(const std::string &_id, const sf::Vector2f &_pos, // _pos is render position
                     float _rawTrackPosMm, float _speed, gui::VehicleStatus _status,
                     const std::string &_taskId, bool _isLoaded,
                     const gui::CargoDisplayInfo &_cargoDetails, float _batteryLevel)
            : SimObject(gui::SimObjectType::Vehicle, _id, _pos),
              position(_pos), // 先初始化position
              rawTrackPositionMm(_rawTrackPosMm),
              speed(_speed),
              status(_status),
              currentTaskId(_taskId),
              isLoaded(_isLoaded),
              cargo(_cargoDetails),
              batteryLevel(_batteryLevel)
        {
        }

        // 从 Core::Vehicle 构建的构造函数
        VehicleState(const Core::Vehicle &coreVehicle, const sf::Vector2f &worldPos)
            : SimObject(SimObjectType::Vehicle, coreVehicle.getId(), worldPos),
              position(worldPos),                                   // 先初始化position
              rawTrackPositionMm(coreVehicle.getTrackPositionMm()), // 假设 Core::Vehicle 有 getTrackPositionMm()
              speed(coreVehicle.getSpeed()),
              status(coreMotionToGuiStatus(coreVehicle.getMotionState())),
              currentTaskId(coreVehicle.getCurrentTaskId()),
              isLoaded(coreVehicle.getIsLoaded()),
              cargo(coreVehicle.getCargoInfo()), // 直接从 Core::CargoInfo 转换/构造
              batteryLevel(coreVehicle.getBatteryLevel())
        {
            // 此处可以根据 coreVehicle.getCurrentTask() 的类型来细化 status
            // 例如，如果任务是去取货，且状态是移动，则 status = MOVING_TO_LOAD
            // if (status == gui::VehicleStatus::UNKNOWN && coreVehicle.getMotionState() != Core::Vehicle::MotionState::IDLE && coreVehicle.getMotionState() != Core::Vehicle::MotionState::OPERATING) {
            //     // 需要访问 Task 的目标或类型来判断
            //     // const Core::Task* task = coreVehicle.getCurrentTask(); // 假设有这个getter
            //     // if (task && task->type == Core::TaskType::LOAD) status = gui::VehicleStatus::MOVING_TO_LOAD;
            //     // else if (task && task->type == Core::TaskType::UNLOAD) status = gui::VehicleStatus::MOVING_TO_UNLOAD;
            // }
        }
    };

} // namespace gui
