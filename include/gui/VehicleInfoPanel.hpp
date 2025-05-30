#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>
#include <string>
#include "../src/Core/Vehicle.hpp"

/**
 * @brief 车辆信息面板类
 *
 * 专门用于显示选中车辆的详细运动信息，包括：
 * - 加减速阶段的起始/终止时间、速度、加速度
 * - 运行统计信息（总运行时间、停车次数等）
 * - 当前运动状态和参数
 */
class VehicleInfoPanel : public sf::Drawable, public sf::Transformable
{
private:
    // 字体和样式
    sf::Font &m_font;
    sf::RectangleShape m_background;
    sf::Color m_backgroundColor;

    // 布局参数
    float m_width;
    float m_height;
    float m_padding;
    float m_lineSpacing;

    // 文本显示对象
    sf::Text m_titleText;
    sf::Text m_motionStateText;
    sf::Text m_currentSpeedText;
    sf::Text m_accelerationText;
    sf::Text m_positionText;

    // 加减速历史信息
    sf::Text m_accelerationHistoryTitle;
    std::vector<sf::Text> m_accelerationHistory;

    // 统计信息
    sf::Text m_statisticsTitle;
    sf::Text m_totalRunTimeText;
    sf::Text m_stopCountText;
    sf::Text m_averageSpeedText;
    // 当前选中的车辆
    const Vehicle *m_selectedVehicle;

    // 加减速事件记录结构
    struct AccelerationEvent
    {
        float startTime;
        float endTime;
        float startSpeed;
        float endSpeed;
        float acceleration;
        std::string type; // "Accelerating", "Decelerating", "Cruising"
    };

    std::vector<AccelerationEvent> m_accelerationEvents;

    // 统计数据
    struct VehicleStatistics
    {
        float totalRunTime = 0.0f;
        int stopCount = 0;
        float averageSpeed = 0.0f;
        float maxSpeed = 0.0f;
        float totalDistance = 0.0f;
    } m_statistics;

public:
    /**
     * @brief 构造函数
     * @param font 字体引用
     * @param width 面板宽度
     * @param height 面板高度
     */
    VehicleInfoPanel(sf::Font &font, float width, float height);
    /**
     * @brief 设置要显示信息的车辆
     * @param vehicle 车辆指针，nullptr表示清空显示
     */
    void setVehicle(const Vehicle *vehicle);

    /**
     * @brief 更新车辆信息显示
     * @param currentTime 当前仿真时间
     */
    void updateInfo(float currentTime);

    /**
     * @brief 记录加减速事件
     * @param startTime 开始时间
     * @param endTime 结束时间
     * @param startSpeed 起始速度
     * @param endSpeed 终止速度
     * @param acceleration 加速度
     * @param type 事件类型
     */
    void recordAccelerationEvent(float startTime, float endTime,
                                 float startSpeed, float endSpeed,
                                 float acceleration, const std::string &type);

    /**
     * @brief 更新统计信息
     * @param deltaTime 时间步长
     */
    void updateStatistics(float deltaTime);

    /**
     * @brief 清空历史记录和统计信息
     */
    void clearHistory();

    /**
     * @brief 设置面板尺寸
     * @param width 新宽度
     * @param height 新高度
     */
    void setSize(float width, float height);

    /**
     * @brief 处理事件
     * @param event SFML事件
     * @param localMousePos 相对于面板的鼠标位置
     * @return 是否处理了事件
     */
    bool handleEvent(const sf::Event &event, const sf::Vector2f &localMousePos);

private:
    /**
     * @brief 初始化UI元素
     */
    void initializeUI();

    /**
     * @brief 更新布局
     */
    void updateLayout();

    /**
     * @brief 更新加减速历史显示
     */
    void updateAccelerationHistoryDisplay();

    /**
     * @brief 格式化时间显示
     * @param seconds 秒数
     * @return 格式化的时间字符串
     */
    std::string formatTime(float seconds) const;

    /**
     * @brief 格式化速度显示
     * @param speed 速度（米/秒）
     * @return 格式化的速度字符串
     */
    std::string formatSpeed(float speed) const;

    /**
     * @brief 将车辆状态转换为字符串
     * @param status 状态枚举值
     * @return 状态字符串
     */
    std::string vehicleStatusToString(int status) const;

    /**
     * @brief 渲染函数（继承自sf::Drawable）
     */
    virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const override;
};
