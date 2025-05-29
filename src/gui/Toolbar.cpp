#include "gui/Toolbar.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <cmath>

Toolbar::Toolbar(sf::Font &font, float width, float height)
    : m_font(font), m_width(width), m_height(height),
      m_isPlaying(false), m_isDragging(false),
      m_currentSpeed(1.0f), m_minSpeed(0.1f), m_maxSpeed(10.0f)
{
    calculateLayout();

    // 设置背景 - 使用现代深色主题
    m_background.setSize(sf::Vector2f(m_width, m_height));
    m_background.setFillColor(sf::Color(45, 45, 45)); // 深灰色背景

    // 设置分隔线
    m_separator.setSize(sf::Vector2f(2.0f, COMPONENT_HEIGHT));
    m_separator.setFillColor(sf::Color(80, 80, 80));

    // 初始化播放按钮
    m_playButton.setSize(sf::Vector2f(BUTTON_WIDTH, COMPONENT_HEIGHT));
    m_playButton.setFillColor(sf::Color(70, 130, 180)); // 钢蓝色

    m_playButtonText.setFont(m_font);
    m_playButtonText.setCharacterSize(14);
    m_playButtonText.setFillColor(sf::Color::White);
    m_playButtonText.setString("▶ Play");

    // 初始化时间显示
    m_timeBackground.setSize(sf::Vector2f(TIME_AREA_WIDTH, COMPONENT_HEIGHT));
    m_timeBackground.setFillColor(sf::Color(35, 35, 35)); // 更深的背景

    m_timeText.setFont(m_font);
    m_timeText.setCharacterSize(14);
    m_timeText.setFillColor(sf::Color(200, 200, 200)); // 浅灰色文字
    m_timeText.setString("Time: 00:00:00.000");

    // 初始化速度控制
    m_speedSliderTrack.setSize(sf::Vector2f(150.0f, 6.0f));
    m_speedSliderTrack.setFillColor(sf::Color(60, 60, 60));

    m_speedSliderHandle.setSize(sf::Vector2f(12.0f, 20.0f));
    m_speedSliderHandle.setFillColor(sf::Color(100, 150, 255));

    m_speedLabel.setFont(m_font);
    m_speedLabel.setCharacterSize(12);
    m_speedLabel.setFillColor(sf::Color(180, 180, 180));
    m_speedLabel.setString("Speed:");

    m_speedValue.setFont(m_font);
    m_speedValue.setCharacterSize(12);
    m_speedValue.setFillColor(sf::Color(180, 180, 180));
    m_speedValue.setString(formatSpeed(m_currentSpeed));

    updateSpeedSlider();
}

void Toolbar::calculateLayout()
{
    float currentX = PADDING;
    float centerY = m_height / 2.0f;
    float componentY = centerY - COMPONENT_HEIGHT / 2.0f;

    // 播放按钮区域
    m_playButtonBounds = sf::FloatRect(currentX, componentY, BUTTON_WIDTH, COMPONENT_HEIGHT);
    currentX += BUTTON_WIDTH + PADDING;

    // 时间显示区域
    m_timeAreaBounds = sf::FloatRect(currentX, componentY, TIME_AREA_WIDTH, COMPONENT_HEIGHT);
    currentX += TIME_AREA_WIDTH + PADDING;

    // 速度控制区域
    m_speedAreaBounds = sf::FloatRect(currentX, componentY, SPEED_AREA_WIDTH, COMPONENT_HEIGHT);
}

void Toolbar::updateSpeedSlider()
{
    // 计算滑块位置
    float trackStart = m_speedAreaBounds.left + 60.0f; // 给标签留空间
    float trackWidth = 150.0f;
    float normalizedPos = (m_currentSpeed - m_minSpeed) / (m_maxSpeed - m_minSpeed);
    float handleX = trackStart + normalizedPos * trackWidth - m_speedSliderHandle.getSize().x / 2.0f;

    m_speedSliderHandle.setPosition(handleX, m_speedAreaBounds.top + (COMPONENT_HEIGHT - 20.0f) / 2.0f);
    m_speedSliderTrack.setPosition(trackStart, m_speedAreaBounds.top + (COMPONENT_HEIGHT - 6.0f) / 2.0f);
}

sf::Color Toolbar::getOptimalTextColor(const sf::Color &backgroundColor) const
{
    // 计算背景亮度
    float luminance = (0.299f * backgroundColor.r + 0.587f * backgroundColor.g + 0.114f * backgroundColor.b) / 255.0f;

    // 根据亮度选择文字颜色
    if (luminance > 0.5f)
    {
        return sf::Color(30, 30, 30); // 深色文字
    }
    else
    {
        return sf::Color(220, 220, 220); // 浅色文字
    }
}

sf::Color Toolbar::getContrastColor(const sf::Color &color) const
{
    // 简单的对比色计算
    return sf::Color(255 - color.r, 255 - color.g, 255 - color.b);
}

std::string Toolbar::formatTime(float seconds) const
{
    int hours = static_cast<int>(seconds / 3600);
    int minutes = static_cast<int>((seconds - hours * 3600) / 60);
    int secs = static_cast<int>(seconds) % 60;
    int milliseconds = static_cast<int>((seconds - static_cast<int>(seconds)) * 1000);

    std::ostringstream ss;
    ss << std::setfill('0') << std::setw(2) << hours << ":"
       << std::setw(2) << minutes << ":"
       << std::setw(2) << secs << "."
       << std::setw(3) << milliseconds;
    return ss.str();
}

std::string Toolbar::formatSpeed(float speed) const
{
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(1) << speed << "x";
    return ss.str();
}

