#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>
#include <vector>
#include <memory>
#include <string>

// 定义M_PI（如果cmath没有提供）
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// This is a simple test program to verify our GUI component functionality
// Does not depend on Core module, uses mock data for display

// Mock vehicle state
struct VehicleState
{
    int id;
    float position; // Track position (0-1)
    float speed;    // Current speed
    bool isLoaded;  // Loading status
};

// Mock device state
enum class DeviceType
{
    StorageIn,     // Storage input interface
    StorageOut,    // Storage output interface
    WorkstationIn, // Workstation input
    WorkstationOut // Workstation output
};

enum class DeviceStatus
{
    idle,    // Idle
    working, // Working
};

struct DeviceState
{
    int id;
    DeviceType type;
    DeviceStatus status;
    int queuedTaskCount;
};

// Mock task
enum class TaskType
{
    input,
    output
};

struct Task
{
    int taskId;
    TaskType type;
    int materialId;
    int startDeviceId;
    int endDeviceId;
    sf::Time createTime;
    sf::Time startTime;
    sf::Time completeTime;
    int assignedVehicleId;
};

// Track renderer
class TrackRenderer
{
private:
    sf::VertexArray m_trackLines;
    sf::Color m_trackColor;
    float m_radius;

public:
    TrackRenderer() : m_trackLines(sf::LinesStrip), m_trackColor(sf::Color(180, 180, 180)), m_radius(100.0f)
    {
        generateTrack();
    }

    void generateTrack()
    {
        // Clear existing vertices
        m_trackLines.clear();

        // Create an elliptical track
        const int segments = 60; // Number of segments
        for (int i = 0; i <= segments; ++i)
        {
            float angle = 2.0f * M_PI * i / segments;
            float x = m_radius * std::cos(angle);
            float y = m_radius * std::sin(angle) * 0.7f; // Elliptical shape

            m_trackLines.append(sf::Vertex(sf::Vector2f(x, y), m_trackColor));
        }
    }

    void draw(sf::RenderTarget &target, sf::RenderStates states) const
    {
        target.draw(m_trackLines, states);
    }
};

// Vehicle renderer
class VehicleRenderer
{
private:
    sf::Color m_emptyColor;
    sf::Color m_loadedColor;
    float m_vehicleSize;

public:
    VehicleRenderer() : m_emptyColor(sf::Color(80, 130, 200)),
                        m_loadedColor(sf::Color(200, 90, 40)),
                        m_vehicleSize(10.0f)
    {
    }

    void draw(sf::RenderTarget &target, const VehicleState &vehicle, float radius)
    {
        float angle = vehicle.position * 2.0f * M_PI;
        float x = radius * std::cos(angle);
        float y = radius * std::sin(angle) * 0.7f;

        // Create vehicle shape
        sf::RectangleShape vehicleShape(sf::Vector2f(m_vehicleSize * 1.5f, m_vehicleSize));
        vehicleShape.setOrigin(m_vehicleSize * 0.75f, m_vehicleSize * 0.5f);
        vehicleShape.setPosition(x, y);
        vehicleShape.setRotation(angle * 180.0f / M_PI + 90.0f);

        // Set color based on load status
        vehicleShape.setFillColor(vehicle.isLoaded ? m_loadedColor : m_emptyColor);

        // Draw vehicle
        target.draw(vehicleShape);

        // Draw ID label
        sf::Font font;
        if (font.loadFromFile("resources/fonts/arial.ttf"))
        {
            sf::Text idText;
            idText.setFont(font);
            idText.setString(std::to_string(vehicle.id));
            idText.setCharacterSize(12);
            idText.setFillColor(sf::Color::White);

            sf::FloatRect textBounds = idText.getLocalBounds();
            idText.setOrigin(textBounds.width / 2, textBounds.height / 2);
            idText.setPosition(x, y - 15);

            target.draw(idText);
        }
    }
};

