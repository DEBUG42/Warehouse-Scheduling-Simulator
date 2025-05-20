#include "UIControls.hpp"
#include <sstream>
#include <iomanip>
#include <cmath>

// Button 实现
Button::Button(const sf::FloatRect &bounds, const sf::Font &font, const std::string &label)
    : m_onClick(nullptr)
{
    // 设置按钮范围
    m_bounds = bounds;

    // 设置按钮背景
    m_background.setSize(sf::Vector2f(bounds.width, bounds.height));
    m_background.setPosition(bounds.left, bounds.top);
    m_background.setFillColor(m_normalColor);

    // 设置按钮文本
    m_text.setFont(font);
    m_text.setString(label);
    m_text.setCharacterSize(static_cast<unsigned int>(bounds.height * 0.7f));
    m_text.setFillColor(sf::Color::White);

    // 居中文本
    sf::FloatRect textBounds = m_text.getLocalBounds();
    float textX = bounds.left + (bounds.width - textBounds.width) / 2.0f - textBounds.left;
    float textY = bounds.top + (bounds.height - textBounds.height) / 2.0f - textBounds.top;
    m_text.setPosition(textX, textY);
}

void Button::setCallback(std::function<void()> callback)
{
    m_onClick = callback;
}

bool Button::handleEvent(const sf::Event &event, const sf::Vector2f &mousePos)
{
    bool wasActive = m_isActive;

    // 鼠标移入/移出事件
    bool isHovering = containsPoint(mousePos);
    if (isHovering != m_isHovered)
    {
        m_isHovered = isHovering;
        m_background.setFillColor(m_isHovered ? m_hoverColor : m_normalColor);
    }

    // 鼠标按下事件
    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
    {
        if (m_isHovered)
        {
            m_isActive = true;
            m_background.setFillColor(m_activeColor);
            return true;
        }
    }
    // 鼠标释放事件
    else if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left)
    {
        if (m_isActive)
        {
            m_isActive = false;
            m_background.setFillColor(m_isHovered ? m_hoverColor : m_normalColor);

            // 只有当鼠标在按钮上释放时才触发回调
            if (m_isHovered && m_onClick)
            {
                m_onClick();
            }

            return true;
        }
    }

    return wasActive != m_isActive;
}

void Button::render(sf::RenderTarget &target, const sf::Vector2f &position)
{
    // 设置按钮位置
    m_background.setPosition(position);
    m_text.setPosition(position + sf::Vector2f(10.f, 5.f));

    // 绘制按钮
    target.draw(m_background);
    target.draw(m_text);
}

// TimeDisplay 实现
TimeDisplay::TimeDisplay(const sf::FloatRect &bounds, const sf::Font &font)
{
    m_bounds = bounds;

    // 设置仿真时间文本
    m_simTimeText.setFont(font);
    m_simTimeText.setCharacterSize(12);
    m_simTimeText.setFillColor(sf::Color::White);
    m_simTimeText.setPosition(bounds.left, bounds.top);

    // 设置真实时间文本
    m_realTimeText.setFont(font);
    m_realTimeText.setCharacterSize(12);
    m_realTimeText.setFillColor(sf::Color(180, 180, 180));
    m_realTimeText.setPosition(bounds.left, bounds.top + 14);

    // 初始化时间显示
    updateTime(0, 0);
}

void TimeDisplay::updateTime(float simTime, float realTime)
{
    m_simTime = simTime;
    m_realTime = realTime;

    // 格式化仿真时间
    int simHours = static_cast<int>(simTime / 3600);
    int simMinutes = static_cast<int>((simTime - simHours * 3600) / 60);
    int simSeconds = static_cast<int>(simTime) % 60;

    std::ostringstream simTimeStr;
    simTimeStr << "仿真时间: "
               << std::setw(2) << std::setfill('0') << simHours << ":"
               << std::setw(2) << std::setfill('0') << simMinutes << ":"
               << std::setw(2) << std::setfill('0') << simSeconds;
    m_simTimeText.setString(simTimeStr.str());

    // 格式化真实时间
    int realMinutes = static_cast<int>(realTime / 60);
    int realSeconds = static_cast<int>(realTime) % 60;

    std::ostringstream realTimeStr;
    realTimeStr << "运行时间: "
                << std::setw(2) << std::setfill('0') << realMinutes << ":"
                << std::setw(2) << std::setfill('0') << realSeconds;
    m_realTimeText.setString(realTimeStr.str());
}

bool TimeDisplay::handleEvent(const sf::Event &event, const sf::Vector2f &mousePos)
{
    // 时间显示无交互功能
    return false;
}

void TimeDisplay::render(sf::RenderTarget &target, const sf::Vector2f &position)
{
    // 设置时间显示位置
    m_simTimeText.setPosition(position);
    m_realTimeText.setPosition(position + sf::Vector2f(0.f, 14.f));

    // 绘制时间显示
    target.draw(m_simTimeText);
    target.draw(m_realTimeText);
}

