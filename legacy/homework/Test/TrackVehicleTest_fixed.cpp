/**
 * @file TrackVehicleTest_fixed.cpp
 * @brief 专门测试轨道渲染和车辆运动的测试文件
 *
 * 本测试文件验证以下内容:
 * 1. 轨道正确渲染：直道长度、弯道半径、轨道宽度都符合物理参数要求
 * 2. 轨道连接正确：直道和弯道平滑过渡，无间隙
 * 3. 车辆在轨道上运动正确：遵循轨道中心线运动，转向角度准确
 * 4. 尺寸比例正确：车辆与轨道尺寸符合真实比例(1:1)
 *
 * 依赖树:
 * - TrackVehicleTest.cpp
 *   |-- SFML/Graphics.hpp (外部库)
 *   |-- ../GUI/TrackRenderer_fixed.hpp
 *      |-- SFML/Graphics.hpp
 */

// 空格：暂停/继续模拟
// 上/下方向键：调整模拟速度
// +/-：缩放视图
// WASD/方向键：平移视图
// G键：切换网格显示
// T键：切换内外轨道显示

#include <SFML/Graphics.hpp>
#include "../GUI/TrackRenderer_fixed.hpp"
#include <iostream>
#include <cmath>
#include <vector>
#include <sstream>
#include <iomanip>

// 使用TrackRenderer中定义的单位转换常量
// 单位换算: 1mm = 0.05px (轨道渲染器内部使用的转换系数)
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// 物理参数定义(mm)
const float TRACK_RADIUS = 2500.0f;                                             // 弯道半径(mm)
const float STRAIGHT_LENGTH = 40000.0f;                                         // 单段直道长度(mm)
const float TRACK_WIDTH = 1200.0f;                                              // 轨道宽度(mm)
const float VEHICLE_LENGTH = 2000.0f;                                           // 车辆长度(mm)
const float VEHICLE_WIDTH = 800.0f;                                             // 车辆宽度(mm)
const float TRACK_TOTAL_LENGTH = 2 * STRAIGHT_LENGTH + 2 * M_PI * TRACK_RADIUS; // 轨道总长(mm)

// 简化的车辆状态结构
struct VehicleState
{
    int id;         // 车辆ID
    float position; // 轨道位置(0-1)
    float speed;    // 速度(mm/s)
    bool isLoaded;  // 载货状态
    int taskId;     // 任务ID

    // 渲染属性
    sf::Color color; // 车辆颜色
    sf::Text label;  // 车辆标签
};

// 获取车辆在轨道上的坐标和角度
std::pair<sf::Vector2f, float> getVehiclePositionAndRotation(float normalizedPosition)
{
    // 将归一化位置(0-1)转换为实际轨道位置(mm)
    float pos = normalizedPosition * TRACK_TOTAL_LENGTH;

    // 计算在轨道上的位置和角度
    sf::Vector2f position;
    float rotation = 0.0f;

    // 各段的分界点(mm)
    float p1 = STRAIGHT_LENGTH;                           // 上直道结束点
    float p2 = STRAIGHT_LENGTH + M_PI * TRACK_RADIUS;     // 右弯道结束点
    float p3 = 2 * STRAIGHT_LENGTH + M_PI * TRACK_RADIUS; // 下直道结束点

    // 确定车辆在轨道的哪一段上
    if (pos < p1)
    {
        // 上直道
        float xPos = -STRAIGHT_LENGTH / 2 + pos;
        position = sf::Vector2f(
            xPos * TrackRenderer::MM_TO_PIXEL,
            -TRACK_RADIUS * TrackRenderer::MM_TO_PIXEL);
        rotation = 0.0f; // 向右
    }
    else if (pos < p2)
    {
        // 右弯道
        float angle = (pos - p1) / TRACK_RADIUS;
        position = sf::Vector2f(
            (STRAIGHT_LENGTH / 2 + TRACK_RADIUS * std::cos(angle - M_PI / 2)) * TrackRenderer::MM_TO_PIXEL,
            (-TRACK_RADIUS + TRACK_RADIUS * std::sin(angle - M_PI / 2)) * TrackRenderer::MM_TO_PIXEL);
        rotation = angle * 180.0f / M_PI; // 角度转换为度
    }
    else if (pos < p3)
    {
        // 下直道
        float xPos = STRAIGHT_LENGTH / 2 - (pos - p2);
        position = sf::Vector2f(
            xPos * TrackRenderer::MM_TO_PIXEL,
            TRACK_RADIUS * TrackRenderer::MM_TO_PIXEL);
        rotation = 180.0f; // 向左
    }
    else
    {
        // 左弯道
        float angle = (pos - p3) / TRACK_RADIUS + M_PI;
        position = sf::Vector2f(
            (-STRAIGHT_LENGTH / 2 + TRACK_RADIUS * std::cos(angle - M_PI / 2)) * TrackRenderer::MM_TO_PIXEL,
            (TRACK_RADIUS + TRACK_RADIUS * std::sin(angle - M_PI / 2)) * TrackRenderer::MM_TO_PIXEL);
        rotation = angle * 180.0f / M_PI; // 角度转换为度
    }

    return {position, rotation};
}

