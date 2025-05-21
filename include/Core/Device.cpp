#include "Device.hpp"

namespace Core
{

    bool StorageInDevice::update(float deltaTime)
    {
        if (m_status == DeviceStatus::working && m_currentTask)
        {
            // 检查处理时间是否达到
            if (m_processingTimer.getElapsedTime().asSeconds() >= 5.0f)
            { // 假设处理时间为5秒
                m_status = DeviceStatus::idle;
                m_currentTask = nullptr;
                return true; // 状态发生变更
            }
        }
        return false;
    }

    void StorageInDevice::notifyCargoPickup(bool success)
    {
        if (success)
        {
            m_readyForUnload = true;
        }
    }

    bool WorkstationOutDevice::update(float deltaTime)
    {
        if (m_status == DeviceStatus::working && m_currentTask)
        {
            // 检查处理时间是否达到
            if (m_processingTimer.getElapsedTime().asSeconds() >= 10.0f)
            { // 假设处理时间为10秒
                m_status = DeviceStatus::idle;
                m_currentTask = nullptr;
                return true; // 状态发生变更
            }
        }
        return false;
    }

    void WorkstationOutDevice::notifyManualUnloadComplete()
    {
        if (m_status == DeviceStatus::working)
        {
            m_status = DeviceStatus::idle;
            m_currentTask = nullptr;
        }
    }

} // namespace Core
