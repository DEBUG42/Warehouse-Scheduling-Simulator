#pragma once
#include <SFML/Graphics.hpp>
#include <functional>

// UI控件基类
class UIControl
{
public:
    virtual ~UIControl() = default;
    virtual void render(sf::RenderTarget &target, const sf::Vector2f &position) = 0;
    virtual bool handleEvent(const sf::Event &event, const sf::Vector2f &mousePos) = 0;
    virtual sf::FloatRect getBounds() const = 0;
};

// 按钮控件
class Button : public UIControl
{
private:
    sf::RectangleShape m_background;
    sf::Text m_text;
    sf::FloatRect m_bounds;
    bool m_isPressed = false;
    std::function<void()> m_callback;

public:
    Button(const sf::FloatRect &bounds, const std::string &text, const sf::Font &font);

    void setCallback(std::function<void()> callback);

    virtual void render(sf::RenderTarget &target, const sf::Vector2f &position) override;
    virtual bool handleEvent(const sf::Event &event, const sf::Vector2f &mousePos) override;
    virtual sf::FloatRect getBounds() const override;
};

// 时间显示控件
class TimeDisplay : public UIControl
{
private:
    sf::RectangleShape m_background;
    sf::Text m_timeText;
    sf::FloatRect m_bounds;
    float m_time = 0.0f;

public:
    TimeDisplay(const sf::FloatRect &bounds, const sf::Font &font);

    void updateTime(float timeValue);

    virtual void render(sf::RenderTarget &target, const sf::Vector2f &position) override;
    virtual bool handleEvent(const sf::Event &event, const sf::Vector2f &mousePos) override;
    virtual sf::FloatRect getBounds() const override;
};

// 速度控制滑块
class SpeedControl : public UIControl
{
private:
    sf::RectangleShape m_track;
    sf::CircleShape m_handle;
    sf::Text m_valueText;
    sf::FloatRect m_bounds;
    
    float m_minValue = 0.1f;
    float m_maxValue = 2.0f;
    float m_currentValue = 1.0f;
    
    bool m_isDragging = false;
    std::function<void(float)> m_callback;

public:
    SpeedControl(const sf::FloatRect &bounds, const sf::Font &font);

    void setValue(float value);
    float getValue() const;
    void setCallback(std::function<void(float)> callback);

    virtual void render(sf::RenderTarget &target, const sf::Vector2f &position) override;
    virtual bool handleEvent(const sf::Event &event, const sf::Vector2f &mousePos) override;
    virtual sf::FloatRect getBounds() const override;
    
    // 添加以下方法的声明
    void updateHandlePosition();
    void updateValueText();
    void updateValueFromPosition(float xPos);
};
