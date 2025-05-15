#include "Toolbar.hpp"
#include <SFML/Graphics.hpp>

/**
 * @brief 按钮类构造函数
 * @param size 按钮尺寸
 * @param font 使用的字体
 * @param text 初始显示文本
 */
Button::Button(const sf::Vector2f& size, const sf::Font& font, const std::string& text)
    : m_shape(size), m_text(text, font, 16) {
    // 初始化按钮样式
    m_shape.setFillColor(sf::Color(200, 200, 200));
    m_shape.setOutlineThickness(1);
    m_shape.setOutlineColor(sf::Color::Black);

    // 计算并居中文本
    sf::FloatRect textBounds = m_text.getLocalBounds();
    m_text.setOrigin(textBounds.left + textBounds.width / 2.0f, 
                    textBounds.top + textBounds.height / 2.0f);
    m_text.setPosition(size.x / 2.0f, size.y / 2.0f);
    m_text.setFillColor(sf::Color::Black);
}

/**
 * @brief 检测点是否在按钮范围内
 * @param point 要检测的坐标点
 * @return 如果在范围内返回true，否则false
 */
bool Button::contains(const sf::Vector2f& point) const {
    return m_shape.getGlobalBounds().contains(point);
}

/**
 * @brief 绘制按钮到渲染目标
 * @param target 要绘制到的渲染目标
 */
void Button::draw(sf::RenderTarget& target) const {
    target.draw(m_shape);
    target.draw(m_text);
}

/**
 * @brief 设置按钮显示文本
 * @param text 新文本内容
 */
void Button::setText(const std::string& text) {
    m_text.setString(text);
}

/**
 * @brief 设置按钮位置
 * @param x X轴坐标
 * @param y Y轴坐标
 */
void Button::setPosition(float x, float y) {
    m_shape.setPosition(x, y);
    // 保持文本在按钮中心
    m_text.setPosition(m_shape.getPosition().x + m_shape.getSize().x / 2.0f, 
                      m_shape.getPosition().y + m_shape.getSize().y / 2.0f);
}

/**
 * @brief 时间显示组件构造函数
 * @param font 使用的字体
 */
TimeDisplay::TimeDisplay(const sf::Font& font) : m_font(font) {
    m_text.setFont(m_font);
    m_text.setFillColor(sf::Color::Black);
    m_text.setCharacterSize(20);
}

/**
 * @brief 更新时间显示内容
 * @param simTime 仿真时间（秒）
 * @param realTime 实际时间（秒）
 */
void TimeDisplay::updateTime(float simTime, float realTime) {
    m_text.setString("Sim Time: " + std::to_string(simTime) + "s, Real Time: " + std::to_string(realTime) + "s");
    // 保持文本居中
    sf::FloatRect textRect = m_text.getLocalBounds();
    m_text.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
}

/**
 * @brief 绘制时间显示组件
 * @param target 渲染目标
 * @param states 渲染状态
 */
void TimeDisplay::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    states.transform *= getTransform();
    target.draw(m_text, states);
}

/**
 * @brief 速度控制组件构造函数
 * @param position 组件位置（左上角）
 * @param minSpeed 最小速度值
 * @param maxSpeed 最大速度值
 */
SpeedControl::SpeedControl(const sf::Vector2f& position, float minSpeed, float maxSpeed)
    : m_track(sf::Vector2f(160.0f, 20.0f)), m_thumb(16.0f), m_minSpeed(minSpeed), m_maxSpeed(maxSpeed), m_currentSpeed(1.0f) {
    // 初始化轨道样式
    m_track.setFillColor(sf::Color(160, 160, 160));
    m_track.setOutlineThickness(1);
    m_track.setOutlineColor(sf::Color::Black);
    m_track.setPosition(position);

    // 初始化滑块位置
    m_thumb.setFillColor(sf::Color::Red);
    float initialX = ((1.0f - minSpeed) / (maxSpeed - minSpeed)) * (m_track.getSize().x - m_thumb.getRadius() * 2);
    m_thumb.setPosition(position.x + initialX, position.y - 8);
}

