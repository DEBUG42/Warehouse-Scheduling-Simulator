#include <SFML/Graphics.hpp>
#include <iostream>

int main()
{
    // 小车物理尺寸 (毫米)
    const float vehicleLengthMm = 2000.0f;
    const float vehicleWidthMm = 1000.0f;

    // 毫米到像素的转换比例 (可调整以改变屏幕显示大小)
    const float mmToPxRatio = 0.1f; // 1mm = 0.1像素

    // 计算小车在屏幕上的像素尺寸
    const float vehicleLengthPx = vehicleLengthMm * mmToPxRatio;
    const float vehicleWidthPx = vehicleWidthMm * mmToPxRatio;

    sf::RenderWindow window(sf::VideoMode(800, 600), "Vehicle Render Test");
    window.setFramerateLimit(60);

    sf::Texture vehicleTexture;
    std::string texturePath = "assets/image/wuliu-car.jpg"; // 路径相对于可执行文件
    // 通常测试的可执行文件在 src/test/ 或 build/test/，而assets在项目根目录
    // 因此，我们可能需要调整路径，例如 "../../assets/image/wuliu-car.jpg"
    // 让我们先尝试一个更可能正确的相对路径，假设可执行文件在 src/test/
    std::string correctedTexturePath = "../../assets/image/wuliu-car.jpg";

    if (!vehicleTexture.loadFromFile(correctedTexturePath))
    {
        // 如果在 src/test/ 目录下找不到，尝试项目根目录下的相对路径（如果可执行文件在根目录）
        // 或其他常见构建输出位置的相对路径。
        // 对于从 src/test 运行的g++命令，../../assets 是正确的
        std::cerr << "Error loading vehicle texture from: " << correctedTexturePath << std::endl;
        // 尝试另一个常见的相对路径，如果可执行文件最终在根目录或类似build的目录
        if (!vehicleTexture.loadFromFile("assets/image/wuliu-car.jpg"))
        {
            std::cerr << "Error loading vehicle texture from: assets/image/wuliu-car.jpg as well. Please check path." << std::endl;
            return -1;
        }
    }

    sf::Sprite vehicleSprite;
    vehicleSprite.setTexture(vehicleTexture);

    // 获取纹理的原始尺寸
    sf::Vector2u textureSize = vehicleTexture.getSize();

    // 计算所需的缩放比例，以使sprite匹配目标像素尺寸
    // sprite的scale是相对于其纹理的原始尺寸的
    float scaleX = vehicleLengthPx / textureSize.x;
    float scaleY = vehicleWidthPx / textureSize.y;

    // 重要：这里假设图片的方向与长度/宽度定义一致。
    // 如果wuliu-car.jpg的视觉上的"长"对应的是物理上的"宽"，则需要交换scaleX和scaleY的计算中的vehicleLengthPx和vehicleWidthPx
    // 或者，如果图片本身是正方形或比例不同，直接缩放会导致拉伸。
    // 我们先按直接缩放处理。
    vehicleSprite.setScale(scaleX, scaleY);

    // 将小车放置在窗口中心
    vehicleSprite.setOrigin(textureSize.x / 2.0f, textureSize.y / 2.0f); // 设置原点为纹理中心以方便定位和旋转
    vehicleSprite.setPosition(window.getSize().x / 2.0f, window.getSize().y / 2.0f);

    std::cout << "Vehicle Sprite Info:" << std::endl;
    std::cout << "  Target Px Size: " << vehicleLengthPx << " (length) x " << vehicleWidthPx << " (width)" << std::endl;
    std::cout << "  Texture Size: " << textureSize.x << " x " << textureSize.y << std::endl;
    std::cout << "  Calculated Scale: " << scaleX << " , " << scaleY << std::endl;
    std::cout << "  Sprite Position: " << vehicleSprite.getPosition().x << " , " << vehicleSprite.getPosition().y << std::endl;
    std::cout << "  Sprite Origin: " << vehicleSprite.getOrigin().x << " , " << vehicleSprite.getOrigin().y << std::endl;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear(sf::Color(200, 200, 200)); // 淡灰色背景
        window.draw(vehicleSprite);
        window.display();
    }

    return 0;
}