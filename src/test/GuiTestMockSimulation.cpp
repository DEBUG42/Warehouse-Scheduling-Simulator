#include "test/GuiTestMockSimulation.hpp"
#include <SFML/System/Time.hpp>
#include <algorithm> // For std::find_if
#include <string>    // For std::stoi
#include <iostream>  // For debug output

namespace test
{

    // Helper to convert sim::VehicleStatus to gui::VehicleStatus
    gui::VehicleStatus convertSimToGuiVehicleStatus(sim::VehicleStatus simStatus)
    {
        switch (simStatus)
        {
        case sim::VehicleStatus::IDLE:
            return gui::VehicleStatus::IDLE;
        case sim::VehicleStatus::MOVING_TO_PICKUP:
            return gui::VehicleStatus::MOVING_TO_LOAD;
        case sim::VehicleStatus::PICKING_UP:
            return gui::VehicleStatus::LOADING;
        case sim::VehicleStatus::MOVING_TO_DROPOFF:
            return gui::VehicleStatus::MOVING_TO_UNLOAD;
        case sim::VehicleStatus::DROPPING_OFF:
            return gui::VehicleStatus::UNLOADING;
        case sim::VehicleStatus::CHARGING:
            return gui::VehicleStatus::CHARGING;
        case sim::VehicleStatus::ERROR:
            return gui::VehicleStatus::ERROR;
        default:
            return gui::VehicleStatus::UNKNOWN;
        }
    }

    // Updated Helper to convert sim::DeviceOperationalStatus to gui::DeviceStatus, considering task count
    gui::DeviceStatus convertSimToGuiDeviceStatus(sim::DeviceOperationalStatus simOpStatus, int taskCount)
    {
        switch (simOpStatus)
        {
        case sim::DeviceOperationalStatus::OPERATIONAL:
            return (taskCount > 0) ? gui::DeviceStatus::BUSY : gui::DeviceStatus::IDLE;
        case sim::DeviceOperationalStatus::OUT_OF_SERVICE:
            return gui::DeviceStatus::OFFLINE;
        case sim::DeviceOperationalStatus::BUSY: // This state from mock might imply it's actively processing
            return gui::DeviceStatus::WORKING;   // or BUSY if it's just queued. Let's map to WORKING for now.
        default:
            return gui::DeviceStatus::UNKNOWN_DEVICE_STATUS;
        }
    }

    GuiTestMockSimulation::GuiTestMockSimulation(float trackTotalLengthMm)
        : m_mockSim(trackTotalLengthMm)
    {
        m_currentState.isPaused = false;
        m_currentState.simulationSpeedFactor = 1.0f;
        m_currentState.simulationTime = 0.0f;
        m_currentState.vehicleCount = 0;
        m_currentState.completedTaskCount = 0;
        m_currentState.pendingTaskCount = 0;
    }

    void GuiTestMockSimulation::update(sf::Time deltaTime)
    {
        if (!m_currentState.isPaused)
        {
            float dtSeconds = deltaTime.asSeconds() * m_currentState.simulationSpeedFactor;
            m_mockSim.update(sf::seconds(dtSeconds));
            m_timeAccumulator += dtSeconds;
            m_currentState.simulationTime = m_timeAccumulator;

            m_currentState.vehicleCount = m_mockSim.getVehicles().size();
            int completed = 0;
            int pending = 0;
            for (const auto &task : m_mockSim.getTasks())
            {
                if (task.status == sim::TaskStatus::COMPLETED)
                    completed++;
                else if (task.status == sim::TaskStatus::PENDING ||
                         task.status == sim::TaskStatus::ASSIGNED ||
                         task.status == sim::TaskStatus::PICKING_UP ||
                         task.status == sim::TaskStatus::TRANSPORTING ||
                         task.status == sim::TaskStatus::DROPPING_OFF)
                    pending++;
            }
            m_currentState.completedTaskCount = completed;
            m_currentState.pendingTaskCount = pending;

            if (m_stateCallback)
                m_stateCallback(getSimulationState());
            if (m_vehicleCallback)
                m_vehicleCallback(getVehicleStates());
            if (m_deviceCallback)
                m_deviceCallback(getDeviceStates());
        }
    }

