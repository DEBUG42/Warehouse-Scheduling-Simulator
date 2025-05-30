#include "gui/RealBackendAdapter.hpp"
#include "gui/SimulationInterface.hpp"
#include "../src/Core/Vehicle.hpp"
#include "../src/Core/Device.hpp"
#include "../src/Core/Scheduler.hpp"
#include <iostream>
#include <vector>

int main()
{
    std::cout << "Testing RealBackendAdapter integration..." << std::endl;

    try
    {
        // 创建 RealBackendAdapter 实例
        RealBackendAdapter adapter;

        // 测试获取仿真状态
        auto simState = adapter.getSimulationState();
        std::cout << "Simulation state - isPaused: " << simState.isPaused
                  << ", speedFactor: " << simState.simulationSpeedFactor
                  << ", vehicleCount: " << simState.vehicleCount << std::endl;

        // 测试获取车辆状态
        auto vehicles = adapter.getVehicleStates();
        std::cout << "Vehicle count: " << vehicles.size() << std::endl;

        // 测试获取设备状态
        auto devices = adapter.getDeviceStates();
        std::cout << "Device count: " << devices.size() << std::endl;

        std::cout << "Integration test passed!" << std::endl;
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Integration test failed: " << e.what() << std::endl;
        return 1;
    }
}
