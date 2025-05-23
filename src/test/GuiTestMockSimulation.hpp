#include "core/Task.hpp" // 确保包含Task定义

namespace test
{

    class GuiTestMockSimulation : public SimulationInterface
    {
    public:
        GuiTestMockSimulation(float trackLengthMm = 100000.0f);
        ~GuiTestMockSimulation() override = default;

        void initialize(const std::string &configPath = "") override;
        void initializeDefaultScenario(); // 特定于mock的初始化
        void update(sf::Time deltaTime) override;
        void pauseSimulation() override;
        void resumeSimulation() override;
        void setSimulationSpeedFactor(float factor) override;
        SimulationState getSimulationState() const override;
        std::vector<Core::Task> getAllTasks() const override;

        void registerStateUpdateCallback(std::function<void(const SimulationState &)> cb) override;
        void registerVehicleUpdateCallback(std::function<void(const std::vector<gui::VehicleState> &)> cb) override;
        void registerDeviceUpdateCallback(std::function<void(const std::vector<gui::DeviceState> &)> cb) override;

        // 允许外部访问内部mock数据进行更细致的场景设置 (可选)
        // sim::MockSimulationInterface& getInternalMock() { return m_mockSim; }

    private:
        void generateRandomTasks(int count);
        void updateVehiclesInternal(sf::Time deltaTime);
        void updateTasksInternal();

        SimulationState m_currentState;
        std::function<void(const SimulationState &)> m_stateUpdateCb;
        std::function<void(const std::vector<gui::VehicleState> &)> m_vehicleUpdateCb;
        std::function<void(const std::vector<gui::DeviceState> &)> m_deviceUpdateCb;

        // Mock data
        float m_trackLengthMm; // 轨道长度，单位毫米
        std::vector<gui::VehicleState> m_mockVehicles;
        std::vector<gui::DeviceState> m_mockDevices;
        std::vector<Core::Task> m_mockTasks;    // 存储所有任务，包括待处理和已完成
        std::vector<Core::Task> m_pendingTasks; // 仅待处理任务，用于getAllTasks

        sf::Time m_elapsedTimeSinceLastVehicleUpdate = sf::Time::Zero;
        sf::Time m_vehicleUpdateInterval = sf::seconds(0.1f);
    };

} // namespace test

#endif // GUITESTMOCKSIMULATION_HPP