#pragma once
#include <SFML/Graphics.hpp>
#include "TrackRenderer_fixed.hpp"
#include "VehicleRenderer.hpp"
#include "WarehouseRenderer.hpp"
#include "SimulationInterface.hpp"
#include <memory>

/**
 * @brief 仿真视图类
 * 
 * 负责将仿真系统的各组件渲染到屏幕上
 * 管理视图的平移、缩放等交互
 * 处理鼠标点击等用户输入
 */
class SimulationView : public sf::Drawable
{
private:
    // 组件渲染器
    TrackRenderer_fixed m_trackRenderer;
    VehicleRenderer m_vehicleRenderer;
    WarehouseRenderer m_warehouseRenderer;
    
    // 仿真接口
    std::shared_ptr<SimulationInterface> m_interface;
    
    // 视图控制
    sf::View m_view;
    sf::Vector2f m_viewCenter{0.0f, 0.0f};
    float m_zoomLevel = 1.0f;
    
    // 交互状态
    bool m_isPanning = false;
    sf::Vector2i m_lastMousePos;
    int m_selectedObjectId = -1;
    bool m_selectedIsVehicle = true;
    
    // 字体
    sf::Font m_font;
    bool m_fontLoaded = false;
    
    // UI元素
    sf::RectangleShape m_infoPanel;
    sf::Text m_infoPanelText;
    bool m_showInfoPanel = false;
    
public:
    /**
     * @brief 构造函数
     */
    SimulationView();
    
    /**
     * @brief 设置仿真接口
     * @param interface 仿真接口对象
     */
    void setSimulationInterface(std::shared_ptr<SimulationInterface> interface);
    
    /**
     * @brief 调整视图大小
     * @param width 窗口宽度
     * @param height 窗口高度
     */
    void resize(unsigned int width, unsigned int height);
    
    /**
     * @brief 处理鼠标按下事件
     * @param mousePos 鼠标位置（窗口坐标）
     * @param button 按下的按钮
     */
    void handleMouseButtonPressed(const sf::Vector2i& mousePos, sf::Mouse::Button button);
    
    /**
     * @brief 处理鼠标释放事件
     * @param mousePos 鼠标位置（窗口坐标）
     * @param button 释放的按钮
     */
    void handleMouseButtonReleased(const sf::Vector2i& mousePos, sf::Mouse::Button button);
    
    /**
     * @brief 处理鼠标移动事件
     * @param mousePos 鼠标位置（窗口坐标）
     */
    void handleMouseMoved(const sf::Vector2i& mousePos);
    
    /**
     * @brief 处理鼠标滚轮事件
     * @param delta 滚轮增量
     * @param mousePos 鼠标位置（窗口坐标）
     */
    void handleMouseWheelScrolled(float delta, const sf::Vector2i& mousePos);
    
    /**
     * @brief 处理键盘事件
     * @param key 键代码
     */
    void handleKeyPressed(sf::Keyboard::Key key);
    
    /**
     * @brief 更新逻辑
     * @param deltaTime 时间增量（秒）
     */
    void update(float deltaTime);
    
    /**
     * @brief 将视图居中到轨道中央
     */
    void centerView();
    
    /**
     * @brief 获取当前视图
     * @return 视图对象的引用
     */
    const sf::View& getView() const { return m_view; }
    
protected:
    /**
     * @brief 重载的绘制方法
     * @param target 渲染目标
     * @param states 渲染状态
     */
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
    
private:
    /**
     * @brief 选择视图中的对象
     * @param worldPos 世界坐标
     */
    void selectObjectAt(const sf::Vector2f& worldPos);
    
    /**
     * @brief 更新信息面板
     */
    void updateInfoPanel();
    
    /**
     * @brief 尝试加载字体
     * @return 是否成功加载
     */
    bool tryLoadFont();
};
