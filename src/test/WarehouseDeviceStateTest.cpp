#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <map>

#include "gui/WarehouseRenderer.hpp"
#include "gui/TrackRenderer.hpp"
#include "../src/Core/Device.hpp"

// Create a DeviceManager for testing
class TestDeviceManager
{
public:
    std::map<int, DeviceState> devices;

    DeviceState &getState(int device_id)
    {
        return devices[device_id];
    }
};

// Helper to draw text
void drawText(sf::RenderWindow &window, const std::string &str, sf::Vector2f pos, const sf::Font &font,
              unsigned int size = 15, sf::Color color = sf::Color::White)
{
    sf::Text text;
    text.setFont(font);
    text.setString(str);
    text.setCharacterSize(size);
    text.setFillColor(color);
    text.setPosition(pos);
    window.draw(text);
}

int main()
{
    sf::RenderWindow window(sf::VideoMode(1280, 720), "Warehouse Device State Test");
    window.setFramerateLimit(60);

    sf::Font font;
    if (!font.loadFromFile("assets/fonts/arial.ttf"))
    {
        if (!font.loadFromFile("../assets/fonts/arial.ttf"))
        {
            std::cerr << "Error loading font arial.ttf" << std::endl;
            return -1;
        }
    }

    // Initialize TrackRenderer
    TrackRenderer trackRenderer;
    trackRenderer.setMmToPxRatio(0.01f);
    trackRenderer.setScaleFactor(2.0f);
    trackRenderer.generateGeometry(trackRenderer.getTrackLength(), trackRenderer.getCurveRadius());

    // Initialize WarehouseRenderer
    WarehouseRenderer warehouseRenderer;
    sf::Vector2f worldOriginOffset(0.f, 0.f);
    warehouseRenderer.initialize(trackRenderer, worldOriginOffset, "assets/icons/"); // Create mock devices with different states
    TestDeviceManager deviceManager;
    // Create devices for IDs 1-6 (first 6 storage devices)
    for (int id = 1; id <= 6; ++id)
    {
        // Initialize device state in DeviceManager
        deviceManager.devices[id] = DeviceState();
    }

    // Set different states for demonstration
    // Device 1: Default (idle, no goods)
    deviceManager.devices[1].has_goods = false;
    deviceManager.devices[1].is_reserved = false;
    deviceManager.devices[1].is_transferring = false;

    // Device 2: Has goods
    deviceManager.devices[2].has_goods = true;
    deviceManager.devices[2].is_reserved = false;
    deviceManager.devices[2].is_transferring = false;

    // Device 3: Reserved
    deviceManager.devices[3].has_goods = false;
    deviceManager.devices[3].is_reserved = true;
    deviceManager.devices[3].is_transferring = false;

    // Device 4: Transferring
    deviceManager.devices[4].has_goods = false;
    deviceManager.devices[4].is_reserved = false;
    deviceManager.devices[4].is_transferring = true;

    // Device 5: Has goods and reserved
    deviceManager.devices[5].has_goods = true;
    deviceManager.devices[5].is_reserved = true;
    deviceManager.devices[5].is_transferring = false;

    // Device 6: Has goods and transferring
    deviceManager.devices[6].has_goods = true;
    deviceManager.devices[6].is_reserved = false;
    deviceManager.devices[6].is_transferring = true;

    // Set up view
    sf::View view = window.getDefaultView();
    view.setCenter(0, 0);
    view.zoom(2.0f);
    window.setView(view);

    bool showInstructions = true;
    int currentTestDevice = 1;

    std::cout << "Warehouse Device State Test Started" << std::endl;
    std::cout << "Device States:" << std::endl;
    std::cout << "Device 1: Idle (Green)" << std::endl;
    std::cout << "Device 2: Has Goods (Blue)" << std::endl;
    std::cout << "Device 3: Reserved (Orange)" << std::endl;
    std::cout << "Device 4: Transferring (Yellow)" << std::endl;
    std::cout << "Device 5: Has Goods + Reserved (Blue/Orange)" << std::endl;
    std::cout << "Device 6: Has Goods + Transferring (Blue/Yellow)" << std::endl;
    std::cout << "\nControls:" << std::endl;
    std::cout << "- 1-6: Focus on specific device" << std::endl;
    std::cout << "- SPACE: Cycle through device states" << std::endl;
    std::cout << "- H: Toggle help display" << std::endl;
    std::cout << "- ESC: Exit" << std::endl;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::KeyPressed)
            {
                switch (event.key.code)
                {
                case sf::Keyboard::Escape:
                    window.close();
                    break;

                case sf::Keyboard::H:
                    showInstructions = !showInstructions;
                    break;

                case sf::Keyboard::Num1:
                    currentTestDevice = 1;
                    std::cout << "Focused on Device 1 (Idle)" << std::endl;
                    break;

                case sf::Keyboard::Num2:
                    currentTestDevice = 2;
                    std::cout << "Focused on Device 2 (Has Goods)" << std::endl;
                    break;

                case sf::Keyboard::Num3:
                    currentTestDevice = 3;
                    std::cout << "Focused on Device 3 (Reserved)" << std::endl;
                    break;

                case sf::Keyboard::Num4:
                    currentTestDevice = 4;
                    std::cout << "Focused on Device 4 (Transferring)" << std::endl;
                    break;

                case sf::Keyboard::Num5:
                    currentTestDevice = 5;
                    std::cout << "Focused on Device 5 (Has Goods + Reserved)" << std::endl;
                    break;

                case sf::Keyboard::Num6:
                    currentTestDevice = 6;
                    std::cout << "Focused on Device 6 (Has Goods + Transferring)" << std::endl;
                    break;

                case sf::Keyboard::Space:
                    // Cycle through states for current device
                    {
                        auto &state = deviceManager.devices[currentTestDevice];
                        if (!state.has_goods && !state.is_reserved && !state.is_transferring)
                        {
                            state.has_goods = true;
                            std::cout << "Device " << currentTestDevice << " now has goods" << std::endl;
                        }
                        else if (state.has_goods && !state.is_reserved && !state.is_transferring)
                        {
                            state.has_goods = false;
                            state.is_reserved = true;
                            std::cout << "Device " << currentTestDevice << " now reserved" << std::endl;
                        }
                        else if (!state.has_goods && state.is_reserved && !state.is_transferring)
                        {
                            state.is_reserved = false;
                            state.is_transferring = true;
                            std::cout << "Device " << currentTestDevice << " now transferring" << std::endl;
                        }
                        else
                        {
                            state.has_goods = false;
                            state.is_reserved = false;
                            state.is_transferring = false;
                            std::cout << "Device " << currentTestDevice << " now idle" << std::endl;
                        }
                    }
                    break;

                default:
                    break;
                }
            }
        }

        // Clear window
        window.clear(sf::Color(240, 240, 240));

        // Draw track
        window.draw(trackRenderer);
        // Update warehouse renderer with current device states
        warehouseRenderer.updateDeviceStates(deviceManager.devices);

        // Draw warehouse devices
        window.draw(warehouseRenderer);

        // Draw instructions if enabled
        if (showInstructions)
        {
            std::vector<std::string> instructions = {
                "Warehouse Device State Test",
                "",
                "Device States (expected colors):",
                "Device 1: Idle -> Green",
                "Device 2: Has Goods -> Blue",
                "Device 3: Reserved -> Orange",
                "Device 4: Transferring -> Yellow",
                "Device 5: Has Goods + Reserved -> Blue/Orange",
                "Device 6: Has Goods + Transferring -> Blue/Yellow",
                "",
                "Controls:",
                "1-6: Focus on device | SPACE: Cycle states",
                "H: Toggle help | ESC: Exit",
                "",
                "Current Focus: Device " + std::to_string(currentTestDevice)};

            float yPos = 20;
            for (const auto &line : instructions)
            {
                sf::Vector2f screenPos = window.mapPixelToCoords(sf::Vector2i(20, static_cast<int>(yPos)));
                drawText(window, line, screenPos, font, 14, sf::Color::Black);
                yPos += 20;
            }
        }

        window.display();
    }
    return 0;
}
