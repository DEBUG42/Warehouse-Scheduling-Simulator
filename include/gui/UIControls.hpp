#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <memory>
#include <functional>

// 格式化工具命名空间
namespace UIUtils
{
    /**
     * @brief 格式化仿真时间为 HH:MM:SS.mmm 格式
     * @param seconds 秒数（包含小数部分）
     * @return 格式化的时间字符串
     */
    std::string formatSimulationTime(float seconds);

    /**
     * @brief 根据优先级获取颜色
     * @param priority 优先级（0=低，1=中，2=高）
     * @return 对应的颜色
     */
    sf::Color getPriorityColor(int priority);
}

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
    const sf::Font &m_fontRef; // 保存字体引用以备后用

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

    // 获取按钮尺寸
    sf::Vector2f getSize() const { return {m_bounds.width, m_bounds.height}; }

    /**
     * @brief 设置按钮标签文本和可选的字体大小。
     * @param newLabel 新的标签文本。
     * @param characterSize 新的字符大小，如果为0则使用基于按钮高度的默认大小。
     */
    void setLabel(const std::string &newLabel, unsigned int characterSize = 0);
};

// 时间显示控件
class TimeDisplay : public UIControl
{
private:
    sf::Text m_simTimeText; // 仿真时间文本
    float m_simTime = 0.0f; // 仿真时间(秒)

public:
    TimeDisplay(const sf::FloatRect &bounds, const sf::Font &font);

    // 更新时间显示
    void updateTime(float simTime);

    // 处理事件(本控件不需要交互)
    bool handleEvent(const sf::Event &event, const sf::Vector2f &mousePos) override;

    // 渲染控件
    void render(sf::RenderTarget &target, const sf::Vector2f &position) override;
};

// 速度控制滑块
class SpeedControl : public UIControl
{
private:
    sf::RectangleShape m_track; // 滑块轨道
    sf::CircleShape m_handle;   // 滑块手柄
    sf::Text m_valueText;       // 数值文本
    sf::Text m_labelText;       // 标签文本

    float m_minValue = 0.1f;     // 最小值
    float m_maxValue = 2.0f;     // 最大值
    float m_currentValue = 1.0f; // 当前值

    bool m_isDragging = false;                             // 是否正在拖动
    std::function<void(float)> m_callback = nullptr;       // 值变化回调
    std::function<void(float)> m_onValueChanged = nullptr; // 值变化回调（兼容旧接口）

public:
    SpeedControl(const sf::FloatRect &bounds, const sf::Font &font, float minValue = 0.1f, float maxValue = 15.0f, float initialValue = 1.0f);

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
