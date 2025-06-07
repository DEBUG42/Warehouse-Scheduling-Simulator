#include <SFML/Graphics.hpp>
#include <iostream>
#include <memory>
#include <vector>
#define _USE_MATH_DEFINES
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// GUI组件包含 - 只使用不依赖复杂组件的
#include "gui/Toolbar.hpp"
#include "gui/UIControls.hpp"
#include "gui/SimulationView.hpp"

// 简化的Vehicle结构（避免复杂依赖）
struct SimpleVehicle {
    int id = 1;
    float position_m = 32.0f;
    float velocity_mps = 0.0f;
    float max_speed = 2.0f;
    bool is_loaded = false;
    bool is_executing = false;
    int towards_device = -1;
    float next_available_time = 0.0f;
    
    struct State {
        float position = 32.0f;
        float currentSpeed = 0.0f;
        enum MotionState { Stopped, Cruising, Accelerating } motionState = Stopped;
        void* currentTask = nullptr;
        float operationTimer = 0.0f;
    } m_state;
};

/**
 * @brief GUI优化第一阶段最小演示测试
 *
 * 展示内容：
 * 1. Toolbar时间显示优化 - HH:MM:SS.mmm格式
 * 2. SimulationView - 仓库渲染和可控制车辆系统
 * 3. 键盘控制和视图切换功能
 */

class MinimalDemoApp
{
private:
    sf::RenderWindow window;
    sf::Font font; 
    
    // GUI组件
    std::unique_ptr<Toolbar> toolbar;
    std::unique_ptr<SimulationView> simulationView;
    
    // 仿真数据
    std::unique_ptr<SimpleVehicle> vehicle = std::make_unique<SimpleVehicle>();

    // 仿真状态
    float simulationTime = 0.0f;
    bool isRunning = false;
    int selectedVehicleId = -1; 
    
    // 布局参数
    static constexpr float TOOLBAR_HEIGHT = 50.0f;
    static constexpr float SIMULATION_VIEW_MARGIN = 10.0f;

public:
    MinimalDemoApp() : window(sf::VideoMode(1600, 900), "GUI Phase 1 - Minimal Demo with Warehouse & Vehicles")
    {
        loadFont();
        initializeComponents();
    }

    bool loadFont()
    {
        // 尝试多个字体路径
        std::vector<std::string> fontPaths = {
            "C:/Windows/Fonts/arial.ttf",
            "C:/Windows/Fonts/calibri.ttf",
            "C:/Windows/Fonts/tahoma.ttf",
            "C:/Windows/Fonts/verdana.ttf",
            "assets/fonts/arial.ttf"
        };

        for (const auto &fontPath : fontPaths)
        {
            if (font.loadFromFile(fontPath))
            {
                std::cout << "Font loaded successfully: " << fontPath << std::endl;
                return true;
            }
        }

        std::cerr << "Failed to load any font! UI text will not display correctly." << std::endl;
        return false;
    }
    
