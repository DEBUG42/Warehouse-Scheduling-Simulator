#include "gui/MockSimulationInterface.hpp"
#include <algorithm> // For std::find_if, std::remove_if
#include <cmath>     // For std::fmod, std::abs
#include <iostream>  // For debug output (std::cout, std::cerr)
#include <limits>    // For std::numeric_limits

namespace sim
{

    // Helper for floating point comparison
    bool basically_equal(float a, float b, float epsilon = 0.1f)
    {
        return std::abs(a - b) <= epsilon;
    }

    MockSimulationInterface::MockSimulationInterface(float trackTotalLengthMm)
        : m_trackTotalLengthMm(trackTotalLengthMm > 0 ? trackTotalLengthMm : 90000.0f) // Default if invalid
    {
        if (trackTotalLengthMm <= 0)
        {
            std::cerr << "Warning: MockSimulationInterface initialized with invalid trackTotalLengthMm." << std::endl;
        }
        // initializeDefaultScenario(); // Can be called externally if needed
    }

    void MockSimulationInterface::update(sf::Time deltaTime)
    {
        float dtSeconds = deltaTime.asSeconds();

        // Update action timers
        for (auto it = m_vehicleActionTimers.begin(); it != m_vehicleActionTimers.end(); /* manual increment */)
        {
            it->second -= dtSeconds;
            if (it->second <= 0)
            {
                it = m_vehicleActionTimers.erase(it); // Remove expired timers
            }
            else
            {
                ++it;
            }
        }

        // Update vehicles
        for (MockVehicle &vehicle : m_vehicles)
        {
            updateVehicleMovement(vehicle, dtSeconds);
            processVehicleLogic(vehicle, dtSeconds); // Process logic after movement
        }

        // Periodically try to assign pending tasks to idle vehicles
        for (auto &task : m_tasks)
        {
            if (task.status == TaskStatus::PENDING)
            {
                assignTaskToAvailableVehicle(task.id);
                // assignTaskToAvailableVehicle might modify task and vehicle states
                // If a task is assigned, the loop might need to re-evaluate or break
                // For simplicity, we'll let it continue. A more robust system might re-check.
            }
        }
    }

    std::optional<MockTask> MockSimulationInterface::getTaskById(int taskId) const
    {
        auto it = std::find_if(m_tasks.begin(), m_tasks.end(),
                               [taskId](const MockTask &t)
                               { return t.id == taskId; });
        if (it != m_tasks.end())
        {
            return *it;
        }
        return std::nullopt;
    }

    void MockSimulationInterface::addTask(const MockTask &task)
    {
        // Prevent duplicate task IDs
        if (getTaskById(task.id))
        {
            std::cerr << "Warning: Attempted to add task with duplicate ID: " << task.id << std::endl;
            return;
        }
        m_tasks.push_back(task);
        std::cout << "MockSim: Added Task ID: " << task.id << " (Pickup: " << task.pickupLocationDistMm
                  << "mm, Dropoff: " << task.dropoffLocationDistMm << "mm)" << std::endl;
    }

    bool MockSimulationInterface::assignTaskToAvailableVehicle(int taskId)
    {
        MockTask *taskToAssign = nullptr;
        for (auto &t : m_tasks)
        {
            if (t.id == taskId)
            {
                taskToAssign = &t;
                break;
            }
        }

        if (!taskToAssign || taskToAssign->status != TaskStatus::PENDING)
        {
            // std::cout << "Debug: Task " << taskId << " not found or not pending." << std::endl;
            return false; // Task not found or not pending
        }

        for (MockVehicle &vehicle : m_vehicles)
        {
            if (vehicle.status == VehicleStatus::IDLE)
            {
                taskToAssign->assignedVehicleId = vehicle.id;
                taskToAssign->status = TaskStatus::ASSIGNED;

                vehicle.currentTaskId = taskToAssign->id;
                vehicle.status = VehicleStatus::MOVING_TO_PICKUP;
                vehicle.targetPositionMm = taskToAssign->pickupLocationDistMm;

                std::cout << "MockSim: Assigned Task ID " << taskToAssign->id
                          << " to Vehicle ID " << vehicle.id
                          << ". Target pickup: " << vehicle.targetPositionMm << "mm" << std::endl;

                // Update device state if pickup device is specified
                if (!taskToAssign->pickupDeviceId.empty())
                {
                    updateDeviceStatus(taskToAssign->pickupDeviceId, DeviceOperationalStatus::BUSY, 1);
                }
                return true;
            }
        }
        // std::cout << "Debug: No idle vehicle found for task " << taskId << std::endl;
        return false; // No idle vehicle found
    }

