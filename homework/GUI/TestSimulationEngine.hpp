#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <map>
#include <cmath>
#include "../Core/Vehicle.hpp"
#include "../Core/Device.hpp"
#include "../Core/TaskQueue.hpp"
#include "../Physics/TrackSegment.hpp"
#include "SimObject.hpp"

// 确保M_PI定义可用
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include "ConfigReader.hpp"

// 临时的仿真引擎类，用于测试GUI功能
class TestSimulationEngine {
private:
    // 模拟状态
    struct {
        float simulationTime = 0.0f;    // 仿真累计时间
        float timeScale = 1.0f;         // 时间缩放比例
        bool isPaused = false;          // 暂停状态
    } m_state;

    // 模拟对象
    std::vector<VehicleState> m_vehicles;       // 车辆状态集合
    std::map<int, DeviceState> m_devices;       // 设备状态映射（ID -> 状态）
    std::map<int, sf::Vector2f> m_devicePositions; // 设备位置映射
    std::vector<Task> m_tasks;                 // 任务队列

    // 配置
    ConfigReader m_config;
    float m_trackLength = 200.0f;
    float m_trackRadius = 100.0f;
    float m_vehicleSpeed = 0.3f;

public:
    // 初始化引擎
    void initialize() {
        // 加载配置
        m_config.loadConfig();
        
        // 创建车辆
        int vehicleCount = m_config.getVehicleCount();
        for (int i = 0; i < vehicleCount; ++i) {
            VehicleState vehicle;
            vehicle.id = i;
            vehicle.position = i * (1.0f / vehicleCount);
            vehicle.speed = 0.3f + i * 0.1f;
            vehicle.isLoaded = (i % 2 == 0);
            m_vehicles.push_back(vehicle);
        }
        
        // 创建设备
        createDevices();
        
        // 创建任务
        createSampleTasks();
    }
    
    // 更新仿真状态
    void update(float deltaTime) {
        if (m_state.isPaused) return;
        
        // 更新仿真时间
        float scaledDelta = deltaTime * m_state.timeScale;
        m_state.simulationTime += scaledDelta;
        
        // 更新车辆位置
        for (auto& vehicle : m_vehicles) {
            vehicle.position += vehicle.speed * scaledDelta * 0.02f;
            if (vehicle.position >= 1.0f) {
                vehicle.position -= 1.0f;
            }
        }
    }
    
    // 暂停/继续仿真
    void togglePause() {
        m_state.isPaused = !m_state.isPaused;
    }
    
    // 设置仿真速率
    void setTimeScale(float scale) {
        m_state.timeScale = scale;
    }
    
    // 获取仿真时间
    float getSimulationTime() const {
        return m_state.simulationTime;
    }
    
    // 获取是否暂停
    bool isPaused() const {
        return m_state.isPaused;
    }
    
    // 获取车辆状态
    const std::vector<VehicleState>& getVehicles() const {
        return m_vehicles;
    }
    
    // 获取设备状态
    const std::map<int, DeviceState>& getDevices() const {
        return m_devices;
    }
      // 获取设备位置
    sf::Vector2f getDevicePosition(int deviceId) const {
        auto it = m_devicePositions.find(deviceId);
        if (it != m_devicePositions.end()) {
            return it->second;
        }
        return sf::Vector2f(0, 0);
    }
    
    // 获取车辆位置
    sf::Vector2f getVehiclePosition(int vehicleIndex) const {
        if (vehicleIndex >= 0 && vehicleIndex < static_cast<int>(m_vehicles.size())) {
            const VehicleState& vehicle = m_vehicles[vehicleIndex];
            // 计算车辆位置
            float angle = vehicle.position * 2.0f * M_PI;
            float radius = getTrackRadius();
            float x = radius * std::cos(angle);
            float y = radius * std::sin(angle) * 0.7f;
            
            return sf::Vector2f(x, y);
        }
        return sf::Vector2f(0, 0);
    }
    
    // 获取任务队列
    const std::vector<Task>& getTasks() const {
        return m_tasks;
    }
    
    // 添加任务
    void addTask(const Task& task) {
        m_tasks.push_back(task);
    }
    
    // 获取车辆数量
    size_t getVehicleCount() const {
        return m_vehicles.size();
    }
    
    // 获取待处理任务数量
    size_t getPendingTaskCount() const {
        size_t count = 0;
        for (const auto& task : m_tasks) {
            if (task.assignedVehicleId < 0) {
                count++;
            }
        }
        return count;
    }
    
    // 获取已完成任务数量
    size_t getCompletedTaskCount() const {
        size_t count = 0;
        for (const auto& task : m_tasks) {
            if (task.completeTime != sf::Time::Zero) {
                count++;
            }
        }
        return count;
    }
      // 获取轨道半径
    float getTrackRadius() const {
        return m_trackRadius;
    }
    
    // 获取轨道长度
    float getTrackLength() const {
        return m_trackLength;
    }
    
    // 获取曲线半径
    float getCurveRadius() const {
        return m_trackRadius / 3.0f; // 弯道半径通常是轨道半径的一部分
    }
    
    // 更新车辆状态
    bool updateVehicle(int id, const VehicleState& newState) {
        for (auto& vehicle : m_vehicles) {
            if (vehicle.id == id) {
                vehicle = newState;
                return true;
            }
        }
        return false;
    }
    
