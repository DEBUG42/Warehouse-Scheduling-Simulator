#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <memory>
#include <functional>

// UI控件基类
class UIControl
{
protected:
    sf::FloatRect m_bounds;   // 控件范围
    bool m_isHovered = false; // 是否悬停
    bool m_isActive = false;  // 是否激活（如按下）

public:
    virtual ~UIControl() = default;

    // 检查点是否在控件范围内
    bool containsPoint(const sf::Vector2f &point) const
    {
        return m_bounds.contains(point);
    }

    // 处理事件
    virtual bool handleEvent(const sf::Event &event, const sf::Vector2f &mousePos) = 0;

    // 渲染控件
    virtual void render(sf::RenderTarget &target, const sf::Vector2f &position) = 0;
    
    // 获取控件边界
    virtual sf::FloatRect getBounds() const { return m_bounds; }
};

// 按钮控件
class Button : public UIControl
{
private:
    sf::RectangleShape m_background;
    sf::Text m_text;
    std::function<void()> m_onClick;

    sf::Color m_normalColor{60, 60, 60};
    sf::Color m_hoverColor{80, 80, 80};
    sf::Color m_activeColor{40, 40, 40};

public:
    Button(const sf::FloatRect &bounds, const sf::Font &font, const std::string &label);

    // 设置点击回调
    void setCallback(std::function<void()> callback);

    // 处理事件
    bool handleEvent(const sf::Event &event, const sf::Vector2f &mousePos) override;
    
    // 渲染控件
    void render(sf::RenderTarget &target, const sf::Vector2f &position) override;
};

// 时间显示控件
class TimeDisplay : public UIControl
{
private:
    sf::Text m_simTimeText;   // 仿真时间文本
    sf::Text m_realTimeText;  // 真实时间文本
    
    float m_simTime = 0.0f;  // 仿真时间(秒)
    float m_realTime = 0.0f; // 真实时间(秒)

public:
    TimeDisplay(const sf::FloatRect &bounds, const sf::Font &font);
    
    // 更新时间显示
    void updateTime(float simTime, float realTime = 0.0f);
    
    // 处理事件(本控件不需要交互)
    bool handleEvent(const sf::Event &event, const sf::Vector2f &mousePos) override;
    
    // 渲染控件
    void render(sf::RenderTarget &target, const sf::Vector2f &position) override;
};

// 速度控制滑块
class SpeedControl : public UIControl
{
private:
    sf::RectangleShape m_track;  // 滑块轨道
    sf::CircleShape m_handle;    // 滑块手柄
    sf::Text m_valueText;        // 数值文本
    
    float m_minValue = 0.1f;     // 最小值
    float m_maxValue = 2.0f;     // 最大值
    float m_currentValue = 1.0f; // 当前值
    
    bool m_isDragging = false;   // 是否正在拖动
    std::function<void(float)> m_callback = nullptr; // 值变化回调

public:
    SpeedControl(const sf::FloatRect &bounds, const sf::Font &font);
    
    // 设置当前值
    void setValue(float value);
    
    // 获取当前值
    float getValue() const { return m_currentValue; }
    
    // 设置值变化回调
    void setCallback(std::function<void(float)> callback) { m_callback = callback; }
    
    // 处理事件
    bool handleEvent(const sf::Event &event, const sf::Vector2f &mousePos) override;
    
    // 渲染控件
    void render(sf::RenderTarget &target, const sf::Vector2f &position) override;
    
private:
    // 更新手柄位置
    void updateHandlePosition();
    
    // 更新值文本
    void updateValueText();
    
    // 根据位置更新值
    void updateValueFromPosition(float xPos);
};

// 时间显示控件
class TimeDisplay : public UIControl
{
private:
    sf::Text m_simTimeText;   // 仿真时间文本
    sf::Text m_realTimeText;  // 真实时间文本
    
    float m_simTime = 0.0f;  // 仿真时间(秒)
    float m_realTime = 0.0f; // 真实时间(秒)

public:
    TimeDisplay(const sf::FloatRect &bounds, const sf::Font &font);
    
    // 更新时间显示
    void updateTime(float simTime, float realTime = 0.0f);
    
    // 处理事件(本控件不需要交互)
    bool handleEvent(const sf::Event &event, const sf::Vector2f &mousePos) override;
    
    // 渲染控件
    void render(sf::RenderTarget &target, const sf::Vector2f &position) override;

    // 渲染控件
    void render(sf::RenderTarget &target) override;
};

// 时间显示控件
class TimeDisplay : public UIControl
{
private:
    sf::Text m_simTimeText;  // 仿真时间文本
    sf::Text m_realTimeText; // 真实时间文本
    float m_simTime = 0.0f;  // 当前仿真时间
    float m_realTime = 0.0f; // 当前真实时间

public:
    TimeDisplay(const sf::FloatRect &bounds, const sf::Font &font);

    // 更新时间值
    void updateTime(float simTime, float realTime);

    // 处理事件（时间显示无交互）
    bool handleEvent(const sf::Event &event, const sf::Vector2f &mousePos) override;

    // 渲染控件
    void render(sf::RenderTarget &target) override;
};

// 速度控制滑块
class SpeedControl : public UIControl
{
private:
    sf::RectangleShape m_track; // 滑块轨道
    sf::CircleShape m_handle;   // 滑块手柄
    sf::Text m_labelText;       // 标签文本
    sf::Text m_valueText;       // 值文本

    float m_minValue = 0.1f;     // 最小值
    float m_maxValue = 5.0f;     // 最大值
    float m_currentValue = 1.0f; // 当前值

    std::function<void(float)> m_onValueChanged; // 值变化回调

    bool m_isDragging = false; // 是否正在拖动

public:
    SpeedControl(const sf::FloatRect &bounds, const sf::Font &font);

    // 设置值变化回调
    void setCallback(std::function<void(float)> callback);

    // 获取当前值
    float getValue() const { return m_currentValue; }

    // 设置当前值
    void setValue(float value);

    // 处理事件
    bool handleEvent(const sf::Event &event, const sf::Vector2f &mousePos) override;

    // 渲染控件
    void render(sf::RenderTarget &target) override;
};
