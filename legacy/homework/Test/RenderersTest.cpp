/**
 * @file RenderersTest.cpp
 * @brief 测试TrackRenderer和VehicleRenderer类的功能实现
 *
 * 依赖树：
 * - RenderersTest.cpp
 *   |-- SFML/Graphics.hpp (外部库)
 *   |-- ../GUI/TrackRenderer.hpp
 *   |   |-- SFML/Graphics.hpp
 *   |
 *   |-- ../GUI/VehicleRenderer.hpp
 *       |-- SFML/Graphics.hpp
 *       |-- ../GUI/SimObject.hpp (仅用于VehicleState结构定义)
 *
 * 注意：为了简化测试，本文件直接定义了VehicleState结构，避免引入完整的SimObject.hpp
 */

#include <SFML/Graphics.hpp>
#include "../GUI/TrackRenderer.hpp"
#include <iostream>
#include <cmath>
#include <vector>

// 简化的VehicleState结构，仅包含测试所需字段
struct VehicleState
{
    int id;
    float position;    // 轨道位置 (0-1)
    float speed;       // 当前速度
    bool isLoaded;     // 载货状态
    int currentTaskId; // 当前任务ID (-1表示无任务)
};

// 简化的VehicleRenderer类，仅用于测试
class VehicleRenderer
{
private:
    // 单位转换常量
    static constexpr float MM_TO_PIXEL = 0.05f; // 毫米到像素的转换因子 (1mm = 0.05px)

    // 车辆尺寸（像素）
    const sf::Vector2f m_baseSize{
        2000.0f * MM_TO_PIXEL, // 车长：2000mm = 100像素
        800.0f * MM_TO_PIXEL   // 车宽：800mm = 40像素
    };

