#include "../../TGUIWrapper.hpp"
#include <SFML/Graphics.hpp>
#include <iostream>

/**
 * @brief 简单的TGUI示例，展示如何使用TGUIWrapper创建基本UI
 */
int main()
{
    // 创建SFML窗口
    sf::RenderWindow window(sf::VideoMode(800, 600), "仓储仿真系统 - TGUI示例");
    window.setFramerateLimit(60);

    // 初始化TGUI
    WarehouseUI::TGUIWrapper gui(window);

    // 创建一个面板
    auto panel = gui.createPanel({"10%", "10%"}, {"80%", "80%"});
    panel->getRenderer()->setBackgroundColor(sf::Color(50, 50, 50, 200));

    // 创建标题标签
    auto titleLabel = gui.createLabel("仓储仿真系统", {"50%", "5%"}, 24);
    titleLabel->setOrigin(0.5f, 0.0f); // 居中对齐
    titleLabel->getRenderer()->setTextColor(sf::Color::White);

    // 创建按钮
    auto startButton = gui.createButton("开始仿真", {"50%", "30%"}, {"30%", "8%"}, []()
                                        { std::cout << "开始仿真按钮被点击" << std::endl; });
    startButton->setOrigin(0.5f, 0.5f); // 居中对齐

    auto settingsButton = gui.createButton("系统设置", {"50%", "45%"}, {"30%", "8%"}, []()
                                           { std::cout << "系统设置按钮被点击" << std::endl; });
    settingsButton->setOrigin(0.5f, 0.5f); // 居中对齐

    auto exitButton = gui.createButton("退出系统", {"50%", "60%"}, {"30%", "8%"}, []()
                                       { std::cout << "退出系统按钮被点击" << std::endl; });
    exitButton->setOrigin(0.5f, 0.5f); // 居中对齐

    // 创建下拉列表
    auto comboBox = gui.createComboBox({"50%", "75%"}, {"30%", "6%"}, [](std::string item)
                                       { std::cout << "选择了: " << item << std::endl; });
    comboBox->setOrigin(0.5f, 0.5f); // 居中对齐
    comboBox->addItem("模式1: 自动调度");
    comboBox->addItem("模式2: 手动控制");
    comboBox->addItem("模式3: 混合模式");

    // 创建版本标签
    auto versionLabel = gui.createLabel("版本: 0.1.0-alpha", {"95%", "95%"}, 12);
    versionLabel->setOrigin(1.0f, 1.0f); // 右下对齐
    versionLabel->getRenderer()->setTextColor(sf::Color(200, 200, 200));

    // 主循环
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            // 处理TGUI事件
            gui.handleEvent(event);

            // 处理窗口关闭事件
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
        }

        // 清除窗口
        window.clear(sf::Color(30, 30, 30));

        // 绘制TGUI
        gui.draw();

        // 显示窗口内容
        window.display();
    }

    return 0;
}