    // 为测试选择车辆
    std::shared_ptr<SimObject> selectVehicle(int id) {
        // 创建一个虚拟的SimObject，仅用于测试选择功能
        return std::make_shared<SimObject>(SimObject::ObjectType::Vehicle, id);
    }
    
    // 为测试选择设备
    std::shared_ptr<SimObject> selectDevice(int id) {
        // 创建一个虚拟的SimObject，仅用于测试选择功能
        return std::make_shared<SimObject>(SimObject::ObjectType::Device, id);
    }

private:
    // 创建模拟设备
    void createDevices() {
        // 计算设备位置的基准参数
        float radius = m_trackRadius;
        float outerRadius = radius + 50.0f;
        
        // 入库接口设备位置与状态
        int storageInCount = m_config.getStorageInCount();
        for (int i = 0; i < storageInCount; ++i) {
            int deviceId = i + 1;
            DeviceState device;
            device.id = deviceId;
            device.type = DeviceType::StorageIn;
            device.status = (i % 3 == 0) ? DeviceStatus::working : DeviceStatus::idle;
            device.queuedTaskCount = i % 2;
            m_devices[deviceId] = device;
            
            // 计算位置（顶部扇形区域）
            float angle = M_PI / 7.0f * i - M_PI / 3.0f;
            m_devicePositions[deviceId] = sf::Vector2f(
                std::cos(angle) * outerRadius,
                std::sin(angle) * outerRadius * 0.7f
            );
        }
        
        // 出库接口设备位置与状态
        int storageOutCount = m_config.getStorageOutCount();
        for (int i = 0; i < storageOutCount; ++i) {
            int deviceId = i + storageInCount + 1;
            DeviceState device;
            device.id = deviceId;
            device.type = DeviceType::StorageOut;
            device.status = (i % 4 == 0) ? DeviceStatus::working : DeviceStatus::idle;
            device.queuedTaskCount = (i + 1) % 3;
            m_devices[deviceId] = device;
            
            // 计算位置（底部扇形区域）
            float angle = M_PI + M_PI / 7.0f * i - M_PI / 3.0f;
            m_devicePositions[deviceId] = sf::Vector2f(
                std::cos(angle) * outerRadius,
                std::sin(angle) * outerRadius * 0.7f
            );
        }
        
        // 出库作业站位置与状态
        int workstationOutCount = m_config.getWorkstationOutCount();
        for (int i = 0; i < workstationOutCount; ++i) {
            int deviceId = i + storageInCount + storageOutCount + 1;
            DeviceState device;
            device.id = deviceId;
            device.type = DeviceType::WorkstationOut;
            device.status = (i % 2 == 0) ? DeviceStatus::working : DeviceStatus::idle;
            device.queuedTaskCount = i % 2;
            m_devices[deviceId] = device;
            
            // 计算位置（左侧垂直排列）
            m_devicePositions[deviceId] = sf::Vector2f(
                -outerRadius - 20.0f,
                -60.0f + i * 60.0f
            );
        }
        
        // 入库作业站位置与状态
        int workstationInCount = m_config.getWorkstationInCount();
        for (int i = 0; i < workstationInCount; ++i) {
            int deviceId = i + storageInCount + storageOutCount + workstationOutCount + 1;
            DeviceState device;
            device.id = deviceId;
            device.type = DeviceType::WorkstationIn;
            device.status = (i % 3 == 0) ? DeviceStatus::working : DeviceStatus::idle;
            device.queuedTaskCount = (i - 15) % 2;
            m_devices[deviceId] = device;
            
            // 计算位置（右侧垂直排列）
            m_devicePositions[deviceId] = sf::Vector2f(
                outerRadius + 20.0f,
                -60.0f + i * 60.0f
            );
        }
    }
    
    // 创建示例任务
    void createSampleTasks() {
        sf::Clock clock;
        
        // 添加一些示例任务
        Task t1;
        t1.taskId = 101;
        t1.type = TaskType::input;
        t1.materialId = 5001;
        t1.startDeviceId = 2;
        t1.endDeviceId = 15;
        t1.createTime = clock.getElapsedTime();
        t1.startTime = clock.getElapsedTime() + sf::seconds(5);
        t1.completeTime = sf::Time::Zero;
        t1.assignedVehicleId = 1;
        m_tasks.push_back(t1);
        
        Task t2;
        t2.taskId = 102;
        t2.type = TaskType::output;
        t2.materialId = 5002;
        t2.startDeviceId = 14;
        t2.endDeviceId = 8;
        t2.createTime = clock.getElapsedTime() + sf::seconds(2);
        t2.startTime = clock.getElapsedTime() + sf::seconds(10);
        t2.completeTime = sf::Time::Zero;
        t2.assignedVehicleId = 2;
        m_tasks.push_back(t2);
        
        Task t3;
        t3.taskId = 103;
        t3.type = TaskType::input;
        t3.materialId = 5003;
        t3.startDeviceId = 3;
        t3.endDeviceId = 16;
        t3.createTime = clock.getElapsedTime() + sf::seconds(4);
        t3.startTime = sf::Time::Zero;
        t3.completeTime = sf::Time::Zero;
        t3.assignedVehicleId = -1;
        m_tasks.push_back(t3);
    }
};