    void initializeComponents()
    {
        // 创建工具栏
        toolbar = std::make_unique<Toolbar>(font, window.getSize().x, TOOLBAR_HEIGHT);

        // 创建仿真视图
        simulationView = std::make_unique<SimulationView>(font);

        // 初始化仿真视图
        float simulationViewWidth = window.getSize().x - SIMULATION_VIEW_MARGIN * 2;
        float simulationViewHeight = window.getSize().y - TOOLBAR_HEIGHT - SIMULATION_VIEW_MARGIN * 2;
        sf::Vector2f simulationViewSize(simulationViewWidth, simulationViewHeight);

        simulationView->initialize(font, nullptr, simulationViewSize); 
        
        // 设置显示选项
        simulationView->setShowGrid(true);                             // 显示网格
        simulationView->setShowWarehouses(true);                       // 显示仓库
        simulationView->setShowVehicles(true);                         // 显示车辆
        simulationView->setShowDebugInfo(true);                        // 显示调试信息

        // 设置工具栏回调
        toolbar->setOnPlayPauseToggled([this]()
                                       {
            isRunning = !isRunning;
            std::cout << "Simulation State: " << (isRunning ? "Running" : "Paused") << std::endl; });

        toolbar->setOnTimeScaleChanged([this](float speed)
                                       { std::cout << "Speed adjusted to: " << speed << "x" << std::endl; });
    }
      void handleEvents()
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }

            // 获取鼠标位置
            sf::Vector2i mousePixelPos = sf::Mouse::getPosition(window);
            sf::Vector2f mousePos = window.mapPixelToCoords(mousePixelPos);            // 1. 首先处理全局键盘事件（不受区域限制）
            if (event.type == sf::Event::KeyPressed)
            {
                switch (event.key.code)
                {
                case sf::Keyboard::Space:
                    isRunning = !isRunning;
                    toolbar->setPlaying(isRunning);
                    std::cout << "Simulation State: " << (isRunning ? "Running" : "Paused") << std::endl;
                    continue; // 键盘事件已处理，跳过其他处理
                case sf::Keyboard::R:
                    simulationTime = 0.0f;
                    isRunning = false;
                    toolbar->setPlaying(false);
                    resetSimulation();
                    std::cout << "Reset Simulation" << std::endl;
                    continue;
                case sf::Keyboard::Escape:
                    window.close();
                    continue;

                // 显示控制键
                case sf::Keyboard::T:
                    if (simulationView)
                    {
                        bool currentState = simulationView->getShowGrid();
                        simulationView->setShowGrid(!currentState);
                        std::cout << (currentState ? "隐藏坐标网格\n" : "显示坐标网格\n");
                    }
                    continue;
                case sf::Keyboard::G:
                    if (simulationView)
                    {
                        bool currentState = simulationView->getShowWarehouses();
                        simulationView->setShowWarehouses(!currentState);
                        std::cout << (currentState ? "隐藏仓库\n" : "显示仓库\n");
                    }
                    continue;
                case sf::Keyboard::V:
                    if (simulationView)
                    {
                        bool currentState = simulationView->getShowVehicles();
                        simulationView->setShowVehicles(!currentState);
                        std::cout << (currentState ? "隐藏车辆\n" : "显示车辆\n");
                    }
                    continue;
                case sf::Keyboard::C:
                    if (simulationView)
                    {
                        bool currentState = simulationView->getShowDebugInfo();
                        simulationView->setShowDebugInfo(!currentState);
                        std::cout << (currentState ? "隐藏调试信息\n" : "显示调试信息\n");
                    }
                    continue;

                // 车辆控制键
                case sf::Keyboard::W:
                case sf::Keyboard::A:
                case sf::Keyboard::S:
                case sf::Keyboard::D:
                    moveVehicle(event.key.code);
                    continue;
                }
            }

            // 2. 工具栏事件处理（鼠标事件）
            if (toolbar->handleEvent(event, mousePos))
            {
                continue; // 工具栏消费了事件，跳过其他处理
            }

            // 3. 仿真视图鼠标事件处理（仅处理在区域内的鼠标事件）
            if (simulationView)
            {
                // 检查鼠标是否在仿真视图区域内
                float simulationViewLeft = SIMULATION_VIEW_MARGIN;
                float simulationViewTop = TOOLBAR_HEIGHT + SIMULATION_VIEW_MARGIN;
                float simulationViewWidth = window.getSize().x - SIMULATION_VIEW_MARGIN * 2;
                float simulationViewHeight = window.getSize().y - TOOLBAR_HEIGHT - SIMULATION_VIEW_MARGIN * 2;

                bool inSimulationArea = (mousePos.x >= simulationViewLeft &&
                                         mousePos.x <= simulationViewLeft + simulationViewWidth &&
                                         mousePos.y >= simulationViewTop &&
                                         mousePos.y <= simulationViewTop + simulationViewHeight);

                // 只有鼠标事件且在仿真区域内才传递给SimulationView
                if (inSimulationArea && (event.type == sf::Event::MouseButtonPressed ||
                                        event.type == sf::Event::MouseButtonReleased ||
                                        event.type == sf::Event::MouseMoved ||
                                        event.type == sf::Event::MouseWheelScrolled))
                {
                    simulationView->handleViewEvent(event, mousePos);
                    continue;
                }
            }
        }
    }
    
    void update(float deltaTime)
    {
        if (isRunning)
        {
            // 应用倍速系数
            float timeScale = toolbar->getCurrentSpeed();
            simulationTime += deltaTime * timeScale;

            // 更新工具栏时间显示
            toolbar->updateTimeDisplay(simulationTime);
            
            // 更新仿真视图
            if (simulationView)
            {
                simulationView->updateViewTransforms(deltaTime * timeScale);
            }
        }
    }
    
    void moveVehicle(sf::Keyboard::Key key)
    {
        float currentPos = vehicle->position_m;
        float newPos = currentPos;

        const float moveSpeed = 5.0f; // 移动速度（米）

        switch (key)
        {
        case sf::Keyboard::W:
            newPos += moveSpeed; // 向前移动
            break;
        case sf::Keyboard::S:
            newPos -= moveSpeed; // 向后移动
            break;
        case sf::Keyboard::A:
            newPos -= moveSpeed; // 向左（后退）
            break;
        case sf::Keyboard::D:
            newPos += moveSpeed; // 向右（前进）
            break;
        default:
            return;
        }

        // 边界检查
        float maxTrackLengthMm = 2 * 40000.0f + 2 * M_PI * 2500.0f; 
        float maxTrackLength = maxTrackLengthMm / 1000.0f;          

        if (newPos >= 0.0f && newPos <= maxTrackLength)
        {
            vehicle->position_m = newPos;
            vehicle->m_state.position = newPos;
            
            std::cout << "Vehicle moved to position: " << newPos << "m" << std::endl;
        }
    }
    
    void resetSimulation()
    {
        // 重置车辆位置和状态
        vehicle->position_m = 32.0f;
        vehicle->velocity_mps = 0.0f;
        vehicle->is_loaded = false;
        vehicle->m_state.position = vehicle->position_m;
        vehicle->m_state.motionState = SimpleVehicle::State::Stopped;

        std::cout << "Simulation reset" << std::endl;
    }
    
    void render()
    {
        window.clear(sf::Color(245, 245, 245)); // 浅灰色背景

        // 首先渲染仓库仿真视图（背景层）
        if (simulationView)
        {
            // 设置仿真视图的视口
            float simulationViewLeft = SIMULATION_VIEW_MARGIN;
            float simulationViewTop = TOOLBAR_HEIGHT + SIMULATION_VIEW_MARGIN;
            float simulationViewWidth = window.getSize().x - SIMULATION_VIEW_MARGIN * 2;
            float simulationViewHeight = window.getSize().y - TOOLBAR_HEIGHT - SIMULATION_VIEW_MARGIN * 2;

            sf::FloatRect simulationViewport(
                simulationViewLeft / window.getSize().x,
                simulationViewTop / window.getSize().y,
                simulationViewWidth / window.getSize().x,
                simulationViewHeight / window.getSize().y);

            simulationView->updateViewport(simulationViewport);
            simulationView->renderWorld(window);
        }

        // 然后渲染GUI组件（顶层）- 确保在SimulationView之后渲染

        // 渲染工具栏（顶部）
        toolbar->render(window, sf::Vector2f(0, 0));

        // 渲染演示说明文字（最顶层）
        renderInstructions();

        window.display();
    }

    void renderInstructions()
    {
        sf::Text instructions;
        instructions.setFont(font);
        instructions.setCharacterSize(14);
        instructions.setFillColor(sf::Color::Black);
        instructions.setPosition(10, window.getSize().y - 120);
        std::string text = "GUI Phase 1 Minimal Demo - Warehouse & Vehicles\n"
                           "=== 仿真控制 ===\n"
                           "Space: 开始/暂停仿真  |  R: 重置仿真  |  ESC: 退出程序\n"
                           "\n=== 显示控制 ===\n"
                           "T: 切换坐标网格显示  |  G: 切换仓库显示  |  V: 切换车辆显示  |  C: 切换调试信息显示\n"
                           "\n=== 车辆控制 ===\n"
                           "WASD: 移动车辆  |  鼠标: 点击交互";
        instructions.setString(text);

        window.draw(instructions);
    }

    void run()
    {
        sf::Clock clock;
        std::cout << "GUI Phase 1 Minimal Demo Started" << std::endl;
        std::cout << "Features:" << std::endl;
        std::cout << "1. Toolbar - Time format display (HH:MM:SS.mmm)" << std::endl;
        std::cout << "2. SimulationView - Warehouse rendering and vehicle control" << std::endl;
        std::cout << "3. Keyboard controls - Display toggles and vehicle movement" << std::endl;
        std::cout << "\nInstructions: Press Space to start simulation, T/G/V/C to toggle displays" << std::endl;

        while (window.isOpen())
        {
            float deltaTime = clock.restart().asSeconds();

            handleEvents();
            update(deltaTime);
            render();
        }
    }
};

int main()
{
    try
    {
        MinimalDemoApp app;
        app.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Demo program error: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}