    std::optional<MockVehicle> MockSimulationInterface::getVehicleById(int vehicleId) const
    {
        auto it = std::find_if(m_vehicles.begin(), m_vehicles.end(),
                               [vehicleId](const MockVehicle &v)
                               { return v.id == vehicleId; });
        if (it != m_vehicles.end())
        {
            return *it;
        }
        return std::nullopt;
    }

    void MockSimulationInterface::addVehicle(const MockVehicle &vehicle)
    {
        if (getVehicleById(vehicle.id))
        {
            std::cerr << "Warning: Attempted to add vehicle with duplicate ID: " << vehicle.id << std::endl;
            return;
        }
        m_vehicles.push_back(vehicle);
        std::cout << "MockSim: Added Vehicle ID: " << vehicle.id << " at " << vehicle.currentPositionMm << "mm" << std::endl;
    }

    std::optional<MockDeviceState> MockSimulationInterface::getDeviceStateById(const std::string &deviceId) const
    {
        auto it = m_deviceStates.find(deviceId);
        if (it != m_deviceStates.end())
        {
            return it->second;
        }
        return std::nullopt;
    }

    void MockSimulationInterface::addDevice(const MockDeviceState &device)
    {
        if (m_deviceStates.count(device.id))
        {
            std::cerr << "Warning: Attempted to add device with duplicate ID: " << device.id << std::endl;
            return;
        }
        m_deviceStates[device.id] = device;
        std::cout << "MockSim: Added Device ID: " << device.id << std::endl;
    }

    void MockSimulationInterface::updateDeviceStatus(const std::string &deviceId, DeviceOperationalStatus newOpStatus, int taskCountChange)
    {
        auto it = m_deviceStates.find(deviceId);
        if (it != m_deviceStates.end())
        {
            it->second.operationalStatus = newOpStatus;
            it->second.currentTaskCount += taskCountChange;
            if (it->second.currentTaskCount < 0)
                it->second.currentTaskCount = 0;

            // If no tasks and was busy, set to operational (unless out of service)
            if (it->second.currentTaskCount == 0 && newOpStatus == DeviceOperationalStatus::BUSY && it->second.operationalStatus != DeviceOperationalStatus::OUT_OF_SERVICE)
            {
                it->second.operationalStatus = DeviceOperationalStatus::OPERATIONAL;
            }
            std::cout << "MockSim: Device " << deviceId << " status updated. OpStatus: "
                      << static_cast<int>(it->second.operationalStatus) << ", Tasks: " << it->second.currentTaskCount << std::endl;
        }
        else
        {
            std::cerr << "Warning: Attempted to update status for non-existent device: " << deviceId << std::endl;
        }
    }

    void MockSimulationInterface::initializeDefaultScenario()
    {
        m_tasks.clear();
        m_vehicles.clear();
        m_deviceStates.clear();
        m_vehicleActionTimers.clear();

        std::cout << "MockSim: Initializing default scenario..." << std::endl;

        // Add Devices
        addDevice(MockDeviceState("D01"));
        addDevice(MockDeviceState("D02"));
        addDevice(MockDeviceState("D03"));
        addDevice(MockDeviceState("D04"));

        // Add Vehicles
        addVehicle(MockVehicle(1, 0.0f, 1000.0f));     // Vehicle 1, speed 1 m/s
        addVehicle(MockVehicle(2, 10000.0f, 1500.0f)); // Vehicle 2, speed 1.5 m/s
        addVehicle(MockVehicle(3, 20000.0f, 1200.0f)); // Vehicle 3, speed 1.2 m/s

        // Add Tasks (Distances are examples, adjust to your track length)
        // Track total length is m_trackTotalLengthMm
        float quarterTrack = m_trackTotalLengthMm / 4.0f;
        float halfTrack = m_trackTotalLengthMm / 2.0f;
        float threeQuarterTrack = 3.0f * m_trackTotalLengthMm / 4.0f;

        addTask(MockTask(101, "Task Alpha (D01 to D02)", quarterTrack, halfTrack, "D01", "D02"));
        addTask(MockTask(102, "Task Beta (D03 to D04)", threeQuarterTrack, quarterTrack / 2.0f, "D03", "D04"));
        addTask(MockTask(103, "Task Gamma (D01 to D03)", 0, threeQuarterTrack, "D01", "D03"));
        addTask(MockTask(104, "Task Delta (Short Loop)", 1000.f, 5000.f, "D02", "D01")); // A short task

        std::cout << "MockSim: Default scenario initialized." << std::endl;
    }

