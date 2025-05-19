#pragma once
#include "SimulationInterface.hpp"
#include <random>
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <atomic>

/**
 * @brief 模拟仿真接口类
 * 
 * 用于测试前端GUI，提供模拟的后端数据
 * 包括生成模拟车辆运动、仓库状态变化等功能
 */
class MockSimulationInterface : public SimulationInterface {
private:
    // 轨道参数
    float m_trackLength = 126000.0f;  // 轨道总长度（毫米）
    float m_curveRadius = 2500.0f;    // 弯道半径（毫米）
    float m_straightLength = 40000.0f; // 直道长度（毫米，单向）

    // 仿真状态
    std::vector<VehicleState> m_vehicles;       // 车辆状态集合
    std::vector<WarehouseState> m_warehouses;   // 仓库状态集合
    float m_simulationSpeed = 1.0f;             // 仿真速度倍率
    std::atomic<bool> m_isPaused{false};        // 是否暂停

    // 回调函数
    VehicleUpdateCallback m_vehicleUpdateCallback = nullptr;
    WarehouseUpdateCallback m_warehouseUpdateCallback = nullptr;
    SimSpeedUpdateCallback m_simSpeedUpdateCallback = nullptr;
    SimPauseUpdateCallback m_pauseUpdateCallback = nullptr;

    // 模拟线程控制
    std::thread m_simulationThread;             // 模拟线程
    std::atomic<bool> m_isRunning{true};        // 线程运行标志
    std::mutex m_mutex;                         // 数据访问互斥锁
    std::condition_variable m_cv;               // 条件变量，用于暂停控制

    // 随机数生成器
    std::mt19937 m_rng;
    std::uniform_real_distribution<float> m_speedDist{5.0f, 150.0f}; // 速度分布（毫米/秒）
    std::uniform_int_distribution<int> m_warehouseLoadDist{0, 100};  // 仓库负载分布

public:
    /**
     * @brief 构造函数
     * @param vehicleCount 模拟车辆数量
     * @param warehouseCount 模拟仓库/接口设备数量
     */
    MockSimulationInterface(int vehicleCount = 3, int warehouseCount = 6) {
        initializeVehicles(vehicleCount);
        initializeWarehouses(warehouseCount);
        startSimulationThread();
    }

    /**
     * @brief 析构函数，停止模拟线程
     */
    ~MockSimulationInterface() override {
        m_isRunning = false;
        m_cv.notify_all();
        if (m_simulationThread.joinable()) {
            m_simulationThread.join();
        }
    }

    // 实现SimulationInterface接口
    int getVehicleCount() const override {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_vehicles.size();
    }

    std::vector<VehicleState> getVehicleStates() const override {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_vehicles;
    }

    int getWarehouseCount() const override {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_warehouses.size();
    }

    std::vector<WarehouseState> getWarehouseStates() const override {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_warehouses;
    }

    float getTrackLength() const override {
        return m_trackLength;
    }

    float getCurveRadius() const override {
        return m_curveRadius;
    }

    float getSimulationSpeed() const override {
        return m_simulationSpeed;
    }

    bool isPaused() const override {
        return m_isPaused;
    }

    void setSimulationSpeed(float speed) override {
        m_simulationSpeed = speed;
        if (m_simSpeedUpdateCallback) {
            m_simSpeedUpdateCallback(speed);
        }
    }

    void setPaused(bool paused) override {
        m_isPaused = paused;
        if (m_pauseUpdateCallback) {
            m_pauseUpdateCallback(paused);
        }
        m_cv.notify_all();
    }

    void setVehicleUpdateCallback(VehicleUpdateCallback callback) override {
        m_vehicleUpdateCallback = callback;
    }

    void setWarehouseUpdateCallback(WarehouseUpdateCallback callback) override {
        m_warehouseUpdateCallback = callback;
    }

    void setSimSpeedUpdateCallback(SimSpeedUpdateCallback callback) override {
        m_simSpeedUpdateCallback = callback;
    }

