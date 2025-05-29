#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <map>                    // For std::map
#include "../src/Core/Device.hpp" // For DeviceType and DeviceBase

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
    }; // Helper function to convert ::DeviceType to gui::DeviceType
    inline gui::DeviceType coreToGuiDeviceType(::DeviceType coreType)
    {
        switch (coreType)
        {
        case ::DeviceType::StorageIn:
            return gui::DeviceType::StorageIn;
        case ::DeviceType::StorageOut:
            return gui::DeviceType::StorageOut;
        case ::DeviceType::WorkstationIn:
            return gui::DeviceType::WorkstationIn;
        case ::DeviceType::WorkstationOut:
            return gui::DeviceType::WorkstationOut;
        default:
            // Handle unknown case, maybe throw or return a default
            throw std::runtime_error("Unknown DeviceType");
        }
    }

    // Helper function to update gui::DeviceStatus from ::DeviceState
    // This is not a direct enum mapping like DeviceType.
    // It copies relevant fields.
    inline gui::DeviceStatus coreToGuiDeviceStatus(const ::DeviceState &coreState)
    {
        gui::DeviceStatus guiStatus;
        guiStatus.has_goods = coreState.has_goods;
        guiStatus.is_reserved = coreState.is_reserved;
        // map other fields as needed
        return guiStatus;
    }
    struct DeviceState
    {
        int id;                      // Device ID
        DeviceType type;             // Type of the device
        DeviceStatus status;         // Current status of the device (has_goods, is_reserved)
        std::string currentTask;     // ID of the task currently assigned or being processed
        float 작업진행도;            // Progress of the current operation (0.0 to 1.0)
        sf::Vector2f screenPosition; // Screen position for rendering
        // sf::Color deviceColor;   // Base color, might be determined by type or status

        // Default constructor
        DeviceState()
            : id(0),
              type(gui::DeviceType::StorageIn), // Default type
              작업진행도(0.0f),
              screenPosition(0.0f, 0.0f)
        // deviceColor(sf::Color::White)
        {
            // status will be default constructed
        }

        // Constructor to initialize from DeviceBase (or relevant parts of it)
        // This constructor needs access to DeviceBase definition.
        DeviceState(const DeviceBase &coreDevice, const sf::Vector2f &_pos)
            : id(coreDevice.m_id),
              type(coreToGuiDeviceType(coreDevice.m_type)),       // m_type needs to be accessible from DeviceBase
              status(coreToGuiDeviceStatus(coreDevice.m_status)), // m_status needs to be accessible
              screenPosition(_pos),
              작업진행도(0.0f)
        {
            // Initialize other members like currentTask as needed
            // deviceColor = determineColorByType(type); // Example
        }

        // Add other constructors or methods as needed
    };

} // namespace gui