// Device renderer
class DeviceRenderer
{
private:
    std::map<DeviceType, sf::Color> m_deviceColors;
    float m_deviceSize;

public:
    DeviceRenderer() : m_deviceSize(15.0f)
    {
        // Set different device type colors
        m_deviceColors[DeviceType::StorageIn] = sf::Color(75, 185, 85);       // Green
        m_deviceColors[DeviceType::StorageOut] = sf::Color(215, 60, 60);      // Red
        m_deviceColors[DeviceType::WorkstationIn] = sf::Color(80, 130, 200);  // Blue
        m_deviceColors[DeviceType::WorkstationOut] = sf::Color(230, 170, 50); // Yellow
    }

    void draw(sf::RenderTarget &target, const DeviceState &device, const sf::Vector2f &position)
    {
        // Create device shape
        sf::CircleShape deviceShape(m_deviceSize);
        deviceShape.setOrigin(m_deviceSize, m_deviceSize);
        deviceShape.setPosition(position);

        // Set device color
        if (m_deviceColors.find(device.type) != m_deviceColors.end())
        {
            deviceShape.setFillColor(m_deviceColors[device.type]);
        }
        else
        {
            deviceShape.setFillColor(sf::Color(150, 150, 150)); // Default gray
        }

        // Add blinking effect if device is working
        if (device.status == DeviceStatus::working)
        {
            static float time = 0.0f;
            time += 0.05f;
            float alpha = 128 + 127 * std::sin(time);
            deviceShape.setFillColor(sf::Color(
                deviceShape.getFillColor().r,
                deviceShape.getFillColor().g,
                deviceShape.getFillColor().b,
                static_cast<sf::Uint8>(alpha)));
        }

        target.draw(deviceShape);

        // Draw ID label
        sf::Font font;
        if (font.loadFromFile("resources/fonts/arial.ttf"))
        {
            sf::Text idText;
            idText.setFont(font);
            idText.setString(std::to_string(device.id));
            idText.setCharacterSize(12);
            idText.setFillColor(sf::Color::White);

            sf::FloatRect textBounds = idText.getLocalBounds();
            idText.setOrigin(textBounds.width / 2, textBounds.height / 2);
            idText.setPosition(position);

            target.draw(idText);
        }
    }
};

// Simulation data generator class
class SimulationDataGenerator
{
public:
    static std::vector<Task> generateSampleTasks()
    {
        std::vector<Task> tasks;

        // Add some sample tasks
        for (int i = 0; i < 8; ++i)
        {
            Task task;
            task.taskId = 100 + i;
            task.type = (i % 2 == 0) ? TaskType::input : TaskType::output;
            task.materialId = 1000 + i * 10;
            task.startDeviceId = 1 + i % 6;
            task.endDeviceId = 7 + i % 6;
            task.createTime = sf::seconds(i * 60.0f);

            // Randomly set some task assignments
            if (i < 3)
            {
                task.assignedVehicleId = i;
                task.startTime = sf::seconds((i + 1) * 30.0f);
            }
            else
            {
                task.assignedVehicleId = -1;
            }

            tasks.push_back(task);
        }

        return tasks;
    }

    static std::map<int, DeviceState> generateSampleDevices()
    {
        std::map<int, DeviceState> devices;

        // Create storage input devices
        for (int i = 1; i <= 6; ++i)
        {
            DeviceState device;
            device.id = i;
            device.type = DeviceType::StorageIn;
            device.status = (i % 3 == 0) ? DeviceStatus::working : DeviceStatus::idle;
            device.queuedTaskCount = i % 2;
            devices[i] = device;
        }

        // Create storage output devices
        for (int i = 7; i <= 12; ++i)
        {
            DeviceState device;
            device.id = i;
            device.type = DeviceType::StorageOut;
            device.status = (i % 4 == 0) ? DeviceStatus::working : DeviceStatus::idle;
            device.queuedTaskCount = (i + 1) % 3;
            devices[i] = device;
        }

        // Create workstation output devices
        for (int i = 13; i <= 15; ++i)
        {
            DeviceState device;
            device.id = i;
            device.type = DeviceType::WorkstationOut;
            device.status = (i % 2 == 0) ? DeviceStatus::working : DeviceStatus::idle;
            device.queuedTaskCount = i % 2;
            devices[i] = device;
        }

        // Create workstation input devices
        for (int i = 16; i <= 18; ++i)
        {
            DeviceState device;
            device.id = i;
            device.type = DeviceType::WorkstationIn;
            device.status = (i % 3 == 0) ? DeviceStatus::working : DeviceStatus::idle;
            device.queuedTaskCount = (i - 15) % 2;
            devices[i] = device;
        }

        return devices;
    }
    static std::vector<VehicleState> generateSampleVehicles(int count)
    {
        std::vector<VehicleState> vehicles;

        for (int i = 0; i < count; ++i)
        {
            VehicleState vehicle;
            vehicle.id = i;
            vehicle.position = (float)i / count;
            vehicle.speed = 0.2f + i * 0.1f;
            vehicle.isLoaded = (i % 3 == 0);

            vehicles.push_back(vehicle);
        }

        return vehicles;
    }
};

