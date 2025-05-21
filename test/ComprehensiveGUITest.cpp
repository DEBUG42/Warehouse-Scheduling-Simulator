#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <sstream>
#include <iomanip>

// GUI component header files
#include "gui/UIControls.hpp"
#include "gui/Toolbar.hpp"
#include "gui/StatusPanel.hpp"
#include "gui/TaskListView.hpp"
#include "gui/ObjectInspector.hpp"
#include "gui/VehicleRenderer.hpp"

// Data structure header files
#include "gui/SimObject.hpp"
#include "gui/DeviceState.hpp"
#include "Core/Task.hpp"

// Define test window size
const unsigned int WINDOW_WIDTH = 1280;
const unsigned int WINDOW_HEIGHT = 720;

// Reference TrackRendererGUITest.cpp for successful font path
const std::string FONT_PATH = "../resources/fonts/msyh.ttc"; // Assume this is the path used in TrackRendererGUITest

// Helper function: draw a simple device shape
void drawSimpleDevice(sf::RenderTarget &target, const gui::DeviceState &device, sf::Font &font, const sf::Color &color)
{
    float deviceWidthPx = 60.f;
    float deviceHeightPx = 40.f;
    sf::RectangleShape shape(sf::Vector2f(deviceWidthPx, deviceHeightPx));
    shape.setOrigin(deviceWidthPx / 2.f, deviceHeightPx / 2.f);
    shape.setPosition(device.getPosition());
    shape.setFillColor(color);
    shape.setOutlineThickness(2.f);
    shape.setOutlineColor(sf::Color(color.r / 2, color.g / 2, color.b / 2));
    target.draw(shape);

    // Draw device ID
    sf::Text idText(device.getId(), font, 10);
    sf::FloatRect textBounds = idText.getLocalBounds();
    idText.setOrigin(textBounds.left + textBounds.width / 2.0f, textBounds.top + textBounds.height / 2.0f);
    idText.setPosition(device.getPosition());
    idText.setFillColor(sf::Color::White);
    target.draw(idText);
}

