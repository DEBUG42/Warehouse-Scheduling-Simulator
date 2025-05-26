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
    float buttonHeight = m_height - 10; // 使按钮高度适应工具栏，留出边距
    if (buttonHeight < 15)
        buttonHeight = 15;                                                                    // 最小高度
    float startX = m_speedControl->getBounds().left + m_speedControl->getBounds().width + 20; // 从速度控件后开始
    float spacing = 10;
    float currentButtonY = (m_height - buttonHeight) / 2; // 垂直居中

    // Add Task button
    sf::FloatRect addTaskBounds(startX, currentButtonY, buttonWidth, buttonHeight);
    auto addTaskBtn = std::make_unique<Button>(addTaskBounds, m_font, "Add Task");
    addTaskBtn->setCallback([this]() { // 注意捕获 this
        if (m_onAddTask)
            m_onAddTask();
    });
    m_buttons.push_back(std::move(addTaskBtn));

    // Reset View button
    sf::FloatRect resetViewBounds(startX + buttonWidth + spacing, currentButtonY, buttonWidth, buttonHeight);
    auto resetViewBtn = std::make_unique<Button>(resetViewBounds, m_font, "Reset View");
    resetViewBtn->setCallback([this]() { // 注意捕获 this
        if (m_onResetView)
            m_onResetView();
    });
    m_buttons.push_back(std::move(resetViewBtn));

    // Switch Mode button
    sf::FloatRect switchModeBounds(startX + 2 * (buttonWidth + spacing), currentButtonY, buttonWidth, buttonHeight);
    auto switchModeBtn = std::make_unique<Button>(switchModeBounds, m_font, "Switch Mode");
    switchModeBtn->setCallback([this]() { // 注意捕获 this
        if (m_onSwitchMode)
            m_onSwitchMode();
    });
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

void Toolbar::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    states.transform *= getTransform(); // 应用 Toolbar 自身的变换

    // 绘制背景
    target.draw(m_background, states);

    // 绘制播放/暂停按钮
    // 按钮的位置是相对于Toolbar的，所以直接使用其局部坐标进行绘制
    // Button类应该继承sf::Drawable并有自己的draw方法
    if (m_playPauseButton)
        target.draw(*m_playPauseButton, states);

    // 绘制时间显示
    if (m_timeDisplay)
        target.draw(*m_timeDisplay, states);

    // 绘制速度控制
    if (m_speedControl)
        target.draw(*m_speedControl, states);

    // 绘制其他按钮
    for (const auto &button : m_buttons)
    {
        if (button)
            target.draw(*button, states);
    }

    // 绘制分隔线 (可选，如果背景和按钮已有足够区分度)
    // sf::RectangleShape divider(sf::Vector2f(m_width, 1));
    // divider.setPosition(0, m_height - 1); // 相对于Toolbar的(0,0)
    // divider.setFillColor(sf::Color(70, 70, 70));
    // target.draw(divider, states);
}

void Toolbar::resize(float width)
{
    m_width = width;
    m_background.setSize(sf::Vector2f(width, m_height));
    // TODO: 可能需要重新计算和调整内部控件的位置和大小
    // 例如，如果按钮是动态布局的，这里需要更新它们的 bounds
    // createFunctionButtons(); // 如果按钮位置依赖于宽度，可以重新创建/定位它们
    // 或者更精细地调整每个控件的位置
    if (m_speedControl)
    {
        // 示例：如果速度控件要保持在时间显示右边，并有一定间距
        // float newSpeedControlX = m_timeDisplay->getBounds().left + m_timeDisplay->getBounds().width + 10.f;
        // m_speedControl->setPosition(newSpeedControlX, m_speedControl->getBounds().top);
    }
    // 重新定位功能按钮 (一个简单的例子，可能需要更复杂的逻辑)
    float buttonWidth = 80; // 假设值，应与createFunctionButtons中一致
    float spacing = 10;
    float startX = m_speedControl ? (m_speedControl->getBounds().left + m_speedControl->getBounds().width + 20) : 200.f;
    float currentButtonY = (m_height - (m_height - 10)) / 2; // 假设值
    if (!m_buttons.empty())
    {
        m_buttons[0]->setBounds(sf::FloatRect(startX, currentButtonY, buttonWidth, m_height - 10));
        if (m_buttons.size() > 1)
        {
            m_buttons[1]->setBounds(sf::FloatRect(startX + buttonWidth + spacing, currentButtonY, buttonWidth, m_height - 10));
        }
        if (m_buttons.size() > 2)
        {
            m_buttons[2]->setBounds(sf::FloatRect(startX + 2 * (buttonWidth + spacing), currentButtonY, buttonWidth, m_height - 10));
        }
    }
}

void Toolbar::setTimeScaleCallback(std::function<void(float)> callback)
{
    m_onTimeScaleChanged = callback;
    if (m_speedControl)
    {                                                      // 确保控件存在
        m_speedControl->setCallback(m_onTimeScaleChanged); // SpeedControl也需要一个setCallback
    }
}

void Toolbar::setPlayPauseCallback(std::function<void()> callback)
{
    m_onPlayPauseToggled = callback;
    if (m_playPauseButton)
    { // 确保控件存在
        m_playPauseButton->setCallback(m_onPlayPauseToggled);
    }
}

void Toolbar::setAddTaskCallback(std::function<void()> callback)
{
    m_onAddTask = callback;
    // 之前在 createFunctionButtons 中已经通过lambda设置了按钮回调，这里存储的是 MainWindow 传来的回调
    // 如果按钮在创建时就直接使用 m_onAddTask，则不需要在这里再次设置按钮的物理回调
    // 当前的 createFunctionButtons 实现是正确的，它捕获 this 并调用成员 m_onAddTask
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