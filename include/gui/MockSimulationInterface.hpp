#ifndef MOCK_SIMULATION_INTERFACE_HPP
#define MOCK_SIMULATION_INTERFACE_HPP

#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>
#include <vector>
#include <string>
#include <optional> // 用于可能不存在的查找结果
#include <map>      // 用于存储设备状态

// 前向声明，如果需要与其他GUI组件的特定类型交互
// class TrackRenderer; // 如果需要直接引用TrackRenderer的功能

namespace sim
{

    // 任务状态枚举
    enum class TaskStatus
    {
        PENDING,      // 待处理
        ASSIGNED,     // 已分配给小车
        PICKING_UP,   // 取货中
        TRANSPORTING, // 运输中
        DROPPING_OFF, // 卸货中
        COMPLETED,    // 已完成
        FAILED        // 失败
    };

    // 小车状态枚举
    enum class VehicleStatus
    {
        IDLE,              // 空闲
        MOVING_TO_PICKUP,  // 前往取货点
        PICKING_UP,        // 正在取货
        MOVING_TO_DROPOFF, // 前往卸货点
        DROPPING_OFF,      // 正在卸货
        CHARGING,          // 充电中 (如果需要)
        ERROR              // 故障
    };

    // 设备（仓库接口）状态枚举
    enum class DeviceOperationalStatus
    {
        OPERATIONAL,    // 正常运行
        OUT_OF_SERVICE, // 停止服务
        BUSY            // 忙碌
    };

    // 模拟任务结构体
    struct MockTask
    {
        int id;
        std::string description;
        TaskStatus status;
        int assignedVehicleId;       // -1 如果未分配
        float pickupLocationDistMm;  // 沿轨道的取货点距离 (mm)
        float dropoffLocationDistMm; // 沿轨道的卸货点距离 (mm)
        std::string pickupDeviceId;  // 取货设备ID (例如 "D01", "D02")
        std::string dropoffDeviceId; // 卸货设备ID

        MockTask(int _id, std::string _desc, float _pickupDist, float _dropoffDist,
                 const std::string &_pickupDevId = "", const std::string &_dropoffDevId = "")
            : id(_id), description(std::move(_desc)), status(TaskStatus::PENDING),
              assignedVehicleId(-1), pickupLocationDistMm(_pickupDist),
              dropoffLocationDistMm(_dropoffDist), pickupDeviceId(_pickupDevId),
              dropoffDeviceId(_dropoffDevId) {}
    };

    // 模拟小车结构体
    struct MockVehicle
    {
        int id;
        float currentPositionMm; // 当前在轨道上的位置 (mm from origin)
        VehicleStatus status;
        int currentTaskId;      // -1 如果没有任务
        float speedMmPerSecond; // 小车速度 (mm/s)
        float targetPositionMm; // 当前移动的目标位置

        MockVehicle(int _id, float initialPosMm = 0.0f, float speed = 2000.0f) // 默认速度2m/s
            : id(_id), currentPositionMm(initialPosMm), status(VehicleStatus::IDLE),
              currentTaskId(-1), speedMmPerSecond(speed), targetPositionMm(initialPosMm)
        {
        }
    };

    // 模拟设备状态结构体
    struct MockDeviceState
    {
        std::string id; // 例如 "D01", "D02"
        DeviceOperationalStatus operationalStatus;
        int currentTaskCount; // 当前正在处理/等待的任务数量
        // 可以添加更多如 item_count, last_accessed_time 等

        MockDeviceState(std::string _id)
            : id(std::move(_id)), operationalStatus(DeviceOperationalStatus::OPERATIONAL),
              currentTaskCount(0) {}

        // Add default constructor
        MockDeviceState()
            : id("DEFAULT_MOCK_DEVICE_ID"), operationalStatus(DeviceOperationalStatus::OPERATIONAL), // Or some other default
              currentTaskCount(0)
        {
        }
    };

    class MockSimulationInterface
    {
    public:
        MockSimulationInterface(float trackTotalLengthMm);
        ~MockSimulationInterface() = default;

        // 更新模拟状态
        void update(sf::Time deltaTime);

        // --- 任务管理 ---
        const std::vector<MockTask> &getTasks() const { return m_tasks; }
        std::optional<MockTask> getTaskById(int taskId) const;
        void addTask(const MockTask &task);
        // 尝试分配一个待处理任务给空闲的小车
        bool assignTaskToAvailableVehicle(int taskId);

        // --- 小车管理 ---
        const std::vector<MockVehicle> &getVehicles() const { return m_vehicles; }
        std::optional<MockVehicle> getVehicleById(int vehicleId) const;
        void addVehicle(const MockVehicle &vehicle);

        // --- 设备状态管理 ---
        const std::map<std::string, MockDeviceState> &getDeviceStates() const { return m_deviceStates; }
        std::optional<MockDeviceState> getDeviceStateById(const std::string &deviceId) const;
        void addDevice(const MockDeviceState &device);
        // 更新设备状态（例如，当小车开始/结束在设备处操作时调用）
        void updateDeviceStatus(const std::string &deviceId, DeviceOperationalStatus status, int taskCountChange = 0);

        // --- 初始化和配置 ---
        // void setTrackRenderer(TrackRenderer* track); // 可选，如果需要更复杂的交互
        void initializeDefaultScenario(); // 设置一些初始的小车和任务

    private:
        std::vector<MockTask> m_tasks;
        std::vector<MockVehicle> m_vehicles;
        std::map<std::string, MockDeviceState> m_deviceStates; // 使用 map 以便通过ID快速查找

        float m_trackTotalLengthMm;
        // TrackRenderer* m_trackRendererRef; // 如果需要查询轨道几何信息

        // 内部辅助函数
        void updateVehicleMovement(MockVehicle &vehicle, float deltaTimeSeconds);
        void processVehicleLogic(MockVehicle &vehicle, float deltaTimeSeconds);

        static constexpr float ACTION_DURATION_SECONDS = 2.0f; // 模拟取货/卸货的持续时间
        std::map<int, float> m_vehicleActionTimers;            // <vehicleId, remainingActionTime>
    };

} // namespace sim

#endif // MOCK_SIMULATION_INTERFACE_HPP
