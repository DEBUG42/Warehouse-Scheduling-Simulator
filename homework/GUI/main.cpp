#include "MainWindow.hpp"
#include <iostream>

/**
 * @brief 主函数
 *
 * 程序入口点，创建并运行主窗口
 */
int main()
{
    try
    {
        std::cout << "物流穿梭车系统仿真启动中..." << std::endl;

        // 创建主窗口
        MainWindow window;

        // 运行应用程序
        window.run();

        std::cout << "应用程序正常退出" << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "发生错误: " << e.what() << std::endl;
        return 1;
    }
    catch (...)
    {
        std::cerr << "发生未知错误" << std::endl;
        return 2;
    }

    return 0;
}
