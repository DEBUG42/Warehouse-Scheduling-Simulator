#include "gui/Toolbar.hpp"
#include "gui/UIControls.hpp"
#include <iostream>

Toolbar::Toolbar(sf::Font &font, float height, float width)
    : m_height(height), m_width(width), m_font(font)
{
    // Initialize toolbar background
    m_background.setSize(sf::Vector2f(width, height));
    m_background.setFillColor(sf::Color(50, 50, 50));

    // Create play/pause button
    sf::FloatRect playBtnBounds(10, 5, 20, 20);
    m_playPauseButton = std::make_unique<Button>(playBtnBounds, font, "Play");

    // Create time display
    sf::FloatRect timeDisplayBounds(40, 5, 150, 20);
    m_timeDisplay = std::make_unique<TimeDisplay>(timeDisplayBounds, font);

    // Create speed control slider
    sf::FloatRect speedControlBounds(200, 5, 150, 20);
    m_speedControl = std::make_unique<SpeedControl>(speedControlBounds, font, 0.1f, 15.0f, 1.0f);

    // Create extra function buttons
    createFunctionButtons();
}

void Toolbar::createFunctionButtons()
{
    float buttonWidth = 80;
    float buttonHeight = 20;
    float startX = 370;
    float spacing = 10;

    // Add Task button
    sf::FloatRect addTaskBounds(startX, 5, buttonWidth, buttonHeight);
    auto addTaskBtn = std::make_unique<Button>(addTaskBounds, m_font, "Add Task");
    addTaskBtn->setCallback([this]()
                            {
        if (m_onAddTask) m_onAddTask(); });
    m_buttons.push_back(std::move(addTaskBtn));

    // Reset View button
    sf::FloatRect resetViewBounds(startX + buttonWidth + spacing, 5, buttonWidth, buttonHeight);
    auto resetViewBtn = std::make_unique<Button>(resetViewBounds, m_font, "Reset View");
    resetViewBtn->setCallback([this]()
                              {
        if (m_onResetView) m_onResetView(); });//如果确定含有回调函数，则调用它
    m_buttons.push_back(std::move(resetViewBtn));

    // Switch Mode button
    sf::FloatRect switchModeBounds(startX + 2 * (buttonWidth + spacing), 5, buttonWidth, buttonHeight);
    auto switchModeBtn = std::make_unique<Button>(switchModeBounds, m_font, "Switch Mode");
    switchModeBtn->setCallback([this]()
                               {
        if (m_onSwitchMode) m_onSwitchMode(); });
    m_buttons.push_back(std::move(switchModeBtn));
}

bool Toolbar::handleEvent(const sf::Event &event, const sf::Vector2f &mousePos)
{
    // 检查鼠标是否在工具栏区域内
    if (mousePos.y > m_height)
    {
        return false; // 不在工具栏区域
    }

    // 处理播放/暂停按钮事件
    if (m_playPauseButton->handleEvent(event, mousePos))
    {
        return true;
    }

    // 处理速度控制滑块事件
    if (m_speedControl->handleEvent(event, mousePos))
    {
        // 如果注册了回调，通知时间缩放变化
        if (m_onTimeScaleChanged)
        {
            m_onTimeScaleChanged(m_speedControl->getValue());
        }
        return true;
    }

    // 处理其他按钮事件
    for (auto &button : m_buttons)
    {
        if (button->handleEvent(event, mousePos))
        {
            return true;
        }
    }

    return false;
}

void Toolbar::updateTimeDisplay(float simTime, float realTime)
{
    m_timeDisplay->updateTime(simTime, realTime);
}

void Toolbar::render(sf::RenderTarget &target, const sf::Vector2f &position)
{
    // 绘制背景
    m_background.setPosition(position);
    target.draw(m_background);

    // 绘制播放/暂停按钮
    m_playPauseButton->render(target, position + sf::Vector2f(10.f, 5.f));

    // 绘制时间显示
    m_timeDisplay->render(target, position + sf::Vector2f(120.f, 5.f));

    // 绘制速度控制
    m_speedControl->render(target, position + sf::Vector2f(300.f, 5.f));

    // 绘制其他按钮（紧跟速度控件后，靠左）
    float buttonX = position.x + 370.f;
    for (auto &button : m_buttons)
    {
        button->render(target, sf::Vector2f(buttonX, position.y + 5.f));
        buttonX += button->getSize().x + 10.f;
    }

    // 绘制分隔线
    sf::RectangleShape divider(sf::Vector2f(m_width, 1));
    divider.setPosition(position.x, position.y + m_height - 1);
    divider.setFillColor(sf::Color(70, 70, 70));
    target.draw(divider);
}

void Toolbar::resize(float width)
{
    m_width = width;
    m_background.setSize(sf::Vector2f(width, m_height));
}

void Toolbar::setTimeScaleCallback(std::function<void(float)> callback)
{
    m_onTimeScaleChanged = callback;
    m_speedControl->setCallback(m_onTimeScaleChanged);
}

void Toolbar::setPlayPauseCallback(std::function<void()> callback)
{
    m_onPlayPauseToggled = callback;
    m_playPauseButton->setCallback(m_onPlayPauseToggled);
}

void Toolbar::setAddTaskCallback(std::function<void()> callback)
{
    m_onAddTask = callback;
}

void Toolbar::setResetViewCallback(std::function<void()> callback)
{
    m_onResetView = callback;
}

void Toolbar::setSwitchModeCallback(std::function<void()> callback)
{
    m_onSwitchMode = callback;
}

void Toolbar::updateTimeScale(float scale)
{
    if (m_speedControl)
    {
        m_speedControl->setValue(scale);
    }
}

void Toolbar::updatePlayPauseState(bool isPlaying)
{
    if (m_playPauseButton)
    {
        if (isPlaying)
        {
            m_playPauseButton->setLabel("Pause");
        }
        else
        {
            m_playPauseButton->setLabel("Play");
        }
    }
    // 注意：这里只改变了按钮的视觉表示，
    // 实际的播放/暂停逻辑需要通过 m_onPlayPauseToggled 回调来触发。
}

float Toolbar::getTimeScaleValue() const
{
    if (m_speedControl)
    {
        return m_speedControl->getValue();
    }
    return 1.0f; // 如果速度控件不存在，返回默认值1.0
}