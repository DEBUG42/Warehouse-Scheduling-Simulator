#include <SFML/Graphics.hpp>
#include "gui/MainWindow.hpp"
#include "test/GuiTestMockSimulation.hpp" // 使用我们新的 mock 实现
#include <memory>
#include <iostream>

// 假设ConfigReader的路径和用法，如果不需要或不确定，可以暂时移除
// #include "utils/ConfigReader.hpp"

int main()
{
    // 1. 加载配置 (可选, 如果 ConfigReader 可用且需要)
    // ConfigReader config;
    // if (!config.load("config/gui_test_config.json")) { // 假设配置文件路径
    //     std::cerr << "Warning: Could not load GUITest config. Using defaults." << std::endl;
    // }
    // float trackLength = config.getFloat("simulation.track_length_mm", 100000.0f); // 示例配置项

    float trackLength = 100000.0f; // 100 meters

    // 2. 创建 Mock Simulation Interface
    //    使用 std::make_shared 来创建共享指针
    auto mockSimInterface = std::make_shared<test::GuiTestMockSimulation>(trackLength);

    // 3. 初始化 Mock 数据的场景
    //    这将调用 sim::MockSimulationInterface::initializeDefaultScenario()
    mockSimInterface->initializeDefaultScenario();
    // 你可以通过 getInternalMock() 来添加更多自定义的车辆、任务或设备
    // 例如:
    // sim::MockVehicle v1(1, 0.0f);
    // mockSimInterface->getInternalMock().addVehicle(v1);
    // sim::MockTask t1(1, "Test Task 1", 10000.0f, 50000.0f, "D1", "D2");
    // mockSimInterface->getInternalMock().addTask(t1);
    // sim::MockDeviceState dev1("D1");
    // mockSimInterface->getInternalMock().addDevice(dev1);

    // 4. 创建 MainWindow 实例
    MainWindow mainWindow; // MainWindow 构造函数不接受参数

    // 5. 初始化 MainWindow 并传入仿真接口
    //    MainWindow::initialize 将创建子视图并设置回调
    mainWindow.initialize(mockSimInterface);

    // 6. 运行主事件循环
    //    MainWindow::runEventLoop 将处理窗口事件、更新和渲染
    //    它内部会调用 sf::RenderWindow 的 display() 和 pollEvent()
    //    以及我们 mock 的 update() 方法通过回调机制

    sf::Clock deltaClock;
    std::cout << "Starting main loop..." << std::endl;

    while (mainWindow.isOpen())
    {
        sf::Time dt = deltaClock.restart();

        // Event processing
        sf::Event event;
        while (mainWindow.pollEvent(event)) // Use mainWindow's pollEvent
        {
            if (event.type == sf::Event::Closed)
            {
                mainWindow.close();
            }
            // Pass event to MainWindow for its components
            mainWindow.processEvent(event);

            if (event.type == sf::Event::Resized)
            {
                // Optional: Inform MainWindow or recalculate layout if necessary.
                // MainWindow's handleSystemEvent (called by processEvent)
                // already calls updateLayout on resize.
                // sf::FloatRect visibleArea(0, 0, static_cast<float>(event.size.width), static_cast<float>(event.size.height));
                // mainWindow.setView(sf::View(visibleArea));
            }
        }

        // Update simulation state
        mockSimInterface->update(dt);

        // Render
        mainWindow.renderFrame(); // Calls clear() and draws components
        mainWindow.display();     // Swaps buffers

    } // End of main while loop

    std::cout << "GUITestMain finished." << std::endl;
    return 0;
}