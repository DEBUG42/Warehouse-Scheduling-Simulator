/**
 * @file main.cpp
 * @brief 物流仓库仿真系统的主入口程序
 * @author GitHub Copilot
 * @date 2025-05-19
 */

#include "MainWindow.hpp"
#include "MockSimulationInterface.hpp"
#include <iostream>
#include <memory>

/**
 * @brief 程序主函数
 * @return 程序退出码
 */
int main()
{
    try
    {
        std::cout << "Warehouse Simulation System starting..." << std::endl;

        // 创建模拟仿真接口（用于GUI测试，不需连接实际后端）
        // 参数为模拟的小车数量
        std::shared_ptr<SimulationInterface> simInterface = std::make_shared<MockSimulationInterface>(5);

        // 创建并初始化主窗口
        MainWindow mainWindow;
        mainWindow.initialize(simInterface);

        std::cout << "初始化完成，开始运行..." << std::endl;

        // 运行事件循环
        mainWindow.runEventLoop();

        std::cout << "程序正常退出" << std::endl;
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "错误：" << e.what() << std::endl;
        return -1;
    }
    catch (...)
    {
        std::cerr << "未知错误" << std::endl;
        return -2;
    }
}
