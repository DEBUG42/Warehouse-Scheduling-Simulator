#include "DeviceRenderer.hpp"
#include "../Core/Device.hpp"
#include <iostream>

/**
 * @brief 加载设备图标资源
 *
 * 从指定路径加载四种类型设备图标（入库接口、出库接口、入库作业口、出库作业口）
 * 如果无法找到图标文件，会创建默认的白色方块作为替代
 *
 * @param basePath 图标文件基础路径
 */
void DeviceRenderer::loadResources(const std::string &basePath)
{
    // 加载设备图标纹理
    sf::Texture storageInTexture, storageOutTexture;
    sf::Texture workInTexture, workOutTexture;

    // 尝试加载图标
    bool loadSuccess = true;

    if (!storageInTexture.loadFromFile(basePath + "storage_in.png"))
    {
        std::cerr << "无法加载入库接口图标" << std::endl;
        loadSuccess = false;
    }

    if (!storageOutTexture.loadFromFile(basePath + "storage_out.png"))
    {
        std::cerr << "无法加载出库接口图标" << std::endl;
        loadSuccess = false;
    }

    if (!workInTexture.loadFromFile(basePath + "work_in.png"))
    {
        std::cerr << "无法加载入库作业口图标" << std::endl;
        loadSuccess = false;
    }

    if (!workOutTexture.loadFromFile(basePath + "work_out.png"))
    {
        std::cerr << "无法加载出库作业口图标" << std::endl;
        loadSuccess = false;
    }

    // 如果加载失败，创建默认纹理
    if (!loadSuccess)
    {
        std::cout << "使用默认设备图标" << std::endl;

        // 创建32x32的默认纹理
        sf::Image defaultImage;
        defaultImage.create(32, 32, sf::Color::White);

        storageInTexture.loadFromImage(defaultImage);
        storageOutTexture.loadFromImage(defaultImage);
        workInTexture.loadFromImage(defaultImage);
        workOutTexture.loadFromImage(defaultImage);
    }

    // 将纹理存储到映射表中
    m_iconTextures[DeviceType::StorageIn] = storageInTexture;
    m_iconTextures[DeviceType::StorageOut] = storageOutTexture;
    m_iconTextures[DeviceType::WorkstationIn] = workInTexture;
    m_iconTextures[DeviceType::WorkstationOut] = workOutTexture;
}

/**
 * @brief 绘制单个设备
 *
 * 根据设备类型、状态和位置在目标渲染表面上绘制设备图形
 * 包括设备图标、底座、ID标签以及任务队列指示器
 *
 * @param target 渲染目标
 * @param device 设备数据引用
 * @param position 设备世界坐标
 */
void DeviceRenderer::renderDevice(sf::RenderTarget &target,
                                  const DeviceState &device,
                                  const sf::Vector2f &position)
{
    // 创建设备图标精灵
    sf::Sprite iconSprite;

    // 设置对应类型的纹理
    if (m_iconTextures.find(device.type) != m_iconTextures.end())
    {
        iconSprite.setTexture(m_iconTextures[device.type]);
    }
    else
    {
        // 如果找不到纹理，使用默认纹理
        iconSprite.setTexture(m_iconTextures.begin()->second);
    }

    // 设置精灵中心点
    sf::Vector2f origin(iconSprite.getLocalBounds().width / 2,
                        iconSprite.getLocalBounds().height / 2);
    iconSprite.setOrigin(origin);
    iconSprite.setPosition(position);

    // 设置设备底座
    sf::CircleShape baseShape(20.0f);
    baseShape.setOrigin(20.0f, 20.0f);
    baseShape.setPosition(position);

    // 根据设备状态设置颜色
    sf::Color baseColor;
    switch (device.status)
    {
    case DeviceStatus::IDLE:
        baseColor = COLOR_IDLE;
        break;
    case DeviceStatus::WORKING:
        baseColor = COLOR_WORKING;
        break;
    case DeviceStatus::FAULT:
        baseColor = COLOR_FAULT;
        break;
    case DeviceStatus::OFFLINE:
        baseColor = COLOR_OFFLINE;
        break;
    }
    baseShape.setFillColor(baseColor);

    // 绘制底座和图标
    target.draw(baseShape);
    target.draw(iconSprite); // 绘制设备ID标签
    // 注意：在实际应用中，应将字体加载放到构造函数中而不是每次渲染时加载
    static sf::Font font;
    static bool fontLoaded = false;

    if (!fontLoaded)
    {
        try
        {
            // 尝试多个可能的路径
            if (font.loadFromFile("resources/fonts/arial.ttf"))
            {
                fontLoaded = true;
            }
            else if (font.loadFromFile("GUI/resources/fonts/arial.ttf"))
            {
                fontLoaded = true;
            }
            else if (font.loadFromFile("fonts/arial.ttf"))
            {
                fontLoaded = true;
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << "无法加载字体: " << e.what() << std::endl;
        }
    }

    if (fontLoaded)
    {
        sf::Text idText;
        idText.setFont(font);
        idText.setString(std::to_string(device.id));
        idText.setCharacterSize(16);
        idText.setFillColor(sf::Color::White);

        // 居中文本
        sf::FloatRect textBounds = idText.getLocalBounds();
        idText.setOrigin(textBounds.width / 2, textBounds.height / 2);
        idText.setPosition(position.x, position.y + 30.0f);

        target.draw(idText);
    }

    // 如果设备有队列任务，显示任务数量
    if (device.queuedTaskCount > 0)
    {
        sf::CircleShape taskCountBg(10.0f);
        taskCountBg.setFillColor(sf::Color(230, 60, 60));
        taskCountBg.setOrigin(10.0f, 10.0f);
        taskCountBg.setPosition(position.x + 20.0f, position.y - 20.0f);

        sf::Text taskCountText;
        if (fontLoaded)
        {
            taskCountText.setFont(font);
            taskCountText.setString(std::to_string(device.queuedTaskCount));
            taskCountText.setCharacterSize(14);
            taskCountText.setFillColor(sf::Color::White);

            // 居中文本
            sf::FloatRect textBounds = taskCountText.getLocalBounds();
            taskCountText.setOrigin(textBounds.width / 2, textBounds.height / 2);
            taskCountText.setPosition(position.x + 20.0f, position.y - 20.0f);

            target.draw(taskCountBg);
            target.draw(taskCountText);
        }
    }
}