    SimulationInterface::SimulationState GuiTestMockSimulation::getSimulationState() const
    {
        return m_currentState;
    }

    std::vector<gui::VehicleState> GuiTestMockSimulation::getVehicleStates() const
    {
        std::vector<gui::VehicleState> guiStates;
        const auto &mockVehicles = m_mockSim.getVehicles();
        guiStates.reserve(mockVehicles.size());
        for (const auto &mockVehicle : mockVehicles)
        {
            guiStates.push_back(convertToGuiVehicleState(mockVehicle));
        }
        return guiStates;
    }

    std::vector<gui::DeviceState> GuiTestMockSimulation::getDeviceStates() const
    {
        std::vector<gui::DeviceState> guiStates;
        const auto &mockDevicesMap = m_mockSim.getDeviceStates();
        guiStates.reserve(mockDevicesMap.size());
        for (const auto &pair : mockDevicesMap)
        {
            guiStates.push_back(convertToGuiDeviceState(pair.second));
        }
        return guiStates;
    }

    void GuiTestMockSimulation::setSimulationSpeedFactor(float speedFactor)
    {
        m_currentState.simulationSpeedFactor = std::max(0.0f, speedFactor);
        if (m_stateCallback)
            m_stateCallback(getSimulationState());
    }

    void GuiTestMockSimulation::pauseSimulation()
    {
        m_currentState.isPaused = true;
        if (m_stateCallback)
            m_stateCallback(getSimulationState());
    }

    void GuiTestMockSimulation::resumeSimulation()
    {
        m_currentState.isPaused = false;
        if (m_stateCallback)
            m_stateCallback(getSimulationState());
    }

    void GuiTestMockSimulation::resetSimulation()
    {
        // Re-initialize the internal mock simulation to its default scenario
        m_mockSim.initializeDefaultScenario();

        m_currentState.isPaused = false;
        m_currentState.simulationSpeedFactor = 1.0f;
        m_timeAccumulator = 0.0f;
        m_currentState.simulationTime = 0.0f;

        // Recalculate counts based on the reset scenario
        m_currentState.vehicleCount = m_mockSim.getVehicles().size();
        int completed = 0;
        int pending = 0;
        for (const auto &task : m_mockSim.getTasks())
        {
            if (task.status == sim::TaskStatus::COMPLETED)
                completed++;
            else if (task.status != sim::TaskStatus::FAILED)
                pending++; // Count all non-failed, non-completed as pending
        }
        m_currentState.completedTaskCount = completed;
        m_currentState.pendingTaskCount = pending;

        if (m_stateCallback)
            m_stateCallback(getSimulationState());
        if (m_vehicleCallback)
            m_vehicleCallback(getVehicleStates());
        if (m_deviceCallback)
            m_deviceCallback(getDeviceStates());
    }

    void GuiTestMockSimulation::registerStateUpdateCallback(StateUpdateCallback callback)
    {
        m_stateCallback = callback;
    }

    void GuiTestMockSimulation::registerVehicleUpdateCallback(VehicleUpdateCallback callback)
    {
        m_vehicleCallback = callback;
    }

    void GuiTestMockSimulation::registerDeviceUpdateCallback(DeviceUpdateCallback callback)
    {
        m_deviceCallback = callback;
    }

    gui::VehicleState GuiTestMockSimulation::getVehicleState(int vehicleId) const
    {
        auto mockVehicleOpt = m_mockSim.getVehicleById(vehicleId);
        if (mockVehicleOpt)
        {
            return convertToGuiVehicleState(*mockVehicleOpt);
        }
        gui::VehicleState defaultState(std::to_string(vehicleId), sf::Vector2f(0, 0),
                                       0.0f, 0.0f, gui::VehicleStatus::UNKNOWN,
                                       "", false, gui::CargoDisplayInfo(), 0.0f);
        return defaultState;
    }

