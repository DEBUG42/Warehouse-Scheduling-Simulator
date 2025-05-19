#include <SFML/Graphics.hpp>
#include <iostream>
#include <memory>
#include "MockSimulationInterface.hpp"
#include "SimulationView.hpp"
#include "MainWindow.hpp"

/**
 * @brief 物流仓库穿梭车仿真系统入口点
 * 
 * 初始化仿真系统，创建模拟接口和主窗口
 * 运行主循环直至用户关闭窗口
 */
int main()
{    try {
        // 创建MockSimulationInterface用于模拟后端数据
        auto mockInterface = std::make_shared<MockSimulationInterface>(6);
        
        // 创建主窗口
        sf::RenderWindow window(sf::VideoMode(1280, 720), "物流穿梭车仿真系统", sf::Style::Default);
        window.setFramerateLimit(60);  // 设置帧率上限为60fps
          // 创建仿真视图
        SimulationView simView;
        
        // 加载字体
        sf::Font font;
        if (!font.loadFromFile("resources/fonts/simhei.ttf")) {
            if (!font.loadFromFile("GUI/resources/fonts/simhei.ttf")) {
                if (!font.loadFromFile("resources/fonts/arial.ttf")) {
                    std::cerr << "警告：无法加载字体文件！" << std::endl;
                }
            }
        }
        
        simView.initialize(font, mockInterface);
        simView.updateViewport(sf::FloatRect(0, 0, 1, 1));
        
        // 主循环
        sf::Clock clock;
        while (window.isOpen()) {
            // 计算帧时间
            float deltaTime = clock.restart().asSeconds();
            
            // 处理事件
            sf::Event event;
            while (window.pollEvent(event)) {
                switch (event.type) {
                    case sf::Event::Closed:
                        window.close();
                        break;
                        
                    case sf::Event::Resized:
                        simView.resize(event.size.width, event.size.height);
                        break;
                          case sf::Event::MouseButtonPressed:
                    case sf::Event::MouseButtonReleased:
                    case sf::Event::MouseMoved:
                    case sf::Event::MouseWheelScrolled:
                        {
                            // 获取当前鼠标位置
                            sf::Vector2f mousePos;
                            if (event.type == sf::Event::MouseButtonPressed || 
                                event.type == sf::Event::MouseButtonReleased) {
                                mousePos = sf::Vector2f(event.mouseButton.x, event.mouseButton.y);
                            } else if (event.type == sf::Event::MouseMoved) {
                                mousePos = sf::Vector2f(event.mouseMove.x, event.mouseMove.y);
                            } else if (event.type == sf::Event::MouseWheelScrolled) {
                                mousePos = sf::Vector2f(event.mouseWheelScroll.x, event.mouseWheelScroll.y);
                            }
                            
                            // 处理视图相关事件
                            simView.handleViewEvent(event, mousePos);
                        }
                        break;
                        
                    case sf::Event::KeyPressed:
                        // 处理按键事件
                        if (event.key.code == sf::Keyboard::Escape) {
                            window.close();
                        }
                        break;
                        
                    default:
                        break;
                }
            }
              // 更新逻辑
            simView.updateViewTransforms(deltaTime);
            
            // 清屏并绘制
            window.clear(sf::Color(30, 40, 50));  // 深蓝色背景
            simView.renderWorld(window);
            
            // 显示帧率（调试用）
            static sf::Clock fpsTimer;
            static int frameCount = 0;
            static float fps = 0;
            
            frameCount++;
            if (fpsTimer.getElapsedTime().asSeconds() >= 1.0f) {
                fps = frameCount / fpsTimer.getElapsedTime().asSeconds();
                frameCount = 0;
                fpsTimer.restart();
            }
            
            sf::Text fpsText;
            fpsText.setString("FPS: " + std::to_string((int)fps));
            fpsText.setPosition(10, 10);
            fpsText.setCharacterSize(14);
            window.draw(fpsText);
            
            window.display();
        }
        
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "程序异常: " << e.what() << std::endl;
        return 1;
    }
}