    sf::Color m_colorEmpty{80, 130, 200};    // 空载状态颜色
    sf::Color m_colorLoaded{200, 90, 40};    // 载货状态颜色
    sf::Color m_colorAssigned{140, 80, 160}; // 已分配任务状态颜色

public:
    /**
     * @brief 绘制车辆函数
     * @param target 渲染目标
     * @param vehicle 车辆状态
     * @param trackRadius 轨道半径（毫米）
     */
    void draw(sf::RenderTarget &target, const VehicleState &vehicle, float trackRadius)
    {
        // 计算需要的轨道参数（所有值采用毫米单位，内部转换为像素）
        float curveRadius = trackRadius; // 2500mm
        float straightLength = 40000.0f; // 直道长度，40000mm

        // 轨道的总长度（毫米）
        float totalTrackLength = 2 * straightLength + 2 * M_PI * curveRadius;

        // 车辆在轨道上的实际位置（毫米）
        float distanceOnTrack = vehicle.position * totalTrackLength;

        // 车辆位置和朝向
        sf::Vector2f vehiclePos;
        float rotation = 0.0f;

        // 确定车辆在轨道上的具体位置
        // 注意：所有坐标计算都采用毫米，最后转换为像素

        // 直道和弯道的分界点
        float p1 = straightLength;                              // 上直道结束点
        float p2 = straightLength + M_PI * curveRadius;         // 右弯道结束点
        float p3 = 2 * straightLength + M_PI * curveRadius;     // 下直道结束点
        float p4 = 2 * straightLength + 2 * M_PI * curveRadius; // 左弯道结束点（等同于轨道总长）

        if (distanceOnTrack < p1)
        {
            // 上直道
            float xPos = -straightLength / 2 + distanceOnTrack;
            vehiclePos = sf::Vector2f(
                xPos * MM_TO_PIXEL,
                -curveRadius * MM_TO_PIXEL);
            rotation = 0.0f;
        }
        else if (distanceOnTrack < p2)
        {
            // 右弯道
            float angle = (distanceOnTrack - p1) / curveRadius;
            vehiclePos = sf::Vector2f(
                (straightLength / 2 + curveRadius * cos(angle - M_PI / 2)) * MM_TO_PIXEL,
                (-curveRadius + curveRadius * sin(angle - M_PI / 2)) * MM_TO_PIXEL);
            rotation = angle * 180.0f / M_PI;
        }
        else if (distanceOnTrack < p3)
        {
            // 下直道
            float xPos = straightLength / 2 - (distanceOnTrack - p2);
            vehiclePos = sf::Vector2f(
                xPos * MM_TO_PIXEL,
                curveRadius * MM_TO_PIXEL);
            rotation = 180.0f;
        }
        else
        {
            // 左弯道
            float angle = (distanceOnTrack - p3) / curveRadius + M_PI;
            vehiclePos = sf::Vector2f(
                (-straightLength / 2 + curveRadius * cos(angle - M_PI / 2)) * MM_TO_PIXEL,
                (curveRadius + curveRadius * sin(angle - M_PI / 2)) * MM_TO_PIXEL);
            rotation = angle * 180.0f / M_PI;
        }

        // 创建车辆形状
        sf::RectangleShape vehicleShape(m_baseSize);
        vehicleShape.setOrigin(m_baseSize.x / 2, m_baseSize.y / 2);
        vehicleShape.setPosition(vehiclePos);
        vehicleShape.setRotation(rotation);

        // 设置颜色
        if (vehicle.currentTaskId >= 0)
        {
            vehicleShape.setFillColor(m_colorAssigned);
        }
        else if (vehicle.isLoaded)
        {
            vehicleShape.setFillColor(m_colorLoaded);
        }
        else
        {
            vehicleShape.setFillColor(m_colorEmpty);
        }

        // 绘制车辆
        target.draw(vehicleShape);

        // 绘制ID标签
        sf::Text idText;
        static sf::Font font;

        // 尝试加载字体(实际应用中应在初始化时加载一次)
        static bool fontLoaded = false;
        if (!fontLoaded)
        {
            if (font.loadFromFile("resources/fonts/arial.ttf") ||
                font.loadFromFile("arial.ttf") ||
                font.loadFromFile("C:/Windows/Fonts/arial.ttf"))
            {
                fontLoaded = true;
            }
        }

        if (fontLoaded)
        {
            idText.setFont(font);
            idText.setString(std::to_string(vehicle.id));
            idText.setCharacterSize(12);
            idText.setFillColor(sf::Color::White);

            sf::FloatRect textRect = idText.getLocalBounds();
            idText.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
            idText.setPosition(vehiclePos);
            idText.setRotation(rotation);

            target.draw(idText);
        }
    }
};

