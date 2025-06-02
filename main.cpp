/**
 * @file main.cpp
 * @brief 智能仓库调度仿真系统 - 主程序入口
 *
 * 使用现有的 RealBackendAdapter 来连接前端GUI和后端仿真引擎
 *
 * @author Bai Minghui
 * @date 2025-05-29
 * @version 1.0
 */

#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include <SFML/Graphics.hpp>

#include "gui/MainWindow.hpp"
#include "gui/RealBackendAdapter.hpp"

/**
 * @brief 程序主函数
 *
 * 使用现有的 RealBackendAdapter 来实现前后端集成
 */
int main()
{
    try
    {
        std::cout << "=== 智能仓库调度仿真系统 v1.0 ===" << std::endl;
        std::cout << "正在启动系统..." << std::endl;

        // 创建后端适配器 (使用现有的 RealBackendAdapter)
        std::cout << "初始化后端适配器..." << std::endl;
        auto backendAdapter = std::make_shared<RealBackendAdapter>();
        std::cout << "✓ 后端适配器初始化完成" << std::endl;

        // 创建主窗口
        std::cout << "初始化用户界面..." << std::endl;
        MainWindow mainWindow;
        mainWindow.initialize(backendAdapter);
        std::cout << "✓ GUI界面初始化完成" << std::endl;

        std::cout << "=== 系统启动完成！===" << std::endl;
        std::cout << "使用说明：" << std::endl;
        std::cout << "- 点击工具栏的播放/暂停按钮控制仿真" << std::endl;
        std::cout << "- 拖动速度滑条调整仿真速度" << std::endl;
        std::cout << "- 点击车辆查看详细信息" << std::endl;
        std::cout << "- 关闭窗口退出程序" << std::endl;
        std::cout << "==================" << std::endl;

        // 主事件循环
        sf::Clock updateClock;
        const float updateInterval = 1.0f / 60.0f; // 60Hz更新频率

        while (mainWindow.isOpen())
        {
            // 处理GUI事件
            sf::Event event;
            while (mainWindow.pollEvent(event))
            {
                mainWindow.processEvent(event);

                if (event.type == sf::Event::Closed)
                {
                    std::cout << "用户请求关闭程序..." << std::endl;
                    mainWindow.close();
                }

                // 快捷键处理
                if (event.type == sf::Event::KeyPressed)
                {
                    if (event.key.code == sf::Keyboard::F5)
                    {
                        std::cout << "重置仿真..." << std::endl;
                        backendAdapter->resetSimulation();
                    }
                    else if (event.key.code == sf::Keyboard::Space)
                    {
                        // 空格键暂停/恢复
                        static bool isPaused = false;
                        if (isPaused)
                        {
                            backendAdapter->resumeSimulation();
                        }
                        else
                        {
                            backendAdapter->pauseSimulation();
                        }
                        isPaused = !isPaused;
                    }
                }
            }

            // 定期更新后端
            if (updateClock.getElapsedTime().asSeconds() >= updateInterval)
            {
                backendAdapter->updateBackend(updateInterval);
                updateClock.restart();
            }

            // 渲染界面
            mainWindow.renderFrame();
            mainWindow.display();

            // 避免CPU占用过高
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        std::cout << "程序正常退出" << std::endl;
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "系统错误: " << e.what() << std::endl;
        return -1;
    }
    catch (...)
    {
        std::cerr << "发生未知错误！" << std::endl;
        return -2;
    }
}