    gui::DeviceState GuiTestMockSimulation::getDeviceState(int deviceId) const
    {
        const auto &devicesMap = m_mockSim.getDeviceStates();
        auto it = std::find_if(devicesMap.begin(), devicesMap.end(),
                               [deviceId](const auto &pair)
                               {
                                   const std::string &sId = pair.first;
                                   size_t firstDigitPos = sId.find_first_of("0123456789");
                                   if (firstDigitPos != std::string::npos)
                                   {
                                       try
                                       {
                                           return std::stoi(sId.substr(firstDigitPos)) == deviceId;
                                       }
                                       catch (const std::exception &)
                                       {
                                           return false;
                                       }
                                   }
                                   return false;
                               });

        if (it != devicesMap.end())
        {
            return convertToGuiDeviceState(it->second);
        }

        // Try to find by exact string match if int ID was meant to be a string representation
        std::string deviceIdStr = std::to_string(deviceId);
        auto itStr = devicesMap.find(deviceIdStr);
        if (itStr != devicesMap.end())
        {
            return convertToGuiDeviceState(itStr->second);
        }

        gui::DeviceState defaultState("Device_" + std::to_string(deviceId), sf::Vector2f(0, 0),
                                      gui::DeviceType::UNKNOWN_DEVICE_TYPE,
                                      gui::DeviceStatus::UNKNOWN_DEVICE_STATUS, "");
        return defaultState;
    }

    gui::VehicleState GuiTestMockSimulation::convertToGuiVehicleState(const sim::MockVehicle &mockVehicle) const
    {
        // Position is sf::Vector2f, will be calculated by renderer using rawTrackPositionMm.
        // For now, VehicleState constructor requires a position, so pass (0,0).
        sf::Vector2f placeholderPos(0.f, 0.f);

        // Determine isLoaded (heuristic, mockVehicle doesn't have explicit loaded state)
        bool isLoaded = false;
        if (mockVehicle.status == sim::VehicleStatus::MOVING_TO_DROPOFF ||
            mockVehicle.status == sim::VehicleStatus::DROPPING_OFF)
        {
            isLoaded = true;
        }
        // Cargo info (mock, as sim::MockVehicle doesn't store cargo details)
        gui::CargoDisplayInfo cargoInfo;
        // if (isLoaded) { cargoInfo.materialId = 1; cargoInfo.quantity = 1; } // Example if loaded

        gui::VehicleState state(
            std::to_string(mockVehicle.id),
            placeholderPos,
            mockVehicle.currentPositionMm,
            mockVehicle.speedMmPerSecond / 1000.0f, // mm/s to m/s
            convertSimToGuiVehicleStatus(mockVehicle.status),
            (mockVehicle.currentTaskId == -1) ? "" : std::to_string(mockVehicle.currentTaskId),
            isLoaded,
            cargoInfo,
            1.0f // batteryLevel (mocked as full)
        );
        return state;
    }

