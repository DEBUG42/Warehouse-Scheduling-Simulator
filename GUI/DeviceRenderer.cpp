#include "DeviceRenderer.hpp"
#include <iostream>
#include <cmath>

// 根据设备状态在指定位置渲染设备图标
void DeviceRenderer::renderDevice(sf::RenderTarget& target, 
    const DeviceStatus& device,
    const sf::Vector2f& position){
    
        sf::Sprite sprite;
    
        sprite.setPosition(position);
    
        // 如果设备图标纹理存在，则设置该纹理
        if(m_iconTextures.find(device)!= m_iconTextures.end()){
            sprite.setTexture(m_iconTextures[device]);}
        
        sf::Color color;
        // 根据设备状态设置图标颜色
        switch(device){
            case DeviceStatus::idle:
                color = COLOR_IDLE;
                break;
            case DeviceStatus::working:
                color = COLOR_BUSY;
                break;
            case DeviceStatus::preparing:
                color = COLOR_PENDING;
                break;
        }
        sprite.setColor(color);
        
        // 绘制设备图标到渲染目标
        target.draw(sprite);
}

// 加载设备图标资源
void DeviceRenderer::loadResources(const std::string& basepath){
    // 定义不同设备状态对应的图标文件名
    std::map<DeviceStatus, std::string> iconFileNames = {
        {DeviceStatus::idle, "idle_icon.png"},
        {DeviceStatus::working, "working_icon.png"},
        {DeviceStatus::preparing, "preparing_icon.png"}
    };
    
    // 遍历图标文件名映射，加载纹理到图标纹理映射
    for (const auto& [status, fileName] : iconFileNames) {
        if (!m_iconTextures[status].loadFromFile(basepath + fileName)) {
            // 处理加载失败的情况
            std::cerr << "Failed to load texture for " << static_cast<int>(status) << std::endl;
        }
    }
}