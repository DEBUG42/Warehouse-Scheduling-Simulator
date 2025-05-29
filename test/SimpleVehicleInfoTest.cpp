/**
 * @file SimpleVehicleInfoTest.cpp
 * @brief 简化的VehicleInfoPanel集成测试
 *
 * 这个测试专注于验证VehicleInfoPanel的核心功能：
 * 1. VehicleInfoPanel的显示和渲染
 * 2. 车辆选择机制模拟
 * 3. 信息更新和显示验证
 * 4. GUI组件交互
 */

#include <SFML/Graphics.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <algorithm>

// 简化的Vehicle类用于测试
class TestVehicle
{
public:
    int id;
    sf::Vector2f position;
    float speed;
    float acceleration;
    std::string status;

    TestVehicle(int id, sf::Vector2f pos)
        : id(id), position(pos), speed(0.0f), acceleration(0.0f), status("Idle")
    {
    }

    void updateMovement(float deltaTime)
    {
        static float time = 0.0f;
        time += deltaTime;

        // 简单的运动模拟
        if (id == 1)
        {
            position.x = 100.0f + 200.0f * std::sin(time * 0.5f);
            position.y = 100.0f + 100.0f * std::cos(time * 0.3f);
            speed = 50.0f + 30.0f * std::sin(time * 2.0f);
            acceleration = 20.0f * std::cos(time * 2.0f);
            status = (speed > 60.0f) ? "Accelerating" : (speed < 40.0f) ? "Decelerating"
                                                                        : "Cruising";
        }
        else if (id == 2)
        {
            position.x = 300.0f + 150.0f * std::sin(time * 0.3f);
            position.y = 200.0f;
            speed = std::max(0.0f, 40.0f * std::sin(time * 0.8f));
            acceleration = (speed > 0.1f) ? 15.0f : 0.0f;
            status = (speed < 1.0f) ? "Stopped" : "Moving";
        }
        else
        {
            position.x = 500.0f + 100.0f * std::cos(time * 0.7f);
            position.y = 300.0f + 80.0f * std::sin(time * 0.9f);
            speed = 45.0f + 15.0f * std::sin(time * 1.5f);
            acceleration = 10.0f * std::cos(time * 1.5f);
            status = "Normal";
        }
    }
};

// 简化的VehicleInfoPanel类
class SimpleVehicleInfoPanel : public sf::Drawable
{
private:
    sf::Font &m_font;
    sf::RectangleShape m_background;
    sf::Vector2f m_position;
    float m_width;
    float m_height;

    sf::Text m_titleText;
    sf::Text m_idText;
    sf::Text m_positionText;
    sf::Text m_speedText;
    sf::Text m_accelerationText;
    sf::Text m_statusText;

    const TestVehicle *m_selectedVehicle;

public:
    SimpleVehicleInfoPanel(sf::Font &font, float width, float height)
        : m_font(font), m_width(width), m_height(height), m_selectedVehicle(nullptr)
    {
        // 设置背景
        m_background.setSize(sf::Vector2f(m_width, m_height));
        m_background.setFillColor(sf::Color(40, 40, 40, 230));
        m_background.setOutlineThickness(2.0f);
        m_background.setOutlineColor(sf::Color(100, 150, 200));

        // 设置标题
        m_titleText.setFont(m_font);
        m_titleText.setCharacterSize(16);
        m_titleText.setFillColor(sf::Color(100, 150, 200));
        m_titleText.setString("Vehicle Information");
        m_titleText.setStyle(sf::Text::Bold);

        // 设置信息文本
        setupText(m_idText);
        setupText(m_positionText);
        setupText(m_speedText);
        setupText(m_accelerationText);
        setupText(m_statusText);

        updateLayout();
    }

    void setupText(sf::Text &text)
    {
        text.setFont(m_font);
        text.setCharacterSize(12);
        text.setFillColor(sf::Color::White);
    }

    void setPosition(const sf::Vector2f &position)
    {
        m_position = position;
        updateLayout();
    }