/**
 * @brief 处理速度控制组件的事件
 * @param event SFML事件对象
 * @param mousePos 鼠标当前位置
 * @return 如果事件被处理返回true
 */
bool SpeedControl::handleEvent(const sf::Event& event, const sf::Vector2f& mousePos) {
    // 处理滑块拖拽
    if (event.type == sf::Event::MouseButtonPressed && 
        m_thumb.getGlobalBounds().contains(mousePos)) {
        return true;
    }

    if (event.type == sf::Event::MouseMoved && 
        sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
        // 限制滑块移动范围
        float newX = mousePos.x - m_track.getPosition().x;
        newX = std::max(0.0f, std::min(newX, m_track.getSize().x - m_thumb.getRadius() * 2));
        m_thumb.setPosition(m_track.getPosition().x + newX, m_thumb.getPosition().y);

        // 计算当前速度值
        m_currentSpeed = m_minSpeed + (newX / (m_track.getSize().x - m_thumb.getRadius() * 2)) * (m_maxSpeed - m_minSpeed);
        return true;
    }

    return false;
}

/**
 * @brief 获取当前速度值
 * @return 当前设置的速度倍率
 */
float SpeedControl::getSpeed() const {
    return m_currentSpeed;
}

/**
 * @brief 绘制速度控制组件
 * @param target 渲染目标
 */
void SpeedControl::draw(sf::RenderTarget& target) const {
    target.draw(m_track);
    target.draw(m_thumb);
}

/**
 * @brief 工具栏构造函数
 * @param font 全局使用的字体
 */
Toolbar::Toolbar(const sf::Font& font) : m_font(font), m_isPaused(false) {
    // 初始化工具栏背景
    m_background.setSize(sf::Vector2f(1000, 50));
    m_background.setFillColor(sf::Color(180, 180, 180));

    // 创建暂停/开始按钮
    m_buttons.push_back(std::make_unique<Button>(sf::Vector2f(80, 30), m_font, "Pause"));
    m_buttons.back()->setPosition(10, 10);

    // 初始化时间显示组件
    m_timeDisplay = std::make_unique<TimeDisplay>(m_font);
    m_timeDisplay->setPosition(400, 25);

    // 初始化速度控制组件
    m_speedControl = std::make_unique<SpeedControl>(sf::Vector2f(800, 15), 0.5f, 2.0f);

    // 初始化速度显示文本
    m_speedText.setFont(m_font);
    m_speedText.setFillColor(sf::Color::Black);
    m_speedText.setCharacterSize(20);
    m_speedText.setPosition(800, 10);
}

/**
 * @brief 处理工具栏事件
 * @param event SFML事件对象
 * @param mousePos 鼠标当前位置
 * @return 如果事件被消耗返回true
 */
bool Toolbar::handleEvent(const sf::Event& event, const sf::Vector2f& mousePos) {
    // 处理按钮点击事件
    for (auto& button : m_buttons) {
        if (button->contains(mousePos)) {
            if (event.type == sf::Event::MouseButtonPressed && 
                event.mouseButton.button == sf::Mouse::Left) {
                // 切换暂停状态并更新按钮文本
                m_isPaused = !m_isPaused;
                button->setText(m_isPaused ?  "Start":"Pause" );
                return true;
            }
        }
    }
    return false;
}

/**
 * @brief 更新时间显示内容
 * @param simTime 当前仿真时间（秒）
 * @param realTime 实际运行时间（秒）
 */
void Toolbar::updateTimeDisplay(float simTime, float realTime) {
    if (!m_isPaused) {
        m_timeDisplay->updateTime(simTime, realTime);
    }
    
    // 更新速度显示文本
    float currentSpeed = m_speedControl->getSpeed();
    m_speedText.setString("Speed: " + std::to_string(currentSpeed) + "x");
}

/**
 * @brief 绘制整个工具栏
 * @param target 渲染目标
 */
void Toolbar::draw(sf::RenderTarget& target) const {
    target.draw(m_background);
    for (const auto& button : m_buttons) {
        button->draw(target);
    }
    target.draw(*m_timeDisplay);
    m_speedControl->draw(target);
    target.draw(m_speedText);
}