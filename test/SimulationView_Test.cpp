#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#define _USE_MATH_DEFINES
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// GUI组件包含
#include "gui/SimulationView.hpp"

// Core组件包含
#include "../src/Core/Vehicle.hpp"
#include "../src/Core/Device.hpp"

/**
 * @brief SimulationView 重构测试程序
 * 
 * 基于 VehiclePathPositionTest.cpp 的验证渲染方法测试重构后的 SimulationView
 * 
 * 测试功能：
 * 1. 网格显示切换 (T键)
 * 2. 仓库显示切换 (G键)  
 * 3. 车辆显示切换 (V键)
 * 4. 车辆移动控制 (WASD键)
 * 5. 视图平移和缩放 (鼠标)
 * 6. 车辆选择功能 (鼠标点击)
 */

class SimulationViewTestApp
{
private:
    sf::RenderWindow window;
    sf::Font font;
    std::unique_ptr<SimulationView> simulationView;
    
    // 测试数据
    std::vector<std::unique_ptr<Vehicle>> vehicles;
    std::vector<std::unique_ptr<DeviceBase>> devices;
    
    // 仿真状态
    float simulationTime = 0.0f;
    bool isRunning = false;
    int selectedVehicleId = -1;

public:
    SimulationViewTestApp() 
        : window(sf::VideoMode(1280, 720), "SimulationView Refactoring Test - Based on VehiclePathPositionTest")
    {
        window.setFramerateLimit(60);
        loadFont();
        initializeTestData();
        initializeSimulationView();
    }

    bool loadFont()
    {
        // 尝试加载字体
        if (!font.loadFromFile("assets/fonts/arial.ttf"))
        {
            if (!font.loadFromFile("C:/Windows/Fonts/arial.ttf"))
            {
                std::cerr << "Warning: Cannot load font file, using default font" << std::endl;
                return false;
            }
        }
        return true;
    }

    void initializeTestData()
    {
        // 按照 VehiclePathPositionTest.cpp 的方式创建车辆
        const int vehicleCount = 4;
        for (int i = 0; i < vehicleCount; ++i)
        {
            auto vehicle = std::make_unique<Vehicle>();
            vehicle->id = i + 1;
            vehicle->position_m = (32.000 - 0.002 * i - vehicle->m_length * i); // 每辆车相距间距
            vehicle->is_loaded = false;
            vehicle->is_executing = false;
            vehicle->velocity_mps = 0.0f;
            vehicle->max_speed = 2.0f;
            vehicle->towards_device = -1;
            vehicle->next_available_time = 0.0f;
            
            // 初始化状态
            vehicle->m_state.position = vehicle->position_m;
            vehicle->m_state.currentSpeed = 0.0f;
            vehicle->m_state.motionState = Vehicle::MotionState::Stopped;
            vehicle->m_state.currentTask = nullptr;
            vehicle->m_state.operationTimer = 0.0f;
            
            vehicles.push_back(std::move(vehicle));
        }

        // 设置不同状态（与VehiclePathPositionTest.cpp一致）
        if (vehicles.size() >= 2) {
            vehicles[1]->is_loaded = false; // 车辆2：有任务无货
        }
        if (vehicles.size() >= 3) {
            vehicles[2]->is_loaded = true;  // 车辆3：有货
        }
        if (vehicles.size() >= 4) {
            vehicles[3]->is_loaded = true;  // 车辆4：有任务且有货
        }

        std::cout << "Initialized " << vehicles.size() << " vehicles with VehiclePathPositionTest settings" << std::endl;
    }

    void initializeSimulationView()
    {
        // 创建SimulationView（使用重构后的版本）
        simulationView = std::make_unique<SimulationView>(font);
        
        // 使用与VehiclePathPositionTest.cpp相同的初始化参数
        sf::Vector2f initialViewSize(window.getSize().x, window.getSize().y);
        simulationView->initialize(font, nullptr, initialViewSize);
        
        // 设置显示选项（默认设置）
        simulationView->setShowGrid(false);        // 默认关闭网格
        simulationView->setShowWarehouses(true);   // 显示仓库
        simulationView->setShowVehicles(true);     // 显示车辆
        
        // 更新车辆数据
        updateSimulationViewData();
        
        // 设置车辆选择回调
        simulationView->setVehicleSelectedCallback([this](int vehicleId) {
            selectedVehicleId = vehicleId;
            if (vehicleId > 0) {
                std::cout << "Selected vehicle ID: " << vehicleId << std::endl;
            } else {
                std::cout << "Deselected vehicle" << std::endl;
            }
        });
    }