// SpeedControl 实现
SpeedControl::SpeedControl(const sf::FloatRect &bounds, const sf::Font &font)
{
    m_bounds = bounds;

    // 设置滑块轨道
    m_track.setSize(sf::Vector2f(bounds.width - 20, 4));
    m_track.setPosition(bounds.left + 10, bounds.top + bounds.height / 2 - 2);
    m_track.setFillColor(sf::Color(100, 100, 100));

    // 设置滑块手柄
    m_handle.setRadius(6);
    m_handle.setOrigin(6, 6);
    updateHandlePosition();
    m_handle.setFillColor(sf::Color(180, 180, 180));

    // 设置标签文本
    m_labelText.setFont(font);
    m_labelText.setString("速度: ");
    m_labelText.setCharacterSize(12);
    m_labelText.setFillColor(sf::Color::White);
    m_labelText.setPosition(bounds.left, bounds.top);

    // 设置值文本
    m_valueText.setFont(font);
    m_valueText.setCharacterSize(12);
    m_valueText.setFillColor(sf::Color::White);
    updateValueText();
    m_valueText.setPosition(bounds.left + 40, bounds.top);
}

void SpeedControl::setValue(float value)
{
    // 确保值在有效范围内
    m_currentValue = std::max(m_minValue, std::min(m_maxValue, value));

    // 更新滑块位置和文本
    updateHandlePosition();
    updateValueText();
}

bool SpeedControl::handleEvent(const sf::Event &event, const sf::Vector2f &mousePos)
{
    // 处理鼠标按下事件
    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
    {
        // 检查是否点击了滑块手柄
        float handleX = m_handle.getPosition().x;
        float handleY = m_handle.getPosition().y;
        float distance = std::sqrt(std::pow(mousePos.x - handleX, 2) + std::pow(mousePos.y - handleY, 2));

        if (distance <= m_handle.getRadius())
        {
            m_isDragging = true;
            return true;
        }

        // 也可以直接点击轨道上的位置
        if (mousePos.y >= m_track.getPosition().y - 10 &&
            mousePos.y <= m_track.getPosition().y + m_track.getSize().y + 10 &&
            mousePos.x >= m_track.getPosition().x &&
            mousePos.x <= m_track.getPosition().x + m_track.getSize().x)
        {

            // 直接更新值并设置拖动状态
            updateValueFromPosition(mousePos.x);
            m_isDragging = true;
            return true;
        }
    }
    // 处理鼠标释放事件
    else if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left)
    {
        if (m_isDragging)
        {
            m_isDragging = false;
            return true;
        }
    }
    // 处理鼠标移动事件
    else if (event.type == sf::Event::MouseMoved && m_isDragging)
    {
        updateValueFromPosition(mousePos.x);
        return true;
    }

    return false;
}

void SpeedControl::render(sf::RenderTarget &target, const sf::Vector2f &position)
{
    // 设置速度控制位置
    m_track.setPosition(position);
    m_handle.setPosition(position + sf::Vector2f(10.f, position.y + 2.f));
    m_labelText.setPosition(position + sf::Vector2f(0.f, -20.f));
    m_valueText.setPosition(position + sf::Vector2f(m_track.getSize().x + 10.f, 0.f));

    // 绘制速度控制
    target.draw(m_track);
    target.draw(m_handle);
    target.draw(m_labelText);
    target.draw(m_valueText);
}

void SpeedControl::updateHandlePosition()
{
    // 计算滑块位置
    float normalizedValue = (m_currentValue - m_minValue) / (m_maxValue - m_minValue);
    float handleX = m_track.getPosition().x + normalizedValue * m_track.getSize().x;
    float handleY = m_track.getPosition().y + m_track.getSize().y / 2;
    m_handle.setPosition(handleX, handleY);
}

void SpeedControl::updateValueText()
{
    std::ostringstream valueStr;
    valueStr << std::fixed << std::setprecision(1) << m_currentValue << "x";
    m_valueText.setString(valueStr.str());
}

void SpeedControl::updateValueFromPosition(float xPos)
{
    // 计算位置对应的值
    float trackStart = m_track.getPosition().x;
    float trackEnd = trackStart + m_track.getSize().x;
    float normalizedPos = (xPos - trackStart) / (trackEnd - trackStart);
    normalizedPos = std::max(0.0f, std::min(1.0f, normalizedPos));

    // 计算并设置新值
    float newValue = m_minValue + normalizedPos * (m_maxValue - m_minValue);
    setValue(newValue);

    // 如果有回调函数，通知值变化
    if (m_onValueChanged)
    {
        m_onValueChanged(m_currentValue);
    }
}
