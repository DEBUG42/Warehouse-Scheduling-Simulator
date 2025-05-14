#include <SFML/Graphics.hpp>
#include <iostream>
#include <memory>
#include "MainWindow.hpp"
#include "TestSimulationEngine.hpp"

// 主程序入口
int main()
{
    try
    {        // 创建模拟仿真引擎
        TestSimulationEngine engine;
        engine.init(); // 使用正确的初始化方法
        
        // 创建主窗口
        MainWindow window;
        window.initialize(engine);
        
        // 运行事件循环
        window.runEventLoop();
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
