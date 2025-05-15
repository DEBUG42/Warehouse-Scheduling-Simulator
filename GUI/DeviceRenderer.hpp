//可编译通过但是纹理和图标没有制作 暂时不能渲染
#pragma once
#include <SFML/Graphics.hpp>
#include <map>
#include "../Core/Device.hpp"
  
class DeviceRenderer {
private:
// 状态颜色编码 
    const sf::Color COLOR_IDLE {75, 185, 85};    // 空闲状态
    const sf::Color COLOR_BUSY {215, 60, 60};    // 忙碌状态
    const sf::Color COLOR_PENDING {230, 170, 50};// 准备中状态
    // 设备类型图标映射    
    std::map<DeviceStatus, sf::Texture> m_iconTextures;

public:

    /**
     * @brief 绘制单个设备
     * @param target 渲染目标
     * @param device 设备数据引用
     * @param position 设备世界坐标
     */
    void renderDevice(sf::RenderTarget& target, 
                     const DeviceStatus& device,
                     const sf::Vector2f& position);

    /**
     * @brief 加载设备图标资源
     * @param basePath 图标文件基础路径
     */
    void loadResources(const std::string& basePath);
};