// Simplified simulation data generator (previous MockSimulationEngine class removed)

// Control panel
class ControlPanel
{
private:
    sf::RectangleShape m_background;
    sf::Font m_font;
    sf::Text m_title;
    sf::Text m_status;
    std::vector<sf::Text> m_infoTexts;

    // Button
    sf::RectangleShape m_playButton;
    sf::Text m_playButtonText;
    bool m_isPlaying;

    // Slider
    sf::RectangleShape m_speedSlider;
    sf::CircleShape m_sliderHandle;
    sf::Text m_speedText;
    float m_simulationSpeed;
    bool m_isDragging;

public:
    ControlPanel() : m_isPlaying(true), m_simulationSpeed(1.0f), m_isDragging(false)
    {
        // Load font
        if (!m_font.loadFromFile("resources/fonts/arial.ttf"))
        {
            std::cerr << "Failed to load font!" << std::endl;
        }

        // Initialize background
        m_background.setSize(sf::Vector2f(200, 400));
        m_background.setFillColor(sf::Color(50, 50, 50, 200));
        m_background.setPosition(10, 10);

        // Initialize title
        m_title.setFont(m_font);
        m_title.setString("Simulation Control");
        m_title.setCharacterSize(18);
        m_title.setFillColor(sf::Color::White);
        m_title.setPosition(20, 20);

        // Initialize status text
        m_status.setFont(m_font);
        m_status.setString("Status: Running");
        m_status.setCharacterSize(14);
        m_status.setFillColor(sf::Color::Green);
        m_status.setPosition(20, 50);

        // Initialize play button
        m_playButton.setSize(sf::Vector2f(80, 30));
        m_playButton.setFillColor(sf::Color(70, 70, 70));
        m_playButton.setPosition(20, 80);

        m_playButtonText.setFont(m_font);
        m_playButtonText.setString("Pause");
        m_playButtonText.setCharacterSize(14);
        m_playButtonText.setFillColor(sf::Color::White);
        sf::FloatRect textBounds = m_playButtonText.getLocalBounds();
        m_playButtonText.setPosition(
            m_playButton.getPosition().x + (m_playButton.getSize().x - textBounds.width) / 2,
            m_playButton.getPosition().y + (m_playButton.getSize().y - textBounds.height) / 2 - 5);

        // Initialize speed slider
        m_speedSlider.setSize(sf::Vector2f(160, 5));
        m_speedSlider.setFillColor(sf::Color(100, 100, 100));
        m_speedSlider.setPosition(20, 130);

        m_sliderHandle.setRadius(8);
        m_sliderHandle.setFillColor(sf::Color(200, 200, 200));
        m_sliderHandle.setOrigin(8, 8);
        updateSliderPosition();

        m_speedText.setFont(m_font);
        m_speedText.setString("Speed: 1.0x");
        m_speedText.setCharacterSize(14);
        m_speedText.setFillColor(sf::Color::White);
        m_speedText.setPosition(20, 145);

        // Initialize info texts
        for (int i = 0; i < 5; ++i)
        {
            sf::Text infoText;
            infoText.setFont(m_font);
            infoText.setCharacterSize(12);
            infoText.setFillColor(sf::Color(200, 200, 200));
            infoText.setPosition(20, 180 + i * 20);
            m_infoTexts.push_back(infoText);
        }

        updateInfoTexts();
    }