int main()
{
    // 创建窗口
    sf::RenderWindow window(sf::VideoMode(1280, 720), "轨道与车辆渲染测试");
    window.setFramerateLimit(60);

    // 创建视图，使原点在窗口中心
    sf::View view = window.getDefaultView();
    view.setCenter(0, 0);

    // 轨道参数（毫米）
    const float trackRadius = 2500.0f;
    const float straightLength = 40000.0f;

    // 计算轨道总宽度（毫米）以设置合适的视图缩放
    float trackTotalWidth = straightLength + 2 * trackRadius; // 约45000毫米
    float trackTotalHeight = 2 * trackRadius;                 // 约5000毫米

    // 设置缩放使整个轨道可见
    // 窗口尺寸 1280x720 像素，留一些边距
    float scaleFactorX = 1280 * 0.9f / (trackTotalWidth * 0.05f); // 0.05是毫米到像素的转换
    float scaleFactorY = 720 * 0.9f / (trackTotalHeight * 0.05f);
    float zoomFactor = std::min(scaleFactorX, scaleFactorY);

    view.zoom(1.0f / zoomFactor); // 缩放视图以适应轨道

    // 创建轨道渲染器
    TrackRenderer trackRenderer;
    trackRenderer.generateGeometry(95000.0f, 2500.0f); // 初始化轨道几何（95米总长，2.5米弯道半径）

    // 创建车辆渲染器
    VehicleRenderer vehicleRenderer;

    // 创建测试车辆
    std::vector<VehicleState> vehicles = {
        {1, 0.0f, 3.0f, false, -1},  // 空载车辆
        {2, 0.25f, 2.5f, true, -1},  // 载货车辆
        {3, 0.5f, 2.0f, false, 101}, // 已分配任务车辆
        {4, 0.75f, 1.5f, true, 102}  // 载货且已分配任务
    };

    // 模拟车辆移动的速度因子
    float simulationSpeed = 0.0001f;

    // 运行游戏循环
    sf::Clock clock;
    while (window.isOpen())
    {
        sf::Time deltaTime = clock.restart();

        // 处理事件
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
            else if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::Escape)
                {
                    window.close();
                }
                else if (event.key.code == sf::Keyboard::Equal || event.key.code == sf::Keyboard::Add)
                {
                    // 放大
                    view.zoom(0.9f);
                }
                else if (event.key.code == sf::Keyboard::Hyphen || event.key.code == sf::Keyboard::Subtract)
                {
                    // 缩小
                    view.zoom(1.1f);
                }
                else if (event.key.code == sf::Keyboard::Up)
                {
                    // 加快仿真速度
                    simulationSpeed *= 1.5f;
                }
                else if (event.key.code == sf::Keyboard::Down)
                {
                    // 减慢仿真速度
                    simulationSpeed /= 1.5f;
                }
                else if (event.key.code == sf::Keyboard::Space)
                {
                    // 暂停/继续仿真
                    simulationSpeed = (std::abs(simulationSpeed) < 1e-6) ? 0.0001f : 0.0f;
                }
            }
            else if (event.type == sf::Event::MouseWheelScrolled)
            {
                // 鼠标滚轮缩放
                if (event.mouseWheelScroll.delta > 0)
                {
                    view.zoom(0.9f);
                }
                else
                {
                    view.zoom(1.1f);
                }
            }
        }

        // 处理键盘输入来平移视图
        float moveSpeed = 10.0f / zoomFactor; // 根据当前缩放调整移动速度
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
        {
            view.move({-moveSpeed, 0});
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
        {
            view.move({moveSpeed, 0});
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
        {
            view.move({0, -moveSpeed});
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
        {
            view.move({0, moveSpeed});
        }

        // 更新车辆位置
        for (auto &vehicle : vehicles)
        {
            vehicle.position += vehicle.speed * deltaTime.asSeconds() * simulationSpeed;
            if (vehicle.position >= 1.0f)
            {
                vehicle.position -= 1.0f;
            }
        }

        // 清空窗口
        window.clear(sf::Color(40, 40, 40));

        // 应用视图
        window.setView(view);

        // 绘制轨道
        window.draw(trackRenderer);

        // 绘制车辆
        for (const auto &vehicle : vehicles)
        {
            vehicleRenderer.draw(window, vehicle, trackRadius);
        }

        // 显示帮助信息（固定在屏幕上）
        window.setView(window.getDefaultView());

        sf::Font font;
        if (font.loadFromFile("resources/fonts/arial.ttf") ||
            font.loadFromFile("arial.ttf") ||
            font.loadFromFile("C:/Windows/Fonts/arial.ttf"))
        {

            sf::Text helpText;
            helpText.setFont(font);
            helpText.setCharacterSize(14);
            helpText.setFillColor(sf::Color::White);
            helpText.setPosition(10, 10);

            std::string helpString =
                "操作指南:\n"
                "WASD/方向键: 移动视图\n"
                "鼠标滚轮/+/-: 缩放\n"
                "上下方向键: 调整仿真速度\n"
                "空格键: 暂停/继续仿真\n"
                "ESC: 退出\n\n"
                "轨道参数:\n"
                "  直道长度: 40000mm (40m)\n"
                "  弯道半径: 2500mm (2.5m)\n"
                "  轨道宽度: 1200mm (1.2m)\n"
                "  总长度: 约95000mm (95m)\n"
                "车辆参数:\n"
                "  长度: 2000mm (2m)\n"
                "  宽度: 800mm (0.8m)\n\n"
                "当前仿真速度: " +
                std::to_string(simulationSpeed);

            helpText.setString(helpString);
            window.draw(helpText);
        }

        // 更新显示
        window.display();
    }

    return 0;
}