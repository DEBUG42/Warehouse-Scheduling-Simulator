#pragma once
#include <SFML/Graphics.hpp>
#include <functional>
#include <string>
#include "gui/SimulationInterface.hpp"
#include <vector>

/**
 * @brief 现代化工具栏类
 *
 * 重新设计的工具栏，解决文字可见性和交互问题
 * 采用分段布局，确保组件之间无重叠
 * 自动调整文字颜色以保证可读性
 */
class Toolbar
{
private:
    // 核心属性
    sf::Font &m_font;
    float m_width;
    float m_height;

    // 背景和分割
    sf::RectangleShape m_background;
    sf::RectangleShape m_separator;

    // 播放控制区域
    sf::RectangleShape m_playButton;
    sf::Text m_playButtonText;
    bool m_isPlaying;
    sf::FloatRect m_playButtonBounds;

    // 时间显示区域
    sf::RectangleShape m_timeBackground;
    sf::Text m_timeText;
    sf::FloatRect m_timeAreaBounds;

    // 速度控制区域
    sf::RectangleShape m_speedSliderTrack;
    sf::RectangleShape m_speedSliderHandle;
    sf::Text m_speedLabel;
    sf::Text m_speedValue;
    sf::FloatRect m_speedAreaBounds;
    float m_currentSpeed;
    float m_minSpeed;
    float m_maxSpeed;
    bool m_isDragging;

    // 仿真模式切换按钮区域
    struct ModeButton
    {
        sf::RectangleShape button;
        sf::Text text;
        sf::FloatRect bounds;
        SimulationMode mode;
        bool isActive;
    };

    std::vector<ModeButton> m_modeButtons;
    sf::FloatRect m_modeAreaBounds;
    SimulationMode m_currentMode;

    // 布局常量
    static constexpr float PADDING = 10.0f;
    static constexpr float BUTTON_WIDTH = 80.0f;
    static constexpr float TIME_AREA_WIDTH = 200.0f;
    static constexpr float SPEED_AREA_WIDTH = 250.0f;
    static constexpr float MODE_AREA_WIDTH = 350.0f;
    static constexpr float MODE_BUTTON_WIDTH = 70.0f;
    static constexpr float COMPONENT_HEIGHT = 30.0f; // 回调函数
    std::function<void(float)> m_onTimeScaleChanged;
    std::function<void()> m_onPlayPauseToggled;
    std::function<void(SimulationMode)> m_onModeChanged;

    // 智能颜色选择
    sf::Color getOptimalTextColor(const sf::Color &backgroundColor) const;
    sf::Color getContrastColor(const sf::Color &color) const; // 布局计算
    void calculateLayout();
    void updateSpeedSlider();
    void initializeModeButtons();

    // 格式化函数
    std::string formatTime(float seconds) const;
    std::string formatSpeed(float speed) const;

public:
    /**
     * @brief 构造函数
     * @param font 字体引用
     * @param width 工具栏宽度
     * @param height 工具栏高度
     */
    Toolbar(sf::Font &font, float width, float height);

    /**
     * @brief 处理事件
     * @param event SFML事件
     * @param mousePos 鼠标位置
     * @return 是否消费了事件
     */
    bool handleEvent(const sf::Event &event, const sf::Vector2f &mousePos);

    /**
     * @brief 渲染工具栏
     * @param target 渲染目标
     * @param position 位置
     */
    void render(sf::RenderTarget &target, const sf::Vector2f &position);

    /**
     * @brief 更新时间显示
     * @param simTime 仿真时间（秒）
     */
    void updateTimeDisplay(float simTime);

    /**
     * @brief 设置播放状态
     * @param playing 是否播放
     */
    void setPlaying(bool playing);

    /**
     * @brief 获取当前速度
     * @return 当前倍速值
     */
    float getCurrentSpeed() const { return m_currentSpeed; }

    /**
     * @brief 设置速度范围
     * @param minSpeed 最小速度
     * @param maxSpeed 最大速度
     */
    void setSpeedRange(float minSpeed, float maxSpeed); /**
                                                         * @brief 调整工具栏大小
                                                         * @param width 新的宽度
                                                         */
    void resize(float width);

    /**
     * @brief 设置当前仿真模式
     * @param mode 仿真模式
     */
    void setCurrentMode(SimulationMode mode);

    /**
     * @brief 获取当前仿真模式
     * @return 当前仿真模式
     */
    SimulationMode getCurrentMode() const { return m_currentMode; }

    // 回调设置
    void setOnTimeScaleChanged(std::function<void(float)> callback) { m_onTimeScaleChanged = callback; }
    void setOnPlayPauseToggled(std::function<void()> callback) { m_onPlayPauseToggled = callback; }
    void setOnModeChanged(std::function<void(SimulationMode)> callback) { m_onModeChanged = callback; }

    // 尺寸获取
    float getHeight() const { return m_height; }
    float getWidth() const { return m_width; }
};