// 绘制车辆
void drawVehicle(sf::RenderTarget &target, VehicleState &vehicle, sf::Font &font)
{
    // 获取车辆的位置和朝向
    auto [position, rotation] = getVehiclePositionAndRotation(vehicle.position);

    // 创建车辆形状
    sf::RectangleShape vehicleShape(sf::Vector2f(
        VEHICLE_LENGTH * TrackRenderer::MM_TO_PIXEL,
        VEHICLE_WIDTH * TrackRenderer::MM_TO_PIXEL));

    // 设置车辆属性
    vehicleShape.setOrigin(VEHICLE_LENGTH * TrackRenderer::MM_TO_PIXEL / 2, VEHICLE_WIDTH * TrackRenderer::MM_TO_PIXEL / 2);
    vehicleShape.setPosition(position);
    vehicleShape.setRotation(rotation);
    vehicleShape.setFillColor(vehicle.color);
    vehicleShape.setOutlineColor(sf::Color(30, 30, 30));
    vehicleShape.setOutlineThickness(1.0f);

    // 绘制车辆
    target.draw(vehicleShape);

    // 设置车辆ID标签
    vehicle.label.setFont(font);
    vehicle.label.setString(std::to_string(vehicle.id));
    vehicle.label.setCharacterSize(12);
    vehicle.label.setFillColor(sf::Color::White);

    // 居中标签
    sf::FloatRect textBounds = vehicle.label.getLocalBounds();
    vehicle.label.setOrigin(textBounds.width / 2.0f, textBounds.height / 2.0f);

    // 设置标签位置和旋转
    vehicle.label.setPosition(position);
    vehicle.label.setRotation(rotation);

    // 绘制标签
    target.draw(vehicle.label);
}

// 绘制尺寸标注
void drawDimensionLabels(sf::RenderTarget &target, sf::Font &font)
{
    sf::Text label;
    label.setFont(font);
    label.setCharacterSize(14);
    label.setFillColor(sf::Color::Yellow);

    // 标注直道长度
    label.setString("40000mm (40m)");
    label.setPosition(0, (-TRACK_RADIUS - 800) * TrackRenderer::MM_TO_PIXEL);
    label.setRotation(0);
    target.draw(label);

    // 绘制直道长度线
    sf::Vertex straightLine[] = {
        sf::Vertex(sf::Vector2f(-STRAIGHT_LENGTH / 2 * TrackRenderer::MM_TO_PIXEL, (-TRACK_RADIUS - 500) * TrackRenderer::MM_TO_PIXEL), sf::Color::Yellow),
        sf::Vertex(sf::Vector2f(STRAIGHT_LENGTH / 2 * TrackRenderer::MM_TO_PIXEL, (-TRACK_RADIUS - 500) * TrackRenderer::MM_TO_PIXEL), sf::Color::Yellow)};
    target.draw(straightLine, 2, sf::Lines);

    // 标注弯道半径
    label.setString("R=2500mm (2.5m)");
    label.setPosition((STRAIGHT_LENGTH / 2 + TRACK_RADIUS + 500) * TrackRenderer::MM_TO_PIXEL, 0);
    label.setRotation(0);
    target.draw(label);

    // 绘制半径线
    sf::Vertex radiusLine[] = {
        sf::Vertex(sf::Vector2f(STRAIGHT_LENGTH / 2 * TrackRenderer::MM_TO_PIXEL, 0), sf::Color::Yellow),
        sf::Vertex(sf::Vector2f((STRAIGHT_LENGTH / 2 + TRACK_RADIUS) * TrackRenderer::MM_TO_PIXEL, 0), sf::Color::Yellow)};
    target.draw(radiusLine, 2, sf::Lines);

    // 标注轨道宽度
    label.setString("1200mm (1.2m)");
    label.setPosition(-STRAIGHT_LENGTH / 4 * TrackRenderer::MM_TO_PIXEL, (-TRACK_RADIUS - 2000) * TrackRenderer::MM_TO_PIXEL);
    target.draw(label);
}

