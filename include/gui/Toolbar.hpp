#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <functional>
#include "UIControls.hpp"

class TimeDisplay;
class SpeedControl;

/**
 * @brief 工具栏类
 *
 * 位于界面顶部，提供仿真控制功能
 * 包括播放/暂停按钮、时间显示、速度调节滑块等组件
 * 处理用户交互并发送控制命令到仿真引擎
 */
class Toolbar
{
private:
    // 控件集合
    std::unique_ptr<Button> m_playPauseButton;      // 播放/暂停按钮
    std::vector<std::unique_ptr<Button>> m_buttons; // 功能按钮
    std::unique_ptr<TimeDisplay> m_timeDisplay;     // 时间显示组件
    std::unique_ptr<SpeedControl> m_speedControl;   // 速度调节滑块

    // 布局参数
    float m_height;                  // 工具栏高度
    float m_width;                   // 工具栏宽度
    sf::RectangleShape m_background; // 背景矩形
    sf::Font &m_font;                // 字体引用

    // 回调函数
    std::function<void(float)> m_onTimeScaleChanged; // 时间缩放回调
    std::function<void()> m_onPlayPauseToggled;      // 播放/暂停回调
    std::function<void()> m_onAddTask;               // 添加任务回调
    std::function<void()> m_onResetView;             // 视图复位回调
    std::function<void()> m_onSwitchMode;            // 切换模式回调

public:
    /**
     * @brief 构造函数
     * @param font 字体引用
     * @param height 工具栏高度
     * @param width 工具栏宽度
     */
    Toolbar(sf::Font &font, float height, float width);

    /**
     * @brief 创建功能按钮
     */
    void createFunctionButtons();

    /**
     * @brief 处理工具栏区域输入事件
     * @param event SFML事件对象
     * @param mousePos 鼠标位置（相对窗口坐标）
     * @return 是否消耗了该事件
     */
    bool handleEvent(const sf::Event &event, const sf::Vector2f &mousePos);

    /**
     * @brief 更新时间显示数值
     * @param simTime 当前仿真时间
     * @param realTime 程序运行时间
     */
    void updateTimeDisplay(float simTime, float realTime);

    /**
     * @brief 渲染工具栏
     * @param target 渲染目标
     * @param position 工具栏左上角位置
     */
    void render(sf::RenderTarget &target, const sf::Vector2f &position);

    /**
     * @brief 调整工具栏宽度
     * @param width 新的宽度值
     */
    void resize(float width);

    /**
     * @brief 设置时间缩放回调
     * @param callback 回调函数
     */
    void setTimeScaleCallback(std::function<void(float)> callback);

    /**
     * @brief 设置播放/暂停回调
     * @param callback 回调函数
     */
    void setPlayPauseCallback(std::function<void()> callback);

    /**
     * @brief 设置添加任务回调
     * @param callback 回调函数
     */
    void setAddTaskCallback(std::function<void()> callback);

    /**
     * @brief 设置视图复位回调
     * @param callback 回调函数
     */
    void setResetViewCallback(std::function<void()> callback);

    /**
     * @brief 设置切换模式回调
     * @param callback 回调函数
     */
    void setSwitchModeCallback(std::function<void()> callback);

    /**
     * @brief 更新时间缩放值
     * @param scale 新的时间缩放值
     */
    void updateTimeScale(float scale);

    /**
     * @brief 更新播放/暂停状态，并改变按钮标签
     * @param isPlaying 是否正在播放
     */
    void updatePlayPauseState(bool isPlaying);

    /**
     * @brief 获取当前时间缩放控件的值
     * @return 当前速度控件设置的时间缩放比例
     */
    float getTimeScaleValue() const;
};