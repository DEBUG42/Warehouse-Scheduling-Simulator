#ifndef REAL_BACKEND_ADAPTER_HPP
#define REAL_BACKEND_ADAPTER_HPP

#include "gui/SimulationInterface.hpp"
#include "Core/Vehicle.hpp"
#include "Core/Device.hpp"
#include "Core/Scheduler.hpp"
#include "Core/Task.hpp"
// Potentially include EventQueue.hpp and Logger.hpp if direct interaction is needed
// #include "Core/EventQueue.hpp"
// #include "Core/Logger.hpp"

#include <vector>
#include <functional>
#include <memory> // For std::unique_ptr or std::shared_ptr

// Forward declarations if full includes are too heavy or cause circular dependencies
// class VehicleManager; // Assuming VehicleManager is part of Core/Vehicle.hpp or similar
// class DeviceManager;  // Assuming DeviceManager is part of Core/Device.hpp or similar

class RealBackendAdapter : public SimulationInterface
{
public:
    // Constructor: Initializes and potentially owns the backend components
    RealBackendAdapter();
    // Alternative constructor: Takes existing backend components (e.g., for testing or if managed externally)
    // RealBackendAdapter(VehicleManager& vm, DeviceManager& dm, Scheduler& scheduler);

    virtual ~RealBackendAdapter() override;

    // --- Implementation of SimulationInterface ---

    SimulationState getSimulationState() const override;
    std::vector<Core::Vehicle *> getVehicleStates() const override;
    std::vector<Core::DeviceBase *> getDeviceStates() const override;

    void setSimulationSpeedFactor(float speedFactor) override;
    void pauseSimulation() override;
    void resumeSimulation() override;
    void resetSimulation() override;

    void registerStateUpdateCallback(StateUpdateCallback callback) override;
    void registerVehicleUpdateCallback(VehicleUpdateCallback callback) override;
    void registerDeviceUpdateCallback(DeviceUpdateCallback callback) override;

    Core::Vehicle *getVehicleStateById(int vehicleId) const override;
    Core::DeviceBase *getDeviceStateById(int deviceId) const override;

    // --- RealBackendAdapter specific methods ---
    // Method to periodically update the backend and trigger callbacks
    void updateBackend(float deltaTime);

    // Method to initialize the backend (e.g., load scenario, create vehicles/devices)
    void initializeBackend(/* parameters for scenario setup */);

private:
    // Backend components - consider ownership (e.g., std::unique_ptr if owned by adapter)
    // For now, assuming direct instantiation or references to externally managed objects.
    // If VehicleManager and DeviceManager are separate classes in Core:
    // VehicleManager m_vehicleManager;
    // DeviceManager m_deviceManager;
    // Scheduler m_scheduler;
    // EventQueue m_eventQueue; // If used

    // If Vehicle and Device management is within Scheduler or other combined classes:
    Scheduler m_scheduler; // Assuming Scheduler might manage vehicles and devices or provide access.
                           // This needs to align with actual Core architecture.
                           // For now, let's assume Scheduler is the main entry point to the backend logic.

    SimulationState m_currentSimState;

    // Callbacks
    StateUpdateCallback m_stateUpdateCallback;
    VehicleUpdateCallback m_vehicleUpdateCallback;
    DeviceUpdateCallback m_deviceUpdateCallback;

    // Helper methods to interact with the actual backend
    void fetchSimStateFromBackend();
    void fetchVehicleStatesFromBackend();
    void fetchDeviceStatesFromBackend();

    // Temporary storage for exposing pointers to Core objects
    // The GUI will receive pointers to these objects.
    // The RealBackendAdapter needs to ensure these pointers remain valid
    // for the duration they are used by the GUI in a given frame.
    // This might involve copying data if the backend objects are frequently reallocated,
    // or ensuring the backend provides stable pointers/references.
    // For now, assuming VehicleManager and DeviceManager (if they exist) return stable collections.
    std::vector<Core::Vehicle> m_cachedVehicles;   // If backend returns by value and we need to provide pointers
    std::vector<Core::DeviceBase> m_cachedDevices; // If backend returns by value

    // Pointers to be returned by getVehicleStates() and getDeviceStates()
    // These will point to elements within m_cachedVehicles and m_cachedDevices,
    // or directly to objects managed by the backend if their lifetime is suitable.
    std::vector<Core::Vehicle *> m_vehiclePtrs;
    std::vector<Core::DeviceBase *> m_devicePtrs;

    // TODO: Determine how to get vehicle and device data.
    // Does Scheduler own VehicleManager and DeviceManager?
    // Or are they separate and need to be passed in/created?
    // For now, let's assume Scheduler provides access or we have direct members.
    // If Vehicle.hpp contains VehicleManager and Device.hpp contains DeviceManager:
    // VehicleManager m_vehicleManager; // Placeholder
    // DeviceManager m_deviceManager;   // Placeholder
};

#endif // REAL_BACKEND_ADAPTER_HPP