    void MockSimulationInterface::updateVehicleMovement(MockVehicle &vehicle, float deltaTimeSeconds)
    {
        if (vehicle.status != VehicleStatus::MOVING_TO_PICKUP && vehicle.status != VehicleStatus::MOVING_TO_DROPOFF)
        {
            return; // Not moving towards a target
        }

        if (m_trackTotalLengthMm <= 0)
            return; // Avoid division by zero or negative lengths

        float distanceToMove = vehicle.speedMmPerSecond * deltaTimeSeconds;

        // Simplified direct movement towards target; does not consider shortest path on a loop yet.
        // This assumes targetPositionMm is always "ahead" in the current travel direction for simplicity.
        // For a full loop, one might need to calculate if going reverse is shorter.

        float pathToEndOfTrack = m_trackTotalLengthMm - vehicle.currentPositionMm;
        float pathFromStartToTarget = vehicle.targetPositionMm;

        // Calculate distance remaining to target along the track (assuming CCW movement)
        float distanceRemaining;
        if (vehicle.targetPositionMm >= vehicle.currentPositionMm)
        {
            distanceRemaining = vehicle.targetPositionMm - vehicle.currentPositionMm;
        }
        else
        { // Target is past the "0" point on the loop
            distanceRemaining = (m_trackTotalLengthMm - vehicle.currentPositionMm) + vehicle.targetPositionMm;
        }

        if (distanceToMove >= distanceRemaining)
        {
            vehicle.currentPositionMm = vehicle.targetPositionMm; // Arrived
            // Logic for arrival is handled in processVehicleLogic
        }
        else
        {
            vehicle.currentPositionMm += distanceToMove;
            // Normalize position to be within [0, m_trackTotalLengthMm)
            vehicle.currentPositionMm = std::fmod(vehicle.currentPositionMm, m_trackTotalLengthMm);
            if (vehicle.currentPositionMm < 0)
            { // fmod can return negative if dividend is negative
                vehicle.currentPositionMm += m_trackTotalLengthMm;
            }
        }
        // std::cout << "Vehicle " << vehicle.id << " pos: " << vehicle.currentPositionMm << " target: " << vehicle.targetPositionMm << std::endl;
    }