    gui::DeviceState GuiTestMockSimulation::convertToGuiDeviceState(const sim::MockDeviceState &mockDevice) const
    {
        // Position is sf::Vector2f, will be set by WarehouseRenderer.
        // For now, DeviceState constructor requires a position, so pass (0,0).
        sf::Vector2f placeholderPos(0.f, 0.f);

        // Determine gui::DeviceType based on mockDevice.id (heuristic)
        gui::DeviceType deviceType = gui::DeviceType::UNKNOWN_DEVICE_TYPE;
        if (mockDevice.id.find("INPUT") != std::string::npos || mockDevice.id.find("Input") != std::string::npos || mockDevice.id.find("in") != std::string::npos)
        {
            deviceType = gui::DeviceType::INPUT_STATION;
        }
        else if (mockDevice.id.find("OUTPUT") != std::string::npos || mockDevice.id.find("Output") != std::string::npos || mockDevice.id.find("out") != std::string::npos)
        {
            deviceType = gui::DeviceType::OUTPUT_STATION;
        }
        else if (mockDevice.id.find("WORK") != std::string::npos || mockDevice.id.find("Work") != std::string::npos)
        {
            // Could be CORE_WORKSTATION_IN or CORE_WORKSTATION_OUT, needs more info from mockDevice if available
            deviceType = gui::DeviceType::WORK_STATION;
        }
        else if (mockDevice.id.find("CHARGE") != std::string::npos || mockDevice.id.find("Charger") != std::string::npos)
        {
            deviceType = gui::DeviceType::CHARGER;
        }
        // Potentially refine with CORE_* types if mockDevice had more specific type info

        gui::DeviceState state(
            mockDevice.id, // Already a string
            placeholderPos,
            deviceType,
            convertSimToGuiDeviceStatus(mockDevice.operationalStatus, mockDevice.currentTaskCount),
            "" // boundVehicleId (mock does not track this directly)
        );
        // state.capacity = ... ; // mockDevice does not have capacity
        state.queuedTaskCount = mockDevice.currentTaskCount;
        // state.currentLoad = ...; // mockDevice does not have currentLoad
        // state.materialId = ...; // mockDevice does not have materialId
        // state.processingProgress = ...; // mockDevice does not have processingProgress

        return state;
    }

    // Required by SimulationInterface, but not strictly needed for this mock's primary GUI test path
    // if GUITestMain does not call these directly.
    std::optional<Core::Task> GuiTestMockSimulation::getTaskDetails(const std::string &taskId) const
    {
        try
        {
            int numericTaskId = std::stoi(taskId);
            auto mockTaskOpt = m_mockSim.getTaskById(numericTaskId);
            if (mockTaskOpt)
            {
                const auto &mockTask = *mockTaskOpt;
                Core::TaskType taskType = Core::TaskType::MOVE; // Default
                // Basic type inference based on description or device IDs
                if (mockTask.description.find("Input") != std::string::npos || mockTask.description.find("input") != std::string::npos)
                {
                    taskType = Core::TaskType::INPUT;
                }
                else if (mockTask.description.find("Output") != std::string::npos || mockTask.description.find("output") != std::string::npos)
                {
                    taskType = Core::TaskType::OUTPUT;
                }

                int materialId = 0; // Default materialId
                int startDevId = 0;
                int endDevId = 0;

                try
                {
                    if (!mockTask.pickupDeviceId.empty() && mockTask.pickupDeviceId != "null" && mockTask.pickupDeviceId != "None")
                    {
                        // Attempt to remove non-numeric prefix if any, e.g., "D1" -> "1"
                        std::string numericPartStart = mockTask.pickupDeviceId;
                        size_t firstDigitPosStart = numericPartStart.find_first_of("0123456789");
                        if (firstDigitPosStart != std::string::npos)
                        {
                            numericPartStart = numericPartStart.substr(firstDigitPosStart);
                        }
                        startDevId = std::stoi(numericPartStart);
                    }
                    if (!mockTask.dropoffDeviceId.empty() && mockTask.dropoffDeviceId != "null" && mockTask.dropoffDeviceId != "None")
                    {
                        std::string numericPartEnd = mockTask.dropoffDeviceId;
                        size_t firstDigitPosEnd = numericPartEnd.find_first_of("0123456789");
                        if (firstDigitPosEnd != std::string::npos)
                        {
                            numericPartEnd = numericPartEnd.substr(firstDigitPosEnd);
                        }
                        endDevId = std::stoi(numericPartEnd);
                    }
                }
                catch (const std::invalid_argument &ia)
                {
                    std::cerr << "[GuiTestMockSim] Invalid argument for stoi converting device ID for task " << mockTask.id << ": " << ia.what() << std::endl;
                }
                catch (const std::out_of_range &oor)
                {
                    std::cerr << "[GuiTestMockSim] Out of range for stoi converting device ID for task " << mockTask.id << ": " << oor.what() << std::endl;
                }

                return Core::Task(
                    std::to_string(mockTask.id),
                    taskType,
                    materialId,
                    startDevId,
                    endDevId);
            }
        }
        catch (const std::invalid_argument &ia)
        {
            std::cerr << "[GuiTestMockSim] Invalid argument: Cannot convert task ID '" << taskId << "' to int: " << ia.what() << std::endl;
        }
        catch (const std::out_of_range &oor)
        {
            std::cerr << "[GuiTestMockSim] Out of range: Cannot convert task ID '" << taskId << "' to int: " << oor.what() << std::endl;
        }
        return std::nullopt;
    }

