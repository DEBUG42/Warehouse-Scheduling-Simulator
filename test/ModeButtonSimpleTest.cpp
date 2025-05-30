#include <SFML/Graphics.hpp>
#include <iostream>
#include "../include/gui/Toolbar.hpp"

int main()
{
    // 创建窗口
    sf::RenderWindow window(sf::VideoMode(1200, 100), "Mode Button Test");
    window.setFramerateLimit(60);

    // 加载字体
    sf::Font font;
    if (!font.loadFromFile("assets/fonts/arial.ttf"))
    {
        std::cerr << "Failed to load font, using default font" << std::endl;
        // 继续运行，使用默认字体
    }

    // 创建工具栏
    auto toolbar = std::make_unique<Toolbar>(font, 1200.0f, 50.0f);

    // 设置模式切换回调
    toolbar->setOnModeChanged([](SimulationMode mode)
                              {
        std::string modeStr;
        switch (mode) {
            case SimulationMode::TASK1: modeStr = "TASK1"; break;
            case SimulationMode::TASK2_1: modeStr = "TASK2.1"; break;
            case SimulationMode::TASK2_2: modeStr = "TASK2.2"; break;
            case SimulationMode::TASK2_3: modeStr = "TASK2.3"; break;
        }
        std::cout << "Mode switched to: " << modeStr << std::endl; });

    std::cout << "Mode Button Test Started. Click the mode buttons to test mode switching." << std::endl;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }

            // 处理工具栏事件
            sf::Vector2f mousePos = sf::Vector2f(sf::Mouse::getPosition(window));
            toolbar->handleEvent(event, mousePos);
        }

        window.clear(sf::Color::Black);

        // 渲染工具栏
        toolbar->render(window, sf::Vector2f(0, 0));

        window.display();
    }

    return 0;
}