// 绘制参考网格
void drawGrid(sf::RenderTarget &target, float gridSize = 5000.0f)
{
    // 网格范围
    float maxExtent = std::max(
                          STRAIGHT_LENGTH + 2 * TRACK_RADIUS,
                          2 * TRACK_RADIUS) *
                      1.2f;

    // 创建网格线
    std::vector<sf::Vertex> gridLines;
    sf::Color gridColor(80, 80, 80, 100);

    // 水平线
    for (float y = -maxExtent; y <= maxExtent; y += gridSize)
    {
        gridLines.push_back(sf::Vertex(sf::Vector2f(-maxExtent * TrackRenderer::MM_TO_PIXEL, y * TrackRenderer::MM_TO_PIXEL), gridColor));
        gridLines.push_back(sf::Vertex(sf::Vector2f(maxExtent * TrackRenderer::MM_TO_PIXEL, y * TrackRenderer::MM_TO_PIXEL), gridColor));
    }

    // 垂直线
    for (float x = -maxExtent; x <= maxExtent; x += gridSize)
    {
        gridLines.push_back(sf::Vertex(sf::Vector2f(x * TrackRenderer::MM_TO_PIXEL, -maxExtent * TrackRenderer::MM_TO_PIXEL), gridColor));
        gridLines.push_back(sf::Vertex(sf::Vector2f(x * TrackRenderer::MM_TO_PIXEL, maxExtent * TrackRenderer::MM_TO_PIXEL), gridColor));
    }

    // 绘制网格线
    target.draw(gridLines.data(), gridLines.size(), sf::Lines);

    // 绘制坐标轴
    sf::Vertex xAxis[] = {
        sf::Vertex(sf::Vector2f(-maxExtent * TrackRenderer::MM_TO_PIXEL, 0), sf::Color(255, 50, 50, 150)),
        sf::Vertex(sf::Vector2f(maxExtent * TrackRenderer::MM_TO_PIXEL, 0), sf::Color(255, 50, 50, 150))};
    sf::Vertex yAxis[] = {
        sf::Vertex(sf::Vector2f(0, -maxExtent * TrackRenderer::MM_TO_PIXEL), sf::Color(50, 255, 50, 150)),
        sf::Vertex(sf::Vector2f(0, maxExtent * TrackRenderer::MM_TO_PIXEL), sf::Color(50, 255, 50, 150))};

    target.draw(xAxis, 2, sf::Lines);
    target.draw(yAxis, 2, sf::Lines);
}

// 显示工具栏
void drawToolbar(sf::RenderWindow &window, sf::Font &font, float simulationSpeed, bool showGrid, bool showInnerOuterTracks)
{
    // 背景
    sf::RectangleShape toolbarBg(sf::Vector2f(window.getSize().x, 40));
    toolbarBg.setFillColor(sf::Color(40, 40, 40, 200));

    // 文字
    sf::Text toolbarText;
    toolbarText.setFont(font);
    toolbarText.setCharacterSize(14);
    toolbarText.setFillColor(sf::Color::White);
    std::stringstream ss;
    ss << "模拟速度: " << std::fixed << std::setprecision(5) << simulationSpeed
       << " | G: " << (showGrid ? "显示" : "隐藏") << "网格"
       << " | T: " << (showInnerOuterTracks ? "完整轨道" : "仅中心线")
       << " | 空格: 暂停/继续 | +/-: 缩放 | 方向键: 移动视图";

    toolbarText.setString(ss.str());
    toolbarText.setPosition(10, 10);

    // 切换到默认视图来绘制工具栏
    sf::View defaultView = window.getDefaultView();
    sf::View currentView = window.getView();

    window.setView(defaultView);
    window.draw(toolbarBg);
    window.draw(toolbarText);
    window.setView(currentView);
}