    void updateSimulationViewData()
    {
        // 转换为指针数组
        std::vector<Vehicle*> vehiclePtrs;
        for (auto& vehicle : vehicles) {
            vehiclePtrs.push_back(vehicle.get());
        }
        simulationView->updateVehicles(vehiclePtrs);
        
        // 更新设备数据（目前为空）
        std::vector<DeviceBase*> devicePtrs;
        simulationView->updateDevices(devicePtrs);
    }

    void handleEvents()
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            // 获取鼠标位置
            sf::Vector2i mousePixelPos = sf::Mouse::getPosition(window);
            sf::Vector2f mousePos = window.mapPixelToCoords(mousePixelPos);

            // SimulationView事件处理（包括视图平移、缩放、对象选择）
            if (simulationView) {
                simulationView->handleViewEvent(event, mousePos);
            }

            // 键盘控制
            if (event.type == sf::Event::KeyPressed) {
                switch (event.key.code) {
                    case sf::Keyboard::Space:
                        isRunning = !isRunning;
                        std::cout << "Simulation " << (isRunning ? "Started" : "Paused") << std::endl;
                        break;
                        
                    case sf::Keyboard::R:
                        resetSimulation();
                        std::cout << "Simulation Reset" << std::endl;
                        break;
                        
                    case sf::Keyboard::Escape:
                        window.close();
                        break;
                        
                    // 显示控制键（与VehiclePathPositionTest.cpp一致）
                    case sf::Keyboard::T:
                        if (simulationView) {
                            bool currentState = simulationView->getShowGrid();
                            simulationView->setShowGrid(!currentState);
                            std::cout << (!currentState ? "显示坐标网格\n" : "隐藏坐标网格\n");
                        }
                        break;
                        
                    case sf::Keyboard::G:
                        if (simulationView) {
                            bool currentState = simulationView->getShowWarehouses();
                            simulationView->setShowWarehouses(!currentState);
                            std::cout << (!currentState ? "显示仓库\n" : "隐藏仓库\n");
                        }
                        break;
                        
                    case sf::Keyboard::V:
                        if (simulationView) {
                            bool currentState = simulationView->getShowVehicles();
                            simulationView->setShowVehicles(!currentState);
                            std::cout << (!currentState ? "显示车辆\n" : "隐藏车辆\n");
                        }
                        break;
                        
                    // 车辆选择和移动控制
                    case sf::Keyboard::Num1:
                    case sf::Keyboard::Num2:
                    case sf::Keyboard::Num3:
                    case sf::Keyboard::Num4:
                        {
                            int vehicleId = event.key.code - sf::Keyboard::Num0;
                            if (vehicleId > 0 && vehicleId <= vehicles.size()) {
                                selectedVehicleId = vehicleId;
                                std::cout << "Selected vehicle " << vehicleId << " via keyboard" << std::endl;
                            }
                        }
                        break;
                        
                    case sf::Keyboard::W:
                    case sf::Keyboard::A:
                    case sf::Keyboard::S:
                    case sf::Keyboard::D:
                        if (selectedVehicleId > 0 && selectedVehicleId <= vehicles.size()) {
                            moveSelectedVehicle(event.key.code);
                        }
                        break;
                }
            }
        }
    }

    void moveSelectedVehicle(sf::Keyboard::Key key)
    {
        if (selectedVehicleId <= 0 || selectedVehicleId > vehicles.size()) {
            return;
        }
        
        auto& vehicle = vehicles[selectedVehicleId - 1];
        float currentPos = vehicle->position_m;
        float newPos = currentPos;
        
        const float moveSpeed = 2.0f; // 移动速度（米）
        
        switch (key) {
            case sf::Keyboard::W:
            case sf::Keyboard::D:
                newPos += moveSpeed; // 向前移动
                break;
            case sf::Keyboard::S:
            case sf::Keyboard::A:
                newPos -= moveSpeed; // 向后移动
                break;
            default:
                return;
        }
        
        // 边界检查（保持在轨道范围内）
        float maxTrackLengthMm = 2 * 40000.0f + 2 * M_PI * 2500.0f; // ≈ 95700mm
        float maxTrackLength = maxTrackLengthMm / 1000.0f;          // 转换为米
        
        if (newPos >= 0.0f && newPos <= maxTrackLength) {
            vehicle->position_m = newPos;
            vehicle->m_state.position = newPos;
            vehicle->m_state.motionState = Vehicle::MotionState::Cruising;
            
            std::cout << "Vehicle " << selectedVehicleId << " moved to position: " << newPos << "m" << std::endl;
            
            // 更新SimulationView中的车辆数据
            updateSimulationViewData();
        }
    }

    void resetSimulation()
    {
        // 重置所有车辆位置和状态
        for (int i = 0; i < vehicles.size(); ++i) {
            auto& vehicle = vehicles[i];
            vehicle->position_m = (32.000 - 0.002 * i - vehicle->m_length * i);
            vehicle->velocity_mps = 0.0f;
            vehicle->m_state.position = vehicle->position_m;
            vehicle->m_state.motionState = Vehicle::MotionState::Stopped;
        }
        
        selectedVehicleId = -1;
        simulationTime = 0.0f;
        isRunning = false;
        
        // 更新SimulationView数据
        updateSimulationViewData();
    }

    void update(float deltaTime)
    {
        if (isRunning) {
            simulationTime += deltaTime;
            
            // 更新SimulationView
            if (simulationView) {
                simulationView->updateViewTransforms(deltaTime);
            }
        }
    }

    void render()
    {
        // 使用与VehiclePathPositionTest.cpp相同的背景色
        window.clear(sf::Color(230, 240, 230));
        
        // 渲染SimulationView（使用重构后的方法）
        if (simulationView) {
            simulationView->renderWorld(window);
        }
        
        // 渲染控制说明
        renderInstructions();
        
        window.display();
    }

    void renderInstructions()
    {
        sf::Text instructions;
        instructions.setFont(font);
        instructions.setCharacterSize(14);
        instructions.setFillColor(sf::Color::Black);
        instructions.setPosition(10, 10);
        
        std::string text = 
            "SimulationView Refactoring Test - Based on VehiclePathPositionTest.cpp\n\n"
            "Controls:\n"
            "Space: Start/Pause Simulation\n"
            "R: Reset Simulation\n"
            "T: Toggle Grid Display\n"
            "G: Toggle Warehouse Display\n"
            "V: Toggle Vehicle Display\n"
            "1/2/3/4: Select Vehicle\n"
            "WASD: Move Selected Vehicle\n"
            "Mouse: Pan/Zoom View, Click to Select\n"
            "ESC: Exit\n\n"
            "Status:\n"
            "Simulation: " + std::string(isRunning ? "Running" : "Paused") + "\n" +
            "Selected Vehicle: " + (selectedVehicleId > 0 ? std::to_string(selectedVehicleId) : "None") + "\n" +
            "Grid: " + std::string(simulationView ? (simulationView->getShowGrid() ? "On" : "Off") : "Unknown") + "\n" +
            "Warehouses: " + std::string(simulationView ? (simulationView->getShowWarehouses() ? "On" : "Off") : "Unknown") + "\n" +
            "Vehicles: " + std::string(simulationView ? (simulationView->getShowVehicles() ? "On" : "Off") : "Unknown");
            
        instructions.setString(text);
        window.draw(instructions);
    }

    void run()
    {
        sf::Clock clock;
        std::cout << "=== SimulationView Refactoring Test ===" << std::endl;
        std::cout << "Based on VehiclePathPositionTest.cpp proven rendering approach" << std::endl;
        std::cout << "Features tested:" << std::endl;
        std::cout << "1. Grid rendering (T key)" << std::endl;
        std::cout << "2. Warehouse rendering (G key)" << std::endl;
        std::cout << "3. Vehicle rendering using VehicleRenderer (V key)" << std::endl;
        std::cout << "4. Path origin marker rendering" << std::endl;
        std::cout << "5. UI overlay rendering (selection highlighting)" << std::endl;
        std::cout << "6. View transformation (mouse pan/zoom)" << std::endl;
        std::cout << "7. Object selection (mouse click)" << std::endl;
        std::cout << "\nPress keys to test different rendering features!" << std::endl;

        while (window.isOpen()) {
            float deltaTime = clock.restart().asSeconds();
            
            handleEvents();
            update(deltaTime);
            render();
        }
    }
};

int main()
{
    try {
        SimulationViewTestApp app;
        app.run();
    }
    catch (const std::exception& e) {
        std::cerr << "Test program error: " << e.what() << std::endl;
        return -1;
    }
    
    return 0;
}
