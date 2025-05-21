#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include <filesystem>

int main()
{
    // 创建窗口
    sf::RenderWindow window(sf::VideoMode(800, 600), "字体测试");
    window.setFramerateLimit(60);

    // 获取当前工作目录
    std::cout << "当前工作目录: " << std::filesystem::current_path() << std::endl;

    // 尝试加载字体
    sf::Font font;
    std::string fontPath = "../../assets/fonts/SourceHanSansSC-Regular.otf";
    std::cout << "尝试加载字体: " << fontPath << std::endl;

    if (!font.loadFromFile(fontPath))
    {
        std::cerr << "无法加载字体文件：" << fontPath << std::endl;
        std::cerr << "尝试加载备用字体..." << std::endl;

        // 尝试加载备用字体
        fontPath = "../../assets/fonts/arial.ttf";
        if (!font.loadFromFile(fontPath))
        {
            std::cerr << "无法加载备用字体文件：" << fontPath << std::endl;
            return -1;
        }
        std::cout << "成功加载备用字体：" << fontPath << std::endl;
    }
    else
    {
        std::cout << "成功加载字体：" << fontPath << std::endl;
    }

    // 创建中文文本
    sf::Text chineseText;
    chineseText.setFont(font);
    chineseText.setString("你好，世界！");
    chineseText.setCharacterSize(30);
    chineseText.setFillColor(sf::Color::White);
    chineseText.setPosition(50, 50);

    // 创建英文文本
    sf::Text englishText;
    englishText.setFont(font);
    englishText.setString("Hello, World!");
    englishText.setCharacterSize(30);
    englishText.setFillColor(sf::Color::White);
    englishText.setPosition(50, 100);

    // 创建混合文本
    sf::Text mixedText;
    mixedText.setFont(font);
    mixedText.setString("中英文混合测试 Mixed Text Test");
    mixedText.setCharacterSize(30);
    mixedText.setFillColor(sf::Color::White);
    mixedText.setPosition(50, 150);

    // 主循环
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // 清空窗口
        window.clear(sf::Color(50, 50, 50));

        // 绘制文本
        window.draw(chineseText);
        window.draw(englishText);
        window.draw(mixedText);

        // 显示绘制的内容
        window.display();
    }

    return 0;
}