int main()
{
    // 创建窗口
    sf::RenderWindow window(sf::VideoMode(1280, 720), "轨道与车辆渲染测试");
    window.setFramerateLimit(60);

    // 创建字体
    sf::Font font;
    if (!font.loadFromFile("../../homework/GUI/resources/fonts/arial.ttf") &&
        !font.loadFromFile("../GUI/resources/fonts/arial.ttf") &&
        !font.loadFromFile("C:/Windows/Fonts/arial.ttf"))
    {
        std::cerr << "无法加载字体！" << std::endl;
        return -1;
    }

    // 创建视图，设置窗口原点在窗口中心
    sf::View view = window.getDefaultView();
    view.setCenter(0, 0);

    // 设置初始缩放使整个轨道可见
    float viewportWidthInPixels = window.getSize().x;
    float viewportHeightInPixels = window.getSize().y;
    float trackWidthInPixels = (STRAIGHT_LENGTH + 2 * TRACK_RADIUS) * TrackRenderer::MM_TO_PIXEL;
    float trackHeightInPixels = 2 * (TRACK_RADIUS + TRACK_WIDTH / 2) * TrackRenderer::MM_TO_PIXEL * 1.2f; // 添加20%边距

    float scaleX = viewportWidthInPixels / trackWidthInPixels;
    float scaleY = viewportHeightInPixels / trackHeightInPixels;
    float scale = std::min(scaleX, scaleY) * 0.9f; // 再给10%边距

    view.zoom(1.0f / scale);

    // 创建轨道渲染器
    TrackRenderer trackRenderer;
    // 设置轨道宽度为1200mm（与常量TRACK_WIDTH相同）
    trackRenderer.setTrackWidth(TRACK_WIDTH);
    // 生成轨道几何形状
    trackRenderer.generateGeometry(TRACK_TOTAL_LENGTH, TRACK_RADIUS);

    // 创建测试车辆
    std::vector<VehicleState> vehicles = {
        {1, 0.00f, 5.0f, false, -1, sf::Color(80, 130, 200)},  // 空载
        {2, 0.15f, 7.0f, true, -1, sf::Color(200, 90, 40)},    // 载货
        {3, 0.40f, 4.0f, false, 101, sf::Color(140, 80, 160)}, // 分配任务
        {4, 0.65f, 6.0f, true, 102, sf::Color(180, 120, 40)},  // 载货+任务
        {5, 0.85f, 3.0f, false, -1, sf::Color(60, 160, 160)}   // 空载
    };

    // 状态变量
    float simulationSpeed = 0.00005f; // 模拟速度(较慢，方便观察)
    bool isPaused = false;            // 是否暂停
    bool showGrid = true;             // 是否显示网格
    bool showInnerOuterTracks = true; // 是否显示内外轨道

    // 游戏循环
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
                else if (event.key.code == sf::Keyboard::Equal ||
                         event.key.code == sf::Keyboard::Add)
                {
                    // 放大
                    view.zoom(0.9f);
                }
                else if (event.key.code == sf::Keyboard::Hyphen ||
                         event.key.code == sf::Keyboard::Subtract)
                {
                    // 缩小
                    view.zoom(1.1f);
                }
                else if (event.key.code == sf::Keyboard::Up)
                {
                    // 加速
                    if (!isPaused)
                        simulationSpeed *= 1.5f;
                }
                else if (event.key.code == sf::Keyboard::Down)
                {
                    // 减速
                    if (!isPaused)
                        simulationSpeed /= 1.5f;
                }
                else if (event.key.code == sf::Keyboard::Space)
                {
                    // 暂停/继续
                    isPaused = !isPaused;
                }
                else if (event.key.code == sf::Keyboard::G)
                {
                    // 切换网格显示
                    showGrid = !showGrid;
                }
                else if (event.key.code == sf::Keyboard::T)
                {
                    // 切换内外轨道显示
                    showInnerOuterTracks = !showInnerOuterTracks;
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

        // 键盘控制视图移动
        float moveSpeed = 10.0f / scale;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
        {
            view.move({-moveSpeed, 0});
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
        {
            view.move({moveSpeed, 0});
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
        {
            view.move({0, -moveSpeed});
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
        {
            view.move({0, moveSpeed});
        }

        // 更新车辆位置
        if (!isPaused)
        {
            for (auto &vehicle : vehicles)
            {
                vehicle.position += (vehicle.speed * deltaTime.asSeconds() * simulationSpeed) / TRACK_TOTAL_LENGTH;
                if (vehicle.position >= 1.0f)
                {
                    vehicle.position -= 1.0f;
                }
                else if (vehicle.position < 0.0f)
                {
                    vehicle.position += 1.0f;
                }
            }
        }

        // 清除窗口
        window.clear(sf::Color(30, 30, 30));

        // 设置视图
        window.setView(view);

        // 绘制背景网格
        if (showGrid)
        {
            drawGrid(window);
        }

        // 绘制轨道
        if (showInnerOuterTracks)
        {
            // 使用TrackRenderer绘制完整轨道（包括内外轨道）
            window.draw(trackRenderer);
        }
        else
        {
            // 只使用TrackRenderer绘制中心线
            trackRenderer.setCenterLineColor(sf::Color(180, 180, 180));
            window.draw(trackRenderer);
            trackRenderer.setCenterLineColor(sf::Color(180, 180, 180));
        }

        // 绘制尺寸标注
        drawDimensionLabels(window, font);

        // 绘制所有车辆
        for (auto &vehicle : vehicles)
        {
            drawVehicle(window, vehicle, font);
        }

        // 绘制工具栏
        drawToolbar(window, font, simulationSpeed, showGrid, showInnerOuterTracks);

        // 显示
        window.display();
    }

    return 0;
}
