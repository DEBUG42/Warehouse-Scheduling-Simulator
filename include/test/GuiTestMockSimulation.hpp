#ifndef GUI_TEST_MOCK_SIMULATION_HPP
#define GUI_TEST_MOCK_SIMULATION_HPP

#include "gui/SimulationInterface.hpp"
#include "gui/MockSimulationInterface.hpp" // 包含现有的 sim::MockSimulationInterface
#include <memory>
#include <vector>
#include <string>
#include <map>
#include <optional>

// 前向声明 gui::VehicleState 和 gui::DeviceState，如果它们定义在 SimObject.hpp 和 DeviceState.hpp 中
// 确保这些文件被正确包含
#include "gui/SimObject.hpp"   // 假设 gui::VehicleState 在此定义或通过此文件可访问
#include "gui/DeviceState.hpp" // 假设 gui::DeviceState 在此定义

namespace test
{

    class GuiTestMockSimulation : public SimulationInterface
    {
    public:
        GuiTestMockSimulation(float trackTotalLengthMm = 100000.0f); // 默认轨道长度100m
        ~GuiTestMockSimulation() override = default;

        // 更新模拟状态，内部会调用 m_mockSim.update()
        void update(sf::Time deltaTime);

        // --- 实现 SimulationInterface 的纯虚函数 ---
        SimulationState getSimulationState() const override;
        std::vector<gui::VehicleState> getVehicleStates() const override;
        std::vector<gui::DeviceState> getDeviceStates() const override;
        void setSimulationSpeedFactor(float speedFactor) override;
        void pauseSimulation() override;
        void resumeSimulation() override;
        void resetSimulation() override;
        void registerStateUpdateCallback(StateUpdateCallback callback) override;
        void registerVehicleUpdateCallback(VehicleUpdateCallback callback) override;
        void registerDeviceUpdateCallback(DeviceUpdateCallback callback) override;
        gui::VehicleState getVehicleState(int vehicleId) const override;
        gui::DeviceState getDeviceState(int deviceId) const override; // 注意：SimulationInterface 用 int deviceId, MockDeviceState 用 string

        // Implementations for Core::Task related methods from SimulationInterface
        std::optional<Core::Task> getTaskDetails(const std::string &taskId) const;
        std::vector<Core::Task> getAllTasks() const;
        void createTask(const Core::Task &task);
        void cancelTask(const std::string &taskId);

        // Implementations for other vehicle control methods from SimulationInterface
        void setVehicleRoute(int vehicleId, const std::vector<int> &routeNodeIds);
        std::string getVehicleCurrentLocationNode(int vehicleId) const;
        void sendVehicleToCharge(int vehicleId);

        // --- Mock 控制 ---
        sim::MockSimulationInterface &getInternalMock() { return m_mockSim; }
        void initializeDefaultScenario() { m_mockSim.initializeDefaultScenario(); }

    private:
        sim::MockSimulationInterface m_mockSim;
        SimulationState m_currentState;
        float m_timeAccumulator = 0.0f; // 用于累积时间以模拟 sf::Time 到 float 的转换

        StateUpdateCallback m_stateCallback;
        VehicleUpdateCallback m_vehicleCallback;
        DeviceUpdateCallback m_deviceCallback;

        // 辅助函数，用于将 sim::MockTask/Vehicle/DeviceState 转换为 gui::*State
        gui::VehicleState convertToGuiVehicleState(const sim::MockVehicle &mockVehicle) const;
        gui::DeviceState convertToGuiDeviceState(const sim::MockDeviceState &mockDevice) const;
        // 注意：sim::MockTask 目前没有直接对应的 gui::TaskState，但如果需要可以添加
    };

} // namespace test

#endif // GUI_TEST_MOCK_SIMULATION_HPP