    void handleEvent(const sf::Event &event, const sf::Vector2f &mousePos)
    {
        // Handle play button click
        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
        {
            sf::FloatRect buttonBounds = m_playButton.getGlobalBounds();
            if (buttonBounds.contains(mousePos))
            {
                m_isPlaying = !m_isPlaying;
                m_playButtonText.setString(m_isPlaying ? "Pause" : "Resume");
                m_status.setString(m_isPlaying ? "Status: Running" : "Status: Paused");
                m_status.setFillColor(m_isPlaying ? sf::Color::Green : sf::Color::Yellow);

                // Update button text position
                sf::FloatRect textBounds = m_playButtonText.getLocalBounds();
                m_playButtonText.setPosition(
                    m_playButton.getPosition().x + (m_playButton.getSize().x - textBounds.width) / 2,
                    m_playButton.getPosition().y + (m_playButton.getSize().y - textBounds.height) / 2 - 5);
            }

            // Handle slider click
            sf::FloatRect sliderBounds(m_speedSlider.getPosition().x, m_speedSlider.getPosition().y - 10,
                                       m_speedSlider.getSize().x, 20);
            if (sliderBounds.contains(mousePos))
            {
                m_isDragging = true;
                updateSpeedFromPosition(mousePos.x);
            }
        }

        // Handle mouse release
        if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left)
        {
            m_isDragging = false;
        }

        // Handle mouse drag
        if (event.type == sf::Event::MouseMoved && m_isDragging)
        {
            updateSpeedFromPosition(mousePos.x);
        }
    }

    void updateInfoTexts()
    {
        m_infoTexts[0].setString("Sim Time: 00:00:00");
        m_infoTexts[1].setString("Vehicles: 5");
        m_infoTexts[2].setString("Active Tasks: 3");
        m_infoTexts[3].setString("Completed: 0");
        m_infoTexts[4].setString("Avg Cycle: 0.0s");
    }

    void updateTime(float seconds)
    {
        if (!m_isPlaying)
            return;

        int hours = static_cast<int>(seconds / 3600);
        int minutes = static_cast<int>((seconds - hours * 3600) / 60);
        int secs = static_cast<int>(seconds) % 60;

        char timeStr[20];
        std::snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d", hours, minutes, secs);
        m_infoTexts[0].setString("Sim Time: " + std::string(timeStr));
    }

    void draw(sf::RenderTarget &target)
    {
        target.draw(m_background);
        target.draw(m_title);
        target.draw(m_status);
        target.draw(m_playButton);
        target.draw(m_playButtonText);
        target.draw(m_speedSlider);
        target.draw(m_sliderHandle);
        target.draw(m_speedText);

        for (const auto &text : m_infoTexts)
        {
            target.draw(text);
        }
    }

    bool isPlaying() const
    {
        return m_isPlaying;
    }

    float getSimulationSpeed() const
    {
        return m_simulationSpeed;
    }

private:
    void updateSliderPosition()
    {
        // Calculate slider position
        float normalizedSpeed = (m_simulationSpeed - 0.1f) / 4.9f; // Speed range 0.1-5.0
        float positionX = m_speedSlider.getPosition().x + normalizedSpeed * m_speedSlider.getSize().x;
        m_sliderHandle.setPosition(positionX, m_speedSlider.getPosition().y + 2.5f);
    }

    void updateSpeedFromPosition(float posX)
    {
        float sliderStart = m_speedSlider.getPosition().x;
        float sliderEnd = sliderStart + m_speedSlider.getSize().x;
        float normalizedPos = std::max(0.0f, std::min(1.0f, (posX - sliderStart) / (sliderEnd - sliderStart)));

        m_simulationSpeed = 0.1f + normalizedPos * 4.9f;
        m_simulationSpeed = std::round(m_simulationSpeed * 10.0f) / 10.0f; // Round to 1 decimal place

        m_speedText.setString("Speed: " + std::to_string(m_simulationSpeed).substr(0, 3) + "x");
        updateSliderPosition();
    }
};

// Task panel and object detail display
class TaskPanel
{
private:
    sf::RectangleShape m_background;
    sf::Font m_font;
    sf::Text m_title;
    std::vector<sf::Text> m_taskTexts;