    std::vector<Core::Task> GuiTestMockSimulation::getAllTasks() const
    {
        std::vector<Core::Task> coreTasks;
        const auto &mockTasks = m_mockSim.getTasks();
        coreTasks.reserve(mockTasks.size());
        for (const auto &mockTask : mockTasks)
        {
            Core::TaskType taskType = Core::TaskType::MOVE; // Default
            if (mockTask.description.find("Input") != std::string::npos || mockTask.description.find("input") != std::string::npos)
            {
                taskType = Core::TaskType::INPUT;
            }
            else if (mockTask.description.find("Output") != std::string::npos || mockTask.description.find("output") != std::string::npos)
            {
                taskType = Core::TaskType::OUTPUT;
            }

            int materialId = 0; // Default materialId
            int startDevId = 0;
            int endDevId = 0;

            try
            {
                if (!mockTask.pickupDeviceId.empty() && mockTask.pickupDeviceId != "null" && mockTask.pickupDeviceId != "None")
                {
                    std::string numericPartStart = mockTask.pickupDeviceId;
                    size_t firstDigitPosStart = numericPartStart.find_first_of("0123456789");
                    if (firstDigitPosStart != std::string::npos)
                    {
                        numericPartStart = numericPartStart.substr(firstDigitPosStart);
                    }
                    startDevId = std::stoi(numericPartStart);
                }
                if (!mockTask.dropoffDeviceId.empty() && mockTask.dropoffDeviceId != "null" && mockTask.dropoffDeviceId != "None")
                {
                    std::string numericPartEnd = mockTask.dropoffDeviceId;
                    size_t firstDigitPosEnd = numericPartEnd.find_first_of("0123456789");
                    if (firstDigitPosEnd != std::string::npos)
                    {
                        numericPartEnd = numericPartEnd.substr(firstDigitPosEnd);
                    }
                    endDevId = std::stoi(numericPartEnd);
                }
            }
            catch (const std::invalid_argument &ia)
            {
                std::cerr << "[GuiTestMockSim] Invalid argument for stoi converting device ID for task " << mockTask.id << ": " << ia.what() << std::endl;
            }
            catch (const std::out_of_range &oor)
            {
                std::cerr << "[GuiTestMockSim] Out of range for stoi converting device ID for task " << mockTask.id << ": " << oor.what() << std::endl;
            }

            coreTasks.emplace_back(
                std::to_string(mockTask.id),
                taskType,
                materialId,
                startDevId,
                endDevId);
        }
        return coreTasks;
    }

    void GuiTestMockSimulation::createTask(const Core::Task &task) { /* Not implemented for this mock */ }
    void GuiTestMockSimulation::cancelTask(const std::string &taskId) { /* Not implemented for this mock */ }
    void GuiTestMockSimulation::setVehicleRoute(int vehicleId, const std::vector<int> &routeNodeIds) { /* Not implemented */ }
    std::string GuiTestMockSimulation::getVehicleCurrentLocationNode(int vehicleId) const { return ""; /* Not implemented */ }
    void GuiTestMockSimulation::sendVehicleToCharge(int vehicleId) { /* Not implemented */ }

} // namespace test