    void updateLayout()
    {
        m_background.setPosition(m_position);

        float padding = 10.0f;
        float lineHeight = 20.0f;
        float yOffset = m_position.y + padding;

        m_titleText.setPosition(m_position.x + padding, yOffset);
        yOffset += lineHeight * 1.5f;

        m_idText.setPosition(m_position.x + padding, yOffset);
        yOffset += lineHeight;

        m_positionText.setPosition(m_position.x + padding, yOffset);
        yOffset += lineHeight;

        m_speedText.setPosition(m_position.x + padding, yOffset);
        yOffset += lineHeight;

        m_accelerationText.setPosition(m_position.x + padding, yOffset);
        yOffset += lineHeight;

        m_statusText.setPosition(m_position.x + padding, yOffset);
    }

    void setSelectedVehicle(const TestVehicle *vehicle)
    {
        m_selectedVehicle = vehicle;
        updateDisplay();
    }

    void updateDisplay()
    {
        if (!m_selectedVehicle)
        {
            m_idText.setString("ID: None");
            m_positionText.setString("Position: --");
            m_speedText.setString("Speed: --");
            m_accelerationText.setString("Acceleration: --");
            m_statusText.setString("Status: --");
            return;
        }

        // 格式化显示信息
        m_idText.setString("ID: " + std::to_string(m_selectedVehicle->id));

        std::ostringstream posStream;
        posStream << "Position: (" << std::fixed << std::setprecision(1)
                  << m_selectedVehicle->position.x << ", " << m_selectedVehicle->position.y << ")";
        m_positionText.setString(posStream.str());

        std::ostringstream speedStream;
        speedStream << "Speed: " << std::fixed << std::setprecision(1) << m_selectedVehicle->speed << " m/s";
        m_speedText.setString(speedStream.str());

        std::ostringstream accelStream;
        accelStream << "Acceleration: " << std::fixed << std::setprecision(1) << m_selectedVehicle->acceleration << " m/s²";
        m_accelerationText.setString(accelStream.str());

        m_statusText.setString("Status: " + m_selectedVehicle->status);
    }

    bool contains(const sf::Vector2f &point) const
    {
        return m_background.getGlobalBounds().contains(point);
    }

private:
    virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const override
    {
        target.draw(m_background, states);
        target.draw(m_titleText, states);
        target.draw(m_idText, states);
        target.draw(m_positionText, states);
        target.draw(m_speedText, states);
        target.draw(m_accelerationText, states);
        target.draw(m_statusText, states);
    }
};

// 简化的车辆渲染器
class SimpleVehicleRenderer : public sf::Drawable
{
private:
    std::vector<TestVehicle> &m_vehicles;
    sf::Font &m_font;
    mutable sf::CircleShape m_vehicleShape; // 添加mutable
    mutable sf::Text m_idText;              // 添加mutable

public:
    SimpleVehicleRenderer(std::vector<TestVehicle> &vehicles, sf::Font &font)
        : m_vehicles(vehicles), m_font(font)
    {
        m_vehicleShape.setRadius(15.0f);
        m_vehicleShape.setOrigin(15.0f, 15.0f);

        m_idText.setFont(m_font);
        m_idText.setCharacterSize(10);
        m_idText.setFillColor(sf::Color::White);
    }

    int getVehicleAtPosition(const sf::Vector2f &position) const
    {
        for (const auto &vehicle : m_vehicles)
        {
            sf::Vector2f diff = position - vehicle.position;
            float distance = std::sqrt(diff.x * diff.x + diff.y * diff.y);
            if (distance <= 20.0f) // 点击检测半径
            {
                return vehicle.id;
            }
        }
        return -1; // 未找到车辆
    }

private:
    virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const override
    {
        for (const auto &vehicle : m_vehicles)
        {
            // 根据状态设置颜色
            sf::Color vehicleColor = sf::Color::Blue;
            if (vehicle.status == "Accelerating")
                vehicleColor = sf::Color::Green;
            else if (vehicle.status == "Decelerating")
                vehicleColor = sf::Color::Red;
            else if (vehicle.status == "Stopped")
                vehicleColor = sf::Color(128, 128, 128); // Gray color

            m_vehicleShape.setPosition(vehicle.position);
            m_vehicleShape.setFillColor(vehicleColor);
            target.draw(m_vehicleShape, states);

            // 绘制车辆ID
            m_idText.setString(std::to_string(vehicle.id));
            sf::FloatRect textBounds = m_idText.getLocalBounds();
            m_idText.setPosition(vehicle.position.x - textBounds.width / 2.0f,
                                 vehicle.position.y - textBounds.height / 2.0f);
            target.draw(m_idText, states);
        }
    }
};