    std::vector<Task> m_tasks; // Task list

public:
    TaskPanel()
    {
        // Load font
        if (!m_font.loadFromFile("resources/fonts/arial.ttf"))
        {
            std::cerr << "Failed to load font!" << std::endl;
        }

        // Initialize background
        m_background.setSize(sf::Vector2f(280, 300));
        m_background.setFillColor(sf::Color(50, 50, 50, 200));
        m_background.setPosition(734, 10);

        // Initialize title
        m_title.setFont(m_font);
        m_title.setString("Task Queue");
        m_title.setCharacterSize(16);
        m_title.setFillColor(sf::Color::White);
        m_title.setPosition(744, 15);

        // Create sample tasks
        createSampleTasks();
        updateTaskTexts();
    }

    void createSampleTasks()
    {
        // Create some sample tasks
        sf::Clock clock;

        Task t1;
        t1.taskId = 101;
        t1.type = TaskType::input;
        t1.materialId = 5001;
        t1.startDeviceId = 2;
        t1.endDeviceId = 15;
        t1.createTime = clock.getElapsedTime();
        t1.startTime = clock.getElapsedTime() + sf::seconds(5);
        t1.completeTime = sf::Time::Zero;
        t1.assignedVehicleId = 1;
        m_tasks.push_back(t1);

        Task t2;
        t2.taskId = 102;
        t2.type = TaskType::output;
        t2.materialId = 5002;
        t2.startDeviceId = 14;
        t2.endDeviceId = 8;
        t2.createTime = clock.getElapsedTime() + sf::seconds(2);
        t2.startTime = clock.getElapsedTime() + sf::seconds(10);
        t2.completeTime = sf::Time::Zero;
        t2.assignedVehicleId = 2;
        m_tasks.push_back(t2);

        Task t3;
        t3.taskId = 103;
        t3.type = TaskType::input;
        t3.materialId = 5003;
        t3.startDeviceId = 3;
        t3.endDeviceId = 16;
        t3.createTime = clock.getElapsedTime() + sf::seconds(4);
        t3.startTime = sf::Time::Zero;
        t3.completeTime = sf::Time::Zero;
        t3.assignedVehicleId = -1;
        m_tasks.push_back(t3);
    }

    void updateTaskTexts()
    {
        m_taskTexts.clear();

        int y = 45;
        for (const auto &task : m_tasks)
        {
            std::string typeStr = (task.type == TaskType::input) ? "Input" : "Output";
            std::string assignedStr = (task.assignedVehicleId >= 0) ? "Vehicle:" + std::to_string(task.assignedVehicleId) : "Waiting";

            sf::Text idText;
            idText.setFont(m_font);
            idText.setString("Task #" + std::to_string(task.taskId) + " (" + typeStr + ")");
            idText.setCharacterSize(14);
            idText.setFillColor(sf::Color::White);
            idText.setPosition(744, y);
            m_taskTexts.push_back(idText);

            sf::Text detailText;
            detailText.setFont(m_font);
            std::string fromTo = "From Device #" + std::to_string(task.startDeviceId) +
                                 " To Device #" + std::to_string(task.endDeviceId);
            detailText.setString(fromTo);
            detailText.setCharacterSize(12);
            detailText.setFillColor(sf::Color(200, 200, 200));
            detailText.setPosition(744, y + 20);
            m_taskTexts.push_back(detailText);

            sf::Text statusText;
            statusText.setFont(m_font);
            statusText.setString(assignedStr);
            statusText.setCharacterSize(12);
            statusText.setFillColor(sf::Color(150, 200, 150));
            statusText.setPosition(744, y + 40);
            m_taskTexts.push_back(statusText);

            y += 70;
        }
    }

    void draw(sf::RenderTarget &target)
    {
        target.draw(m_background);
        target.draw(m_title);

        for (const auto &text : m_taskTexts)
        {
            target.draw(text);
        }
    }
};

// Object info panel
class ObjectInfoPanel
{
private:
    sf::RectangleShape m_background;
    sf::Font m_font;
    sf::Text m_title;
    std::vector<sf::Text> m_infoTexts;

    enum class SelectedType
    {
        None,
        Vehicle,
        Device
    } m_selectedType = SelectedType::None;