int main()
{
    // 1. Initialize SFML window
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Comprehensive GUI Test");
    window.setFramerateLimit(60);

    // 2. Load font
    sf::Font font;
    std::string fontPath = "../assets/fonts/arial.ttf";
    std::cout << "Trying to load font: " << fontPath << std::endl;
    if (!font.loadFromFile(fontPath))
    {
        std::cerr << "Failed to load font file: " << fontPath << std::endl;
        return -1;
    }
    else
    {
        std::cout << "Successfully loaded font: " << fontPath << std::endl;
    }

    // 3. Instantiate GUI components
    float toolbarHeight = 30.f;
    Toolbar toolbar(font, toolbarHeight, static_cast<float>(WINDOW_WIDTH));

    StatusPanel statusPanel(font);
    statusPanel.resize(static_cast<float>(WINDOW_HEIGHT) - toolbarHeight);

    VehicleRenderer vehicleRenderer(font);

    // 4. Define static data
    // --- Vehicles (3) ---
    std::vector<gui::VehicleState> vehicleData;
    gui::CargoDisplayInfo cargo1_details;
    cargo1_details.materialId = 101;
    cargo1_details.quantity = 10;

    // gui::VehicleState: id, render_pos, raw_track_pos_mm, speed, status, taskId, isLoaded, cargo, battery
    vehicleData.push_back(gui::VehicleState{
        "V_001", sf::Vector2f(0.f, 0.f),
        0.0f,
        10.0f, gui::VehicleStatus::IDLE, "", false,
        gui::CargoDisplayInfo{}, 0.85f});
    vehicleData.push_back(gui::VehicleState{
        "V_002", sf::Vector2f(0.f, 0.f),
        20000.0f,
        12.5f, gui::VehicleStatus::MOVING_TO_LOAD, "T_101", false,
        gui::CargoDisplayInfo{}, 0.95f});
    vehicleData.push_back(gui::VehicleState{
        "V_003", sf::Vector2f(0.f, 0.f),
        40000.0f,
        0.0f, gui::VehicleStatus::ERROR, "T_102", true,
        cargo1_details, 0.30f});

    // --- Devices (3) ---
    std::vector<gui::DeviceState> deviceData;
    deviceData.push_back(gui::DeviceState{
        "W_A01", sf::Vector2f(100.f, 100.f),
        gui::DeviceType::CORE_WORKSTATION_OUT, gui::DeviceStatus::IDLE, ""});
    deviceData.push_back(gui::DeviceState{
        "C_001", sf::Vector2f(600.f, 100.f),
        gui::DeviceType::CHARGER, gui::DeviceStatus::CHARGING_VEHICLE, "V_002"});
    deviceData.push_back(gui::DeviceState{
        "W_B01", sf::Vector2f(600.f, 500.f),
        gui::DeviceType::CORE_WORKSTATION_IN, gui::DeviceStatus::ERROR, ""});

    // --- Tasks ---
    std::vector<Core::Task> taskData;
    taskData.push_back(Core::Task{"T_101", Core::TaskType::INPUT, 1, 1, 2});
    taskData.push_back(Core::Task{"T_102", Core::TaskType::OUTPUT, 2, 3, 4});
    taskData.push_back(Core::Task{"T_103", Core::TaskType::INPUT, 3, 1, 5});

    // --- Selected object ---
    const gui::SimObject *selectedObject = &vehicleData[1];

    // 5. GUI component initial setup and callbacks
    toolbar.setPlayPauseCallback([&]()
                                 {
        static bool playing = false; playing = !playing;
        std::cout << (playing ? "Simulation Running" : "Simulation Paused") << std::endl;
        toolbar.updatePlayPauseState(playing); });
    toolbar.setTimeScaleCallback([](float scale)
                                 { std::cout << "Time scale: " << scale << "x" << std::endl; });
    toolbar.setAddTaskCallback([]()
                               { std::cout << "Add Task button clicked" << std::endl; });
    toolbar.setResetViewCallback([]()
                                 { std::cout << "Reset View button clicked" << std::endl; });
    toolbar.setSwitchModeCallback([]()
                                  { std::cout << "Switch Mode button clicked" << std::endl; });

    toolbar.updatePlayPauseState(false);
    toolbar.updateTimeScale(1.0f);

    // StatusPanel initial update
    statusPanel.setSimulationTime(1234.5f);
    statusPanel.setVehicleCount(vehicleData.size());
    statusPanel.setCompletedTaskCount(5);
    statusPanel.setPendingTaskCount(taskData.size());
    statusPanel.refreshContent(selectedObject, taskData);

    // 6. Main loop
    sf::Clock clock;
    float simTime = 1234.5f;

    const float TRACK_TOTAL_STRAIGHT_MM = 80000.0f;
    const float TRACK_CURVE_RADIUS_MM = 15000.0f;

    while (window.isOpen())
    {
        sf::Time elapsed = clock.restart();
        simTime += elapsed.asSeconds() * toolbar.getTimeScaleValue();

        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            sf::Vector2f mousePos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window));
            toolbar.handleEvent(event, mousePos);

            float panelWidth = statusPanel.getPanelWidth();
            sf::Vector2f statusPanelPos(WINDOW_WIDTH - panelWidth, toolbarHeight);
            if (mousePos.x >= statusPanelPos.x && mousePos.y >= statusPanelPos.y)
            {
                sf::Vector2f localMousePos = mousePos - statusPanelPos;
                statusPanel.handleEvent(event, localMousePos);
            }

            // (Optional) Switch selected object on click
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
            {
                bool found = false;
                for (size_t i = 0; i < vehicleData.size(); ++i)
                {
                    sf::FloatRect clickableArea(vehicleData[i].position.x - 20, vehicleData[i].position.y - 10, 40, 20);
                    if (clickableArea.contains(mousePos))
                    {
                        selectedObject = &vehicleData[i];
                        statusPanel.refreshContent(selectedObject, taskData);
                        found = true;
                        break;
                    }
                }
                if (!found)
                {
                    for (size_t i = 0; i < deviceData.size(); ++i)
                    {
                        sf::FloatRect clickableArea(deviceData[i].position.x - 30, deviceData[i].position.y - 20, 60, 40);
                        if (clickableArea.contains(mousePos))
                        {
                            selectedObject = &deviceData[i];
                            statusPanel.refreshContent(selectedObject, taskData);
                            found = true;
                            break;
                        }
                    }
                }
            }
        }

        toolbar.updateTimeDisplay(simTime, elapsed.asSeconds());
        statusPanel.setSimulationTime(simTime);

        window.clear(sf::Color(70, 70, 80));

        // Draw vehicles
        for (const auto &vehicle : vehicleData)
        {
            vehicleRenderer.updateState(vehicle, TRACK_TOTAL_STRAIGHT_MM, TRACK_CURVE_RADIUS_MM);
            window.draw(vehicleRenderer);
        }
        // Draw devices
        for (const auto &device : deviceData)
        {
            sf::Color devColor = sf::Color::Cyan;
            if (device.deviceType == gui::DeviceType::CHARGER)
                devColor = sf::Color::Yellow;
            if (device.status == gui::DeviceStatus::ERROR)
                devColor = sf::Color(255, 100, 100);
            drawSimpleDevice(window, device, font, devColor);
        }

        toolbar.render(window, sf::Vector2f(0.f, 0.f));
        statusPanel.render(window, sf::Vector2f(WINDOW_WIDTH - statusPanel.getPanelWidth(), toolbarHeight));

        window.display();
    }

    return 0;
}