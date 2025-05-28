#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <map>               // For std::map
#include "gui/SimObject.hpp" // For SimObject base class and other GUI types
#include "Core/Device.hpp"   // For Core::DeviceType and Core::DeviceBase (if needed for constructor)

namespace gui
{

    // Matches the Core::DeviceType enum for direct mapping
    enum class DeviceType
    {
        StorageIn,     // 入库接口设备
        StorageOut,    // 出库接口设备
        WorkstationIn, // 入库作业口
        WorkstationOut // 出库作业口
    };

    // Matches the Core::DeviceState struct's relevant parts for display
    struct DeviceStatus
    {
        bool has_goods = false;
        bool is_reserved = false;
        // Add other fields from Core::DeviceState if they need to be visualized
    };

    // Helper function to convert Core::DeviceType to gui::DeviceType
    inline gui::DeviceType coreToGuiDeviceType(Core::DeviceType coreType)
    {
        switch (coreType)
        {
        case Core::DeviceType::StorageIn:
            return gui::DeviceType::StorageIn;
        case Core::DeviceType::StorageOut:
            return gui::DeviceType::StorageOut;
        case Core::DeviceType::WorkstationIn:
            return gui::DeviceType::WorkstationIn;
        case Core::DeviceType::WorkstationOut:
            return gui::DeviceType::WorkstationOut;
        default:
            // Handle unknown case, maybe throw or return a default
            throw std::runtime_error("Unknown Core::DeviceType");
        }
    }

    // Helper function to update gui::DeviceStatus from Core::DeviceState
    // This is not a direct enum mapping like DeviceType.
    // It copies relevant fields.
    inline gui::DeviceStatus coreToGuiDeviceStatus(const Core::DeviceState &coreState)
    {
        gui::DeviceStatus guiStatus;
        guiStatus.has_goods = coreState.has_goods;
        guiStatus.is_reserved = coreState.is_reserved;
        // map other fields as needed
        return guiStatus;
    }

    struct DeviceState : public SimObject
    {
        DeviceType type;         // Type of the device
        DeviceStatus status;     // Current status of the device (has_goods, is_reserved)
        std::string currentTask; // ID of the task currently assigned or being processed
        float 작업진행도;        // Progress of the current operation (0.0 to 1.0)
        // sf::Color deviceColor;   // Base color, might be determined by type or status

        // Default constructor
        DeviceState()
            : SimObject(SimObjectType::Device, ""),
              type(gui::DeviceType::StorageIn), // Default type
              작업진행도(0.0f)
        // deviceColor(sf::Color::White)
        {
            // status will be default constructed
        }

        // Constructor to initialize from Core::DeviceBase (or relevant parts of it)
        // This constructor needs access to Core::DeviceBase definition.
        DeviceState(const Core::DeviceBase &coreDevice, const sf::Vector2f &_pos)
            : SimObject(SimObjectType::Device, std::to_string(coreDevice.m_id)),
              type(coreToGuiDeviceType(coreDevice.m_type)),      // m_type needs to be accessible from Core::DeviceBase
              status(coreToGuiDeviceStatus(coreDevice.m_status)) // m_status needs to be accessible
        {
            screenPosition = _pos;
            // Initialize other members like 작업진행도, currentTask as needed
            // deviceColor = determineColorByType(type); // Example
        }

        // Add other constructors or methods as needed
    };

} // namespace gui