    int m_selectedId = -1;

public:
    ObjectInfoPanel()
    {
        // Load font
        if (!m_font.loadFromFile("resources/fonts/arial.ttf"))
        {
            std::cerr << "Failed to load font!" << std::endl;
        }

        // Initialize background
        m_background.setSize(sf::Vector2f(280, 300));
        m_background.setFillColor(sf::Color(50, 50, 50, 200));
        m_background.setPosition(734, 320);

        // Initialize title
        m_title.setFont(m_font);
        m_title.setString("Object Information");
        m_title.setCharacterSize(16);
        m_title.setFillColor(sf::Color::White);
        m_title.setPosition(744, 325);

        // Initialize no selection message
        sf::Text noSelection;
        noSelection.setFont(m_font);
        noSelection.setString("No object selected\nClick on a vehicle or device for details");
        noSelection.setCharacterSize(14);
        noSelection.setFillColor(sf::Color(180, 180, 180));
        noSelection.setPosition(744, 360);
        m_infoTexts.push_back(noSelection);
    }

    void selectVehicle(const VehicleState &vehicle)
    {
        m_selectedType = SelectedType::Vehicle;
        m_selectedId = vehicle.id;

        updateInfoForVehicle(vehicle);
    }

    void selectDevice(const DeviceState &device)
    {
        m_selectedType = SelectedType::Device;
        m_selectedId = device.id;

        updateInfoForDevice(device);
    }

    void clearSelection()
    {
        m_selectedType = SelectedType::None;
        m_selectedId = -1;

        m_infoTexts.clear();
        sf::Text noSelection;
        noSelection.setFont(m_font);
        noSelection.setString("No object selected\nClick on a vehicle or device for details");
        noSelection.setCharacterSize(14);
        noSelection.setFillColor(sf::Color(180, 180, 180));
        noSelection.setPosition(744, 360);
        m_infoTexts.push_back(noSelection);
    }

    void updateInfoForVehicle(const VehicleState &vehicle)
    {
        m_infoTexts.clear();

        sf::Text titleText;
        titleText.setFont(m_font);
        titleText.setString("Vehicle #" + std::to_string(vehicle.id));
        titleText.setCharacterSize(15);
        titleText.setStyle(sf::Text::Bold);
        titleText.setFillColor(sf::Color::White);
        titleText.setPosition(744, 360);
        m_infoTexts.push_back(titleText);

        sf::Text speedText;
        speedText.setFont(m_font);
        speedText.setString("Speed: " + std::to_string(vehicle.speed).substr(0, 4) + " units/sec");
        speedText.setCharacterSize(14);
        speedText.setFillColor(sf::Color(180, 180, 180));
        speedText.setPosition(744, 390);
        m_infoTexts.push_back(speedText);

        sf::Text posText;
        posText.setFont(m_font);
        posText.setString("Track Position: " + std::to_string(vehicle.position).substr(0, 4));
        posText.setCharacterSize(14);
        posText.setFillColor(sf::Color(180, 180, 180));
        posText.setPosition(744, 410);
        m_infoTexts.push_back(posText);

        sf::Text statusText;
        statusText.setFont(m_font);
        statusText.setString("Status: " + std::string(vehicle.isLoaded ? "Loaded" : "Empty"));
        statusText.setCharacterSize(14);
        statusText.setFillColor(vehicle.isLoaded ? sf::Color(200, 130, 100) : sf::Color(100, 180, 200));
        statusText.setPosition(744, 430);
        m_infoTexts.push_back(statusText);
    }

    void updateInfoForDevice(const DeviceState &device)
    {
        m_infoTexts.clear();

        std::string typeStr;
        switch (device.type)
        {
        case DeviceType::StorageIn:
            typeStr = "Storage Input";
            break;
        case DeviceType::StorageOut:
            typeStr = "Storage Output";
            break;
        case DeviceType::WorkstationIn:
            typeStr = "Workstation Input";
            break;
        case DeviceType::WorkstationOut:
            typeStr = "Workstation Output";
            break;
        default:
            typeStr = "Unknown Type";
        }

        sf::Text titleText;
        titleText.setFont(m_font);
        titleText.setString(typeStr + " #" + std::to_string(device.id));
        titleText.setCharacterSize(15);
        titleText.setStyle(sf::Text::Bold);
        titleText.setFillColor(sf::Color::White);
        titleText.setPosition(744, 360);
        m_infoTexts.push_back(titleText);

        sf::Text statusText;
        statusText.setFont(m_font);
        statusText.setString("Status: " + std::string(device.status == DeviceStatus::working ? "Working" : "Idle"));
        statusText.setCharacterSize(14);
        statusText.setFillColor(device.status == DeviceStatus::working ? sf::Color(200, 130, 100) : sf::Color(100, 180, 100));
        statusText.setPosition(744, 390);
        m_infoTexts.push_back(statusText);

        sf::Text queueText;
        queueText.setFont(m_font);
        queueText.setString("Queue Tasks: " + std::to_string(device.queuedTaskCount));
        queueText.setCharacterSize(14);
        queueText.setFillColor(sf::Color(180, 180, 180));
        queueText.setPosition(744, 410);
        m_infoTexts.push_back(queueText);
    }

