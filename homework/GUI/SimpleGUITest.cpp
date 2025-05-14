#include <SFML/Graphics.hpp>
#include <iostream>
#include <memory>
#include <cmath>  // 添加数学函数支持

// 定义PI常量，如果不存在
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// 简单的测试版本, 仅用于验证GUI集成问题修复

int main()
{
    try
    {
        // 创建窗口
        sf::RenderWindow window(sf::VideoMode(800, 600), "仓储穿梭车仿真系统 - GUI集成测试");
        window.setFramerateLimit(60);
        
        // 创建一个简单的圆形代表车辆
        sf::CircleShape vehicle(20.0f);
        vehicle.setFillColor(sf::Color(100, 150, 250));
        vehicle.setOrigin(20.0f, 20.0f);
        vehicle.setPosition(400.0f, 300.0f);
        
        // 创建轨道形状
        sf::VertexArray track(sf::LineStrip, 37);  // 36个点构成一个圆
        float radius = 200.0f;
        for (int i = 0; i < 37; i++) {
            float angle = i * 10.0f * M_PI / 180.0f;  // 每10度一个点
            float x = 400.0f + radius * std::cos(angle);
            float y = 300.0f + radius * std::sin(angle);
            track[i].position = sf::Vector2f(x, y);
            track[i].color = sf::Color(180, 180, 180);
        }
        
        // 主循环
        float angle = 0.0f;
        sf::Clock clock;
        
        while (window.isOpen())
        {
            // 处理事件
            sf::Event event;
            while (window.pollEvent(event))
            {
                if (event.type == sf::Event::Closed)
                    window.close();
            }
            
            // 更新车辆位置
            float deltaTime = clock.restart().asSeconds();
            angle += deltaTime * 0.5f;  // 半径/秒的速度
            if (angle > 2.0f * M_PI) {
                angle -= 2.0f * M_PI;
            }
            
            vehicle.setPosition(
                400.0f + radius * std::cos(angle),
                300.0f + radius * std::sin(angle)
            );
            
            // 清屏并绘制
            window.clear(sf::Color(50, 50, 50));
            window.draw(track);
            window.draw(vehicle);
            window.display();
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "发生异常: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cerr << "发生未知异常!" << std::endl;
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}
