#include "SimulationEngine.hpp"
#include <algorithm>

namespace Core
{

    SimulationEngine::SimulationEngine()
        : m_simulationTime(0.0f), m_simulationSpeed(1.0f), m_isPaused(false)
    {
    }

    void SimulationEngine::addVehicle(std::shared_ptr<Vehicle> vehicle)
    {
        m_vehicles.push_back(vehicle);
    }

    void SimulationEngine::addDevice(std::shared_ptr<DeviceBase> device)
    {
        m_devices[device->getId()] = device;
    }

    void SimulationEngine::addTask(const Task &task)
    {
        if (task.validate(m_devices))
        {
            m_taskQueue.push(task);
        }
    }

    void SimulationEngine::update(float deltaTime)
    {
        if (m_isPaused)
        {
            return;
        }

        // 更新仿真时间
        m_simulationTime += deltaTime * m_simulationSpeed;

        // 更新所有设备
        for (auto &device : m_devices)
        {
            device.second->update(deltaTime * m_simulationSpeed);
        }

        // 更新所有车辆
        for (auto &vehicle : m_vehicles)
        {
            // 找到前车
            const Vehicle *leadingVehicle = nullptr;
            float minDistance = std::numeric_limits<float>::max();

            for (const auto &otherVehicle : m_vehicles)
            {
                if (otherVehicle != vehicle)
                {
                    float distance = otherVehicle->getTrackPositionMm() - vehicle->getTrackPositionMm();
                    if (distance > 0 && distance < minDistance)
                    {
                        minDistance = distance;
                        leadingVehicle = otherVehicle.get();
                    }
                }
            }

            // 获取当前轨道段信息（这里需要实现）
            Physics::TrackSegment currentTrack(Physics::TrackSegment::Type::STRAIGHT, 1000.0f);

            // 更新车辆物理状态
            vehicle->updatePhysics(deltaTime * m_simulationSpeed, leadingVehicle, currentTrack);
        }

        // 处理任务队列
        while (!m_taskQueue.empty())
        {
            const Task &task = m_taskQueue.front();

            // 查找空闲车辆
            auto it = std::find_if(m_vehicles.begin(), m_vehicles.end(),
                                   [](const std::shared_ptr<Vehicle> &v)
                                   {
                                       return v->getMotionState() == Vehicle::MotionState::IDLE;
                                   });

            if (it != m_vehicles.end())
            {
                // 分配任务给车辆
                auto device = m_devices[task.startDeviceId];
                if (device)
                {
                    (*it)->startTask(task, *device);
                }
                m_taskQueue.pop();
            }
            else
            {
                break; // 没有空闲车辆，等待下一帧
            }
        }
    }

    void SimulationEngine::setSimulationSpeed(float speed)
    {
        m_simulationSpeed = std::max(0.0f, std::min(10.0f, speed));
    }

    void SimulationEngine::setPaused(bool paused)
    {
        m_isPaused = paused;
    }

    float SimulationEngine::getSimulationTime() const
    {
        return m_simulationTime;
    }

    float SimulationEngine::getSimulationSpeed() const
    {
        return m_simulationSpeed;
    }

    bool SimulationEngine::isPaused() const
    {
        return m_isPaused;
    }

} // namespace Core
