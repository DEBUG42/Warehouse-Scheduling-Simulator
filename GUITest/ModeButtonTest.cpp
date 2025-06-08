#include <SFML/Graphics.hpp>
#include <iostream>
#include <memory>
#include "gui/Toolbar.hpp"
#include "gui/RealBackendAdapter.hpp"

/**
 * @brief 模式切换按钮功能测试
 *
 * 测试目标：
 * 1. 验证四个模式按钮(Task1, Task2.1, Task2.2, Task2.3)能正确显示
 * 2. 验证点击按钮能触发模式切换回调
 * 3. 验证模式切换信号能正确传递到后端适配器
 * 4. 验证按钮状态能正确更新（激活/非激活状态）
 */
class ModeButtonTest
{
private:
    sf::RenderWindow m_window;
    sf::Font m_font;
    std::unique_ptr<Toolbar> m_toolbar;
    std::shared_ptr<RealBackendAdapter> m_backendAdapter;

    // 测试状态
    SimulationMode m_currentMode;
    bool m_modeChanged;

public:
    ModeButtonTest()
        : m_window(sf::VideoMode(1200, 600), "Mode Button Test"), m_currentMode(SimulationMode::TASK1), m_modeChanged(false)
    {
        m_window.setFramerateLimit(60);
    }

    bool initialize()
    {
        // 加载字体
        if (!m_font.loadFromFile("assets/fonts/arial.ttf"))
        {
            std::cerr << "Failed to load font!" << std::endl;
            return false;
        }

        // 创建后端适配器
        m_backendAdapter = std::make_shared<RealBackendAdapter>();

        // 创建工具栏
        m_toolbar = std::make_unique<Toolbar>(m_font, 1200.0f, 50.0f);

        // 设置模式切换回调
        m_toolbar->setOnModeChanged([this](SimulationMode mode)
                                    {
            std::cout << "Mode changed to: ";
            switch(mode) {
                case SimulationMode::TASK1:
                    std::cout << "TASK1";
                    break;
                case SimulationMode::TASK2_1:
                    std::cout << "TASK2_1";
                    break;
                case SimulationMode::TASK2_2:
                    std::cout << "TASK2_2";
                    break;
                case SimulationMode::TASK2_3:
                    std::cout << "TASK2_3";
                    break;
            }
            std::cout << std::endl;
            
            // 更新后端适配器
            m_backendAdapter->setSimulationMode(mode);
            m_currentMode = mode;
            m_modeChanged = true; });

        std::cout << "Mode Button Test initialized successfully!" << std::endl;
        std::cout << "Click the mode buttons (Task1, Task2.1, Task2.2, Task2.3) to test mode switching." << std::endl;
        return true;
    }

    void run()
    {
        sf::Clock clock;

        while (m_window.isOpen())
        {
            handleEvents();
            update(clock.restart().asSeconds());
            render();
        }
    }

private:
    void handleEvents()
    {
        sf::Event event;
        while (m_window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                m_window.close();
            }

            // 处理工具栏事件
            sf::Vector2f mousePos = sf::Vector2f(sf::Mouse::getPosition(m_window));
            if (mousePos.y < 50.0f)
            { // 工具栏区域
                m_toolbar->handleEvent(event, mousePos);
            }

            // 键盘快捷键测试
            if (event.type == sf::Event::KeyPressed)
            {
                switch (event.key.code)
                {
                case sf::Keyboard::Num1:
                    m_toolbar->setCurrentMode(SimulationMode::TASK1);
                    std::cout << "Keyboard: Switched to TASK1" << std::endl;
                    break;
                case sf::Keyboard::Num2:
                    m_toolbar->setCurrentMode(SimulationMode::TASK2_1);
                    std::cout << "Keyboard: Switched to TASK2_1" << std::endl;
                    break;
                case sf::Keyboard::Num3:
                    m_toolbar->setCurrentMode(SimulationMode::TASK2_2);
                    std::cout << "Keyboard: Switched to TASK2_2" << std::endl;
                    break;
                case sf::Keyboard::Num4:
                    m_toolbar->setCurrentMode(SimulationMode::TASK2_3);
                    std::cout << "Keyboard: Switched to TASK2_3" << std::endl;
                    break;
                case sf::Keyboard::Escape:
                    m_window.close();
                    break;
                }
            }
        }
    }

    void update(float deltaTime)
    {
        // 更新工具栏时间显示
        static float totalTime = 0.0f;
        totalTime += deltaTime;
        m_toolbar->updateTimeDisplay(totalTime);

        // 验证后端适配器状态
        if (m_modeChanged)
        {
            SimulationMode backendMode = m_backendAdapter->getCurrentSimulationMode();
            if (backendMode == m_currentMode)
            {
                std::cout << "✓ Backend adapter mode sync successful!" << std::endl;
            }
            else
            {
                std::cout << "✗ Backend adapter mode sync failed!" << std::endl;
            }
            m_modeChanged = false;
        }
    }

    void render()
    {
        m_window.clear(sf::Color(30, 30, 30));

        // 绘制工具栏
        m_toolbar->render(m_window, sf::Vector2f(0, 0));

        // 绘制说明文字
        sf::Text instructionText;
        instructionText.setFont(m_font);
        instructionText.setCharacterSize(16);
        instructionText.setFillColor(sf::Color::White);
        instructionText.setPosition(50, 100);
        instructionText.setString("Mode Button Test\n\nInstructions:\n"
                                  "• Click mode buttons in toolbar to switch modes\n"
                                  "• Use number keys 1-4 for keyboard shortcuts\n"
                                  "• Press ESC to exit\n"
                                  "• Watch console output for mode change confirmations");
        m_window.draw(instructionText);

        // 显示当前模式
        sf::Text currentModeText;
        currentModeText.setFont(m_font);
        currentModeText.setCharacterSize(20);
        currentModeText.setFillColor(sf::Color::Green);
        currentModeText.setPosition(50, 300);

        std::string modeStr = "Current Mode: ";
        switch (m_currentMode)
        {
        case SimulationMode::TASK1:
            modeStr += "TASK1";
            break;
        case SimulationMode::TASK2_1:
            modeStr += "TASK2.1";
            break;
        case SimulationMode::TASK2_2:
            modeStr += "TASK2.2";
            break;
        case SimulationMode::TASK2_3:
            modeStr += "TASK2.3";
            break;
        }
        currentModeText.setString(modeStr);
        m_window.draw(currentModeText);

        m_window.display();
    }
};

int main()
{
    ModeButtonTest test;

    if (!test.initialize())
    {
        std::cerr << "Failed to initialize test!" << std::endl;
        return -1;
    }

    test.run();
    return 0;
}