bool Toolbar::handleEvent(const sf::Event &event, const sf::Vector2f &mousePos)
{
    // 检查是否在工具栏区域内
    if (mousePos.y > m_height)
    {
        return false;
    }

    // 处理播放按钮
    if (m_playButtonBounds.contains(mousePos))
    {
        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
        {
            m_isPlaying = !m_isPlaying;
            m_playButtonText.setString(m_isPlaying ? "⏸ Pause" : "▶ Play");

            if (m_onPlayPauseToggled)
            {
                m_onPlayPauseToggled();
            }
            return true;
        }
    }

    // 处理速度滑块
    if (m_speedAreaBounds.contains(mousePos))
    {
        float trackStart = m_speedAreaBounds.left + 60.0f;
        float trackWidth = 150.0f;
        sf::FloatRect trackBounds(trackStart, m_speedAreaBounds.top, trackWidth, COMPONENT_HEIGHT);

        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
        {
            if (trackBounds.contains(mousePos))
            {
                m_isDragging = true;

                // 计算新的速度值
                float normalizedPos = (mousePos.x - trackStart) / trackWidth;
                normalizedPos = std::max(0.0f, std::min(1.0f, normalizedPos));
                m_currentSpeed = m_minSpeed + normalizedPos * (m_maxSpeed - m_minSpeed);

                updateSpeedSlider();
                m_speedValue.setString(formatSpeed(m_currentSpeed));

                if (m_onTimeScaleChanged)
                {
                    m_onTimeScaleChanged(m_currentSpeed);
                }
                return true;
            }
        }

        if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left)
        {
            m_isDragging = false;
            return true;
        }

        if (event.type == sf::Event::MouseMoved && m_isDragging)
        {
            // 拖拽滑块
            float normalizedPos = (mousePos.x - trackStart) / trackWidth;
            normalizedPos = std::max(0.0f, std::min(1.0f, normalizedPos));
            m_currentSpeed = m_minSpeed + normalizedPos * (m_maxSpeed - m_minSpeed);

            updateSpeedSlider();
            m_speedValue.setString(formatSpeed(m_currentSpeed));

            if (m_onTimeScaleChanged)
            {
                m_onTimeScaleChanged(m_currentSpeed);
            }
            return true;
        }
    }

    return false;
}

void Toolbar::render(sf::RenderTarget &target, const sf::Vector2f &position)
{
    // 绘制背景
    m_background.setPosition(position);
    target.draw(m_background);

    // 绘制播放按钮
    m_playButton.setPosition(position.x + m_playButtonBounds.left, position.y + m_playButtonBounds.top);
    target.draw(m_playButton);

    // 播放按钮文字居中
    sf::FloatRect textBounds = m_playButtonText.getLocalBounds();
    m_playButtonText.setPosition(
        position.x + m_playButtonBounds.left + (BUTTON_WIDTH - textBounds.width) / 2.0f,
        position.y + m_playButtonBounds.top + (COMPONENT_HEIGHT - textBounds.height) / 2.0f - 2.0f);
    target.draw(m_playButtonText);

    // 绘制分隔线1
    m_separator.setPosition(
        position.x + m_playButtonBounds.left + BUTTON_WIDTH + PADDING / 2.0f,
        position.y + m_playButtonBounds.top);
    target.draw(m_separator);

    // 绘制时间显示背景
    m_timeBackground.setPosition(position.x + m_timeAreaBounds.left, position.y + m_timeAreaBounds.top);
    target.draw(m_timeBackground);

    // 绘制时间文字
    m_timeText.setPosition(
        position.x + m_timeAreaBounds.left + 10.0f,
        position.y + m_timeAreaBounds.top + (COMPONENT_HEIGHT - 16.0f) / 2.0f);
    target.draw(m_timeText);

    // 绘制分隔线2
    m_separator.setPosition(
        position.x + m_timeAreaBounds.left + TIME_AREA_WIDTH + PADDING / 2.0f,
        position.y + m_timeAreaBounds.top);
    target.draw(m_separator);

    // 绘制速度标签
    m_speedLabel.setPosition(
        position.x + m_speedAreaBounds.left + 5.0f,
        position.y + m_speedAreaBounds.top + (COMPONENT_HEIGHT - 12.0f) / 2.0f);
    target.draw(m_speedLabel);

    // 绘制速度滑块轨道
    m_speedSliderTrack.setPosition(
        position.x + m_speedSliderTrack.getPosition().x,
        position.y + m_speedSliderTrack.getPosition().y);
    target.draw(m_speedSliderTrack);

    // 绘制速度滑块手柄
    m_speedSliderHandle.setPosition(
        position.x + m_speedSliderHandle.getPosition().x,
        position.y + m_speedSliderHandle.getPosition().y);
    target.draw(m_speedSliderHandle);

    // 绘制速度值
    m_speedValue.setPosition(
        position.x + m_speedAreaBounds.left + 220.0f,
        position.y + m_speedAreaBounds.top + (COMPONENT_HEIGHT - 12.0f) / 2.0f);
    target.draw(m_speedValue);
}

void Toolbar::updateTimeDisplay(float simTime)
{
    m_timeText.setString("Time: " + formatTime(simTime));
}

void Toolbar::setPlaying(bool playing)
{
    m_isPlaying = playing;
    m_playButtonText.setString(m_isPlaying ? "⏸ Pause" : "▶ Play");
}

void Toolbar::setSpeedRange(float minSpeed, float maxSpeed)
{
    m_minSpeed = minSpeed;
    m_maxSpeed = maxSpeed;
    updateSpeedSlider();
}

void Toolbar::resize(float width)
{
    m_width = width;
    calculateLayout();

    // 重新设置背景大小
    m_background.setSize(sf::Vector2f(m_width, m_height));

    // 重新计算布局
    updateSpeedSlider();
}