    void draw(sf::RenderTarget &target)
    {
        target.draw(m_background);
        target.draw(m_title);

        for (const auto &text : m_infoTexts)
        {
            target.draw(text);
        }
    }

    int getSelectedId() const
    {
        return m_selectedId;
    }

    SelectedType getSelectedType() const
    {
        return m_selectedType;
    }
};

// Main function
int main()
{
    // Create window
    sf::RenderWindow window(sf::VideoMode(1024, 768), "Warehouse Shuttle Simulation System", sf::Style::Default);
    window.setFramerateLimit(60);
    // Initialize renderers and panels
    TrackRenderer trackRenderer;
    VehicleRenderer vehicleRenderer;
    DeviceRenderer deviceRenderer;
    ControlPanel controlPanel;
    TaskPanel taskPanel;
    ObjectInfoPanel infoPanel;

    // Create view (for zooming and panning)
    sf::View worldView = window.getDefaultView();

    // Create simulated data
    std::vector<VehicleState> vehicles;
    for (int i = 0; i < 5; ++i)
    {
        VehicleState vehicle;
        vehicle.id = i;
        vehicle.position = i * 0.2f;
        vehicle.speed = 0.3f + i * 0.1f;
        vehicle.isLoaded = (i % 2 == 0);
        vehicles.push_back(vehicle);
    }

    std::vector<DeviceState> devices;

    // Add storage input devices
    for (int i = 0; i < 6; ++i)
    {
        DeviceState device;
        device.id = i + 1;
        device.type = DeviceType::StorageIn;
        device.status = (i % 3 == 0) ? DeviceStatus::working : DeviceStatus::idle;
        device.queuedTaskCount = i % 2;
        devices.push_back(device);
    }

    // Add storage output devices
    for (int i = 0; i < 6; ++i)
    {
        DeviceState device;
        device.id = i + 7;
        device.type = DeviceType::StorageOut;
        device.status = (i % 4 == 0) ? DeviceStatus::working : DeviceStatus::idle;
        device.queuedTaskCount = i % 3;
        devices.push_back(device);
    }

    // Add workstation devices
    for (int i = 0; i < 3; ++i)
    {
        DeviceState device;
        device.id = i + 13;
        device.type = DeviceType::WorkstationOut;
        device.status = (i % 2 == 0) ? DeviceStatus::working : DeviceStatus::idle;
        devices.push_back(device);

        device.id = i + 16;
        device.type = DeviceType::WorkstationIn;
        device.status = (i % 2 == 1) ? DeviceStatus::working : DeviceStatus::idle;
        devices.push_back(device);
    }

    // Calculate device positions
    std::map<int, sf::Vector2f> devicePositions;
    float radius = 100.0f;
    float outerRadius = radius + 50.0f;

    // Storage input device positions
    for (int i = 0; i < 6; ++i)
    {
        float angle = M_PI / 7.0f * i - M_PI / 3.0f;
        devicePositions[i + 1] = sf::Vector2f(std::cos(angle) * outerRadius, std::sin(angle) * outerRadius * 0.7f);
    }

    // Storage output device positions
    for (int i = 0; i < 6; ++i)
    {
        float angle = M_PI + M_PI / 7.0f * i - M_PI / 3.0f;
        devicePositions[i + 7] = sf::Vector2f(std::cos(angle) * outerRadius, std::sin(angle) * outerRadius * 0.7f);
    }

    // Workstation device positions
    for (int i = 0; i < 3; ++i)
    {
        devicePositions[i + 13] = sf::Vector2f(-outerRadius - 20.0f, -60.0f + i * 60.0f);
        devicePositions[i + 16] = sf::Vector2f(outerRadius + 20.0f, -60.0f + i * 60.0f);
    }

    // Simulation parameters
    sf::Clock clock;
    float simulationTime = 0.0f;
    bool isDragging = false;
    sf::Vector2f lastMousePos;

    // Main loop
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }

            // Get current mouse position
            sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

            // Handle control panel events
            controlPanel.handleEvent(event, sf::Vector2f(sf::Mouse::getPosition(window)));

            // Handle view dragging
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Right)
            {
                isDragging = true;
                lastMousePos = mousePos;
            }
            else if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Right)
            {
                isDragging = false;
            }
            else if (event.type == sf::Event::MouseMoved && isDragging)
            {
                sf::Vector2f delta = lastMousePos - mousePos;
                worldView.move(delta);
                window.setView(worldView);
                lastMousePos = mousePos;
            }

            // Handle mouse wheel zoom
            if (event.type == sf::Event::MouseWheelScrolled)
            {
                if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel)
                {
                    float factor = event.mouseWheelScroll.delta > 0 ? 0.9f : 1.1f;
                    worldView.zoom(factor);
                    window.setView(worldView);
                }
            }

            // Handle mouse click events for object selection
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
            {
                // Get mouse position in world coordinates
                sf::Vector2f worldPos = window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y), worldView);

                // Try to select a vehicle
                bool vehicleSelected = false;
                for (const auto &vehicle : vehicles)
                {
                    float angle = vehicle.position * 2.0f * M_PI;
                    float x = radius * std::cos(angle);
                    float y = radius * std::sin(angle) * 0.7f;

                    sf::Vector2f vehiclePos(x, y);
                    if (std::sqrt(std::pow(worldPos.x - vehiclePos.x, 2) + std::pow(worldPos.y - vehiclePos.y, 2)) < 15.0f)
                    {
                        infoPanel.selectVehicle(vehicle);
                        vehicleSelected = true;
                        break;
                    }
                }

                // If no vehicle selected, try to select a device
                if (!vehicleSelected)
                {
                    bool deviceSelected = false;
                    for (const auto &device : devices)
                    {
                        sf::Vector2f devicePos = devicePositions[device.id];
                        if (std::sqrt(std::pow(worldPos.x - devicePos.x, 2) + std::pow(worldPos.y - devicePos.y, 2)) < 20.0f)
                        {
                            infoPanel.selectDevice(device);
                            deviceSelected = true;
                            break;
                        }
                    }

                    // If clicked on empty area, clear selection
                    if (!deviceSelected && !vehicleSelected)
                    {
                        infoPanel.clearSelection();
                    }
                }
            }
        }

        // Update simulation
        float deltaTime = clock.restart().asSeconds();
        if (controlPanel.isPlaying())
        {
            float speedFactor = controlPanel.getSimulationSpeed();
            simulationTime += deltaTime * speedFactor;

            // Update vehicle positions
            for (auto &vehicle : vehicles)
            {
                vehicle.position += vehicle.speed * deltaTime * speedFactor * 0.02f;
                if (vehicle.position >= 1.0f)
                {
                    vehicle.position -= 1.0f;
                }
            }

            // Update control panel time
            controlPanel.updateTime(simulationTime);
        }

        // Clear screen
        window.clear(sf::Color(40, 40, 45));

        // Set world view
        window.setView(worldView);

        // Draw track
        trackRenderer.draw(window, sf::RenderStates::Default);

        // Draw devices
        for (const auto &device : devices)
        {
            deviceRenderer.draw(window, device, devicePositions[device.id]);
        }

        // Draw vehicles
        for (const auto &vehicle : vehicles)
        {
            vehicleRenderer.draw(window, vehicle, 100.0f);
        }

        // Restore default view
        window.setView(window.getDefaultView());

        // Draw control panel
        controlPanel.draw(window);
        // Draw task panel
        taskPanel.draw(window);

        // Draw object info panel
        infoPanel.draw(window);

        // Display result
        window.display();
    }

    return 0;
}