    void setPauseUpdateCallback(SimPauseUpdateCallback callback) override {
        m_pauseUpdateCallback = callback;
    }

private:
    /**
     * @brief 初始化模拟车辆
     * @param count 车辆数量
     */
    void initializeVehicles(int count) {
        std::uniform_real_distribution<float> positionDist(0.0f, m_trackLength);
        
        for (int i = 0; i < count; ++i) {
            VehicleState vehicle;
            vehicle.id = i;
            vehicle.trackPosition = positionDist(m_rng);  // 随机初始位置
            vehicle.speed = m_speedDist(m_rng);           // 随机初始速度
            vehicle.isLoaded = (i % 2 == 0);              // 一半车辆载货
            vehicle.currentTaskId = (i % 3 == 0) ? i : -1; // 三分之一车辆有任务
            
            m_vehicles.push_back(vehicle);
        }
    }

    /**
     * @brief 初始化模拟仓库/接口设备
     * @param count 仓库/接口设备数量
     */
    void initializeWarehouses(int count) {
        // 均匀分布仓库/接口设备
        float step = m_trackLength / count;
        
        for (int i = 0; i < count; ++i) {
            WarehouseState warehouse;
            warehouse.id = i;
            warehouse.trackPosition = i * step;          // 均匀分布
            warehouse.isInterface = (i % 2 == 0);        // 一半是接口设备
            warehouse.capacity = warehouse.isInterface ? 5 : 50;  // 接口容量小，仓库容量大
            warehouse.currentLoad = m_warehouseLoadDist(m_rng) % warehouse.capacity; // 随机负载
            
            m_warehouses.push_back(warehouse);
        }
    }

    /**
     * @brief 启动模拟线程
     */
    void startSimulationThread() {
        m_simulationThread = std::thread([this]() {
            using namespace std::chrono;
            
            auto lastUpdateTime = high_resolution_clock::now();
            
            while (m_isRunning) {
                // 处理暂停
                {
                    std::unique_lock<std::mutex> lock(m_mutex);
                    if (m_isPaused) {
                        m_cv.wait(lock, [this]() { return !m_isPaused || !m_isRunning; });
                        if (!m_isRunning) break;
                        lastUpdateTime = high_resolution_clock::now();
                    }
                }
                
                // 计算时间增量
                auto currentTime = high_resolution_clock::now();
                float deltaTime = duration_cast<milliseconds>(currentTime - lastUpdateTime).count() / 1000.0f;
                deltaTime *= m_simulationSpeed;  // 应用仿真速度倍率
                lastUpdateTime = currentTime;
                
                // 更新车辆位置
                updateVehicles(deltaTime);
                
                // 更新仓库状态（偶尔）
                if (rand() % 100 < 5) {
                    updateWarehouses();
                }
                
                // 触发回调
                if (m_vehicleUpdateCallback) {
                    m_vehicleUpdateCallback(m_vehicles);
                }
                
                if (m_warehouseUpdateCallback) {
                    m_warehouseUpdateCallback(m_warehouses);
                }
                
                // 控制更新频率，避免CPU占用过高
                std::this_thread::sleep_for(milliseconds(16));  // 约60Hz
            }
        });
    }

    /**
     * @brief 更新车辆位置
     * @param deltaTime 时间增量（秒）
     */
    void updateVehicles(float deltaTime) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        for (auto& vehicle : m_vehicles) {
            // 更新位置
            vehicle.trackPosition += vehicle.speed * deltaTime;
            
            // 确保位置在轨道范围内（循环）
            while (vehicle.trackPosition >= m_trackLength) {
                vehicle.trackPosition -= m_trackLength;
            }
            
            // 随机变化速度（小概率）
            if (rand() % 100 < 5) {
                vehicle.speed = m_speedDist(m_rng);
            }
            
            // 随机变化载货状态（极小概率）
            if (rand() % 1000 < 5) {
                vehicle.isLoaded = !vehicle.isLoaded;
            }
            
            // 随机变化任务状态（小概率）
            if (rand() % 500 < 5) {
                vehicle.currentTaskId = (vehicle.currentTaskId < 0) ? rand() % 10 : -1;
            }
        }
    }

    /**
     * @brief 更新仓库状态
     */
    void updateWarehouses() {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        for (auto& warehouse : m_warehouses) {
            // 随机变化当前负载
            int change = (rand() % 3) - 1;  // -1, 0, 或 1
            warehouse.currentLoad = std::max(0, std::min(warehouse.capacity, warehouse.currentLoad + change));
        }
    }
};