// 测试控制器
class VehicleInfoTestController
{
private:
    sf::RenderWindow &m_window;
    sf::Font &m_font;
    std::vector<TestVehicle> m_vehicles;
    std::unique_ptr<SimpleVehicleInfoPanel> m_infoPanel;
    std::unique_ptr<SimpleVehicleRenderer> m_vehicleRenderer;
    int m_selectedVehicleId;

    sf::Text m_instructionText;
    sf::Text m_statusText;
    sf::Clock m_testClock;
    int m_testPhase;

public:
    VehicleInfoTestController(sf::RenderWindow &window, sf::Font &font)
        : m_window(window), m_font(font), m_selectedVehicleId(-1), m_testPhase(0)
    {
        // 创建测试车辆
        m_vehicles.emplace_back(1, sf::Vector2f(150.0f, 150.0f));
        m_vehicles.emplace_back(2, sf::Vector2f(350.0f, 250.0f));
        m_vehicles.emplace_back(3, sf::Vector2f(550.0f, 350.0f));

        // 创建组件
        m_infoPanel = std::make_unique<SimpleVehicleInfoPanel>(m_font, 280.0f, 200.0f);
        m_vehicleRenderer = std::make_unique<SimpleVehicleRenderer>(m_vehicles, m_font);

        // 设置信息面板位置
        sf::Vector2u windowSize = m_window.getSize();
        m_infoPanel->setPosition(sf::Vector2f(windowSize.x - 300.0f, 50.0f));

        // 设置指令文本
        m_instructionText.setFont(m_font);
        m_instructionText.setCharacterSize(14);
        m_instructionText.setFillColor(sf::Color::Yellow);
        m_instructionText.setPosition(10.0f, 10.0f);

        m_statusText.setFont(m_font);
        m_statusText.setCharacterSize(12);
        m_statusText.setFillColor(sf::Color::Cyan);
        m_statusText.setPosition(10.0f, windowSize.y - 100.0f);

        updateInstructions();
    }

    void updateInstructions()
    {
        std::string instructions;
        switch (m_testPhase)
        {
        case 0:
            instructions = "TEST PHASE 1: Click on vehicles to select them\n"
                           "Instructions: Click on any colored circle (vehicle) to see info panel update";
            break;
        case 1:
            instructions = "TEST PHASE 2: Automatic vehicle selection test\n"
                           "Instructions: Watch automatic vehicle selection cycling";
            break;
        case 2:
            instructions = "TEST PHASE 3: Real-time data update test\n"
                           "Instructions: Observe real-time vehicle data updates";
            break;
        default:
            instructions = "TEST COMPLETE: All phases finished\n"
                           "Instructions: Press ESC to exit";
            break;
        }

        instructions += "\n\nControls:\n"
                        "- Click vehicles to select\n"
                        "- Press SPACE to advance test phase\n"
                        "- Press ESC to exit";

        m_instructionText.setString(instructions);
    }

    void handleEvent(const sf::Event &event)
    {
        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
        {
            sf::Vector2f mousePos(static_cast<float>(event.mouseButton.x),
                                  static_cast<float>(event.mouseButton.y));

            int clickedVehicleId = m_vehicleRenderer->getVehicleAtPosition(mousePos);
            if (clickedVehicleId != -1)
            {
                selectVehicle(clickedVehicleId);
                std::cout << "Vehicle " << clickedVehicleId << " selected by click" << std::endl;
            }
            else
            {
                selectVehicle(-1); // 取消选择
                std::cout << "Vehicle selection cleared" << std::endl;
            }
        }
        else if (event.type == sf::Event::KeyPressed)
        {
            if (event.key.code == sf::Keyboard::Space)
            {
                m_testPhase++;
                updateInstructions();
                m_testClock.restart();
                std::cout << "Advanced to test phase " << m_testPhase << std::endl;
            }
        }
    }