    void MockSimulationInterface::processVehicleLogic(MockVehicle &vehicle, float /*deltaTimeSeconds*/)
    {
        // Find the task associated with the vehicle, if any
        MockTask *currentTask = nullptr;
        if (vehicle.currentTaskId != -1)
        {
            for (auto &t : m_tasks)
            {
                if (t.id == vehicle.currentTaskId)
                {
                    currentTask = &t;
                    break;
                }
            }
        }

        // If vehicle has a task but it can't be found (should not happen if logic is correct)
        if (vehicle.currentTaskId != -1 && !currentTask)
        {
            std::cerr << "Error: Vehicle " << vehicle.id << " has task ID " << vehicle.currentTaskId << " but task not found!" << std::endl;
            vehicle.status = VehicleStatus::ERROR;
            vehicle.currentTaskId = -1;
            return;
        }

        switch (vehicle.status)
        {
        case VehicleStatus::IDLE:
            // Try to find a new task if any are PENDING
            for (auto &task_iter : m_tasks)
            {
                if (task_iter.status == TaskStatus::PENDING)
                {
                    if (assignTaskToAvailableVehicle(task_iter.id))
                    {
                        // Task assigned, vehicle state changed in assignTaskToAvailableVehicle
                        // No need to do more for this vehicle in this update cycle for IDLE state.
                        return; // Vehicle got a new task
                    }
                }
            }
            break;

        case VehicleStatus::MOVING_TO_PICKUP:
            if (currentTask && basically_equal(vehicle.currentPositionMm, vehicle.targetPositionMm))
            {
                vehicle.status = VehicleStatus::PICKING_UP;
                currentTask->status = TaskStatus::PICKING_UP;
                m_vehicleActionTimers[vehicle.id] = ACTION_DURATION_SECONDS;
                std::cout << "MockSim: Vehicle " << vehicle.id << " arrived for PICKUP for Task " << currentTask->id
                          << " at " << vehicle.currentPositionMm << "mm. Device: " << currentTask->pickupDeviceId << std::endl;
                // Device status was already set to BUSY by assignTask
            }
            break;

        case VehicleStatus::PICKING_UP:
            if (currentTask && m_vehicleActionTimers.find(vehicle.id) == m_vehicleActionTimers.end()) // Timer expired
            {
                vehicle.status = VehicleStatus::MOVING_TO_DROPOFF;
                currentTask->status = TaskStatus::TRANSPORTING;
                vehicle.targetPositionMm = currentTask->dropoffLocationDistMm;
                std::cout << "MockSim: Vehicle " << vehicle.id << " FINISHED PICKUP for Task " << currentTask->id
                          << ". Moving to dropoff at " << vehicle.targetPositionMm << "mm. Device: " << currentTask->pickupDeviceId << std::endl;

                // Update pickup device: task count decreases, might become OPERATIONAL
                if (!currentTask->pickupDeviceId.empty())
                {
                    updateDeviceStatus(currentTask->pickupDeviceId, DeviceOperationalStatus::OPERATIONAL, -1); // taskCountChange -1
                }
                // Update dropoff device: task count increases, becomes BUSY
                if (!currentTask->dropoffDeviceId.empty())
                {
                    updateDeviceStatus(currentTask->dropoffDeviceId, DeviceOperationalStatus::BUSY, 1); // taskCountChange +1
                }
            }
            break;

        case VehicleStatus::MOVING_TO_DROPOFF:
            if (currentTask && basically_equal(vehicle.currentPositionMm, vehicle.targetPositionMm))
            {
                vehicle.status = VehicleStatus::DROPPING_OFF;
                currentTask->status = TaskStatus::DROPPING_OFF;
                m_vehicleActionTimers[vehicle.id] = ACTION_DURATION_SECONDS;
                std::cout << "MockSim: Vehicle " << vehicle.id << " arrived for DROPOFF for Task " << currentTask->id
                          << " at " << vehicle.currentPositionMm << "mm. Device: " << currentTask->dropoffDeviceId << std::endl;
                // Device status was already set to BUSY for dropoff
            }
            break;

        case VehicleStatus::DROPPING_OFF:
            if (currentTask && m_vehicleActionTimers.find(vehicle.id) == m_vehicleActionTimers.end()) // Timer expired
            {
                vehicle.status = VehicleStatus::IDLE;
                currentTask->status = TaskStatus::COMPLETED;
                std::cout << "MockSim: Vehicle " << vehicle.id << " FINISHED DROPOFF for Task " << currentTask->id
                          << ". Status: IDLE. Device: " << currentTask->dropoffDeviceId << std::endl;

                // Update dropoff device: task count decreases, might become OPERATIONAL
                if (!currentTask->dropoffDeviceId.empty())
                {
                    updateDeviceStatus(currentTask->dropoffDeviceId, DeviceOperationalStatus::OPERATIONAL, -1);
                }

                vehicle.currentTaskId = -1; // No current task
                // Vehicle is now IDLE, will look for new tasks in the next cycle's IDLE state check or periodic assignment.
            }
            break;

        case VehicleStatus::CHARGING:
        case VehicleStatus::ERROR:
            // Placeholder for more complex logic
            break;
        }
    }

} // namespace sim