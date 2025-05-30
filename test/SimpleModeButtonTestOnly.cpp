#include <SFML/Graphics.hpp>
#include <iostream>
#include "../include/gui/Toolbar.hpp"

int main()
{
    std::cout << "=== 简化模式按钮测试 ===" << std::endl;
    std::cout << "点击不同的模式按钮来测试模式切换功能" << std::endl;
    std::cout << "支持的模式: Task1, Task2.1, Task2.2, Task2.3" << std::endl;

    // 创建窗口
    sf::RenderWindow window(sf::VideoMode(1200, 80), "Mode Button Test - Simplified");
    window.setFramerateLimit(60);

    // 加载字体
    sf::Font font;
    if (!font.loadFromFile("assets/fonts/arial.ttf"))
    {
        std::cerr << "Failed to load font, using default font" << std::endl;
        // 继续运行，使用默认字体
    }

    // 创建工具栏
    auto toolbar = std::make_unique<Toolbar>(font, 1200.0f, 60.0f);

    // 设置模式切换回调
    toolbar->setOnModeChanged([](SimulationMode mode)
                              {
        std::string modeStr;
        switch (mode) {
            case SimulationMode::TASK1: 
                modeStr = "TASK1"; 
                break;
            case SimulationMode::TASK2_1: 
                modeStr = "TASK2.1"; 
                break;
            case SimulationMode::TASK2_2: 
                modeStr = "TASK2.2"; 
                break;
            case SimulationMode::TASK2_3: 
                modeStr = "TASK2.3"; 
                break;
        }
        std::cout << ">>> 模式切换到: " << modeStr << " (值: " << static_cast<int>(mode) << ")" << std::endl; });

    // 设置播放/暂停回调
    toolbar->setOnPlayPauseToggled([]()
                                   { std::cout << ">>> 播放/暂停按钮被点击" << std::endl; });

    // 设置速度改变回调
    toolbar->setOnTimeScaleChanged([](float scale)
                                   { std::cout << ">>> 速度改变为: " << scale << "x" << std::endl; });

    std::cout << "窗口已打开，当前模式: " << static_cast<int>(toolbar->getCurrentMode()) << std::endl;
    std::cout << "点击模式按钮进行测试..." << std::endl;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                std::cout << "窗口关闭，测试结束" << std::endl;
                window.close();
            }

            if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::Escape)
                {
                    std::cout << "ESC键按下，测试结束" << std::endl;
                    window.close();
                }
            }

            // 处理工具栏事件
            sf::Vector2f mousePos = sf::Vector2f(sf::Mouse::getPosition(window));
            if (toolbar->handleEvent(event, mousePos))
            {
                // 事件被工具栏处理，显示当前状态
                std::cout << "当前模式: " << static_cast<int>(toolbar->getCurrentMode()) << std::endl;
            }
        }

        // 渲染
        window.clear(sf::Color(50, 50, 50)); // 深灰色背景

        // 渲染工具栏
        toolbar->render(window, sf::Vector2f(0, 10)); // 稍微向下偏移

        window.display();
    }

    std::cout << "测试完成！" << std::endl;
    return 0;
}