    void update(float deltaTime)
    {
        // 更新车辆运动
        for (auto &vehicle : m_vehicles)
        {
            vehicle.updateMovement(deltaTime);
        }

        // 更新选中车辆的信息显示
        if (m_selectedVehicleId != -1)
        {
            for (const auto &vehicle : m_vehicles)
            {
                if (vehicle.id == m_selectedVehicleId)
                {
                    m_infoPanel->setSelectedVehicle(&vehicle);
                    break;
                }
            }
        }

        // 自动测试阶段
        float testTime = m_testClock.getElapsedTime().asSeconds();
        if (m_testPhase == 1 && testTime > 2.0f)
        {
            // 自动循环选择车辆
            int autoSelectId = static_cast<int>(testTime / 2.0f) % 4; // 0, 1, 2, 3 循环
            if (autoSelectId == 0 || autoSelectId > 3)
            {
                selectVehicle(-1); // 取消选择
            }
            else
            {
                selectVehicle(autoSelectId);
            }
        }

        // 更新状态显示
        updateStatusDisplay();
    }

    void selectVehicle(int vehicleId)
    {
        m_selectedVehicleId = vehicleId;

        if (vehicleId == -1)
        {
            m_infoPanel->setSelectedVehicle(nullptr);
        }
        else
        {
            for (const auto &vehicle : m_vehicles)
            {
                if (vehicle.id == vehicleId)
                {
                    m_infoPanel->setSelectedVehicle(&vehicle);
                    break;
                }
            }
        }
    }

    void updateStatusDisplay()
    {
        std::ostringstream status;
        status << "Selected Vehicle: " << (m_selectedVehicleId == -1 ? "None" : std::to_string(m_selectedVehicleId)) << "\n";
        status << "Test Phase: " << m_testPhase << "\n";
        status << "Vehicle Count: " << m_vehicles.size() << "\n";
        status << "Test Time: " << std::fixed << std::setprecision(1) << m_testClock.getElapsedTime().asSeconds() << "s\n";

        if (m_selectedVehicleId != -1)
        {
            for (const auto &vehicle : m_vehicles)
            {
                if (vehicle.id == m_selectedVehicleId)
                {
                    status << "Selected Vehicle Status: " << vehicle.status;
                    break;
                }
            }
        }

        m_statusText.setString(status.str());
    }

    void render()
    {
        m_window.draw(*m_vehicleRenderer);
        m_window.draw(*m_infoPanel);
        m_window.draw(m_instructionText);
        m_window.draw(m_statusText);
    }

    bool isTestComplete() const
    {
        return m_testPhase > 3;
    }
};

int main()
{
    try
    {
        std::cout << "=== Simple VehicleInfoPanel Integration Test ===" << std::endl;
        std::cout << "Starting test application..." << std::endl;

        // 创建窗口
        sf::RenderWindow window(sf::VideoMode(1000, 700), "VehicleInfoPanel Integration Test");
        window.setFramerateLimit(60);

        // 加载字体
        sf::Font font;
        if (!font.loadFromFile("assets/fonts/arial.ttf"))
        {
            std::cerr << "Failed to load font. Using default font." << std::endl;
            // 继续使用默认字体
        }

        // 创建测试控制器
        VehicleInfoTestController testController(window, font);

        sf::Clock frameClock;
        std::cout << "Test application initialized successfully." << std::endl;
        std::cout << "Test Instructions:" << std::endl;
        std::cout << "1. Click on vehicles (colored circles) to select them" << std::endl;
        std::cout << "2. Press SPACE to advance through test phases" << std::endl;
        std::cout << "3. Press ESC to exit" << std::endl;

        // 主循环
        while (window.isOpen())
        {
            float deltaTime = frameClock.restart().asSeconds();

            // 处理事件
            sf::Event event;
            while (window.pollEvent(event))
            {
                if (event.type == sf::Event::Closed)
                {
                    window.close();
                }
                else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
                {
                    window.close();
                }
                else
                {
                    testController.handleEvent(event);
                }
            }

            // 更新
            testController.update(deltaTime);

            // 渲染
            window.clear(sf::Color(30, 30, 30));
            testController.render();
            window.display();

            // 检查测试完成
            if (testController.isTestComplete())
            {
                std::cout << "All test phases completed successfully!" << std::endl;
            }
        }

        std::cout << "Test application terminated." << std::endl;
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return -1;
    }
}
