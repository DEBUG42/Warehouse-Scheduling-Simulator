#include "gui/VehicleInfoPanel.hpp"
#include "gui/UIControls.hpp" // 使用UIUtils的格式化函数
#include <sstream>
#include <iomanip>
#include <algorithm>

VehicleInfoPanel::VehicleInfoPanel(sf::Font &font, float width, float height)
    : m_font(font), m_width(width), m_height(height), m_selectedVehicle(nullptr)
{
    // 设置布局参数
    m_padding = 10.0f;
    m_lineSpacing = 18.0f;
    m_backgroundColor = sf::Color(245, 245, 245, 230); // 浅灰色半透明背景

    // 初始化背景
    m_background.setSize(sf::Vector2f(m_width, m_height));
    m_background.setFillColor(m_backgroundColor);
    m_background.setOutlineThickness(1.0f);
    m_background.setOutlineColor(sf::Color(120, 120, 120));

    initializeUI();
    updateLayout();
}

void VehicleInfoPanel::initializeUI()
{
    // 标题
    m_titleText.setFont(m_font);
    m_titleText.setCharacterSize(14);
    m_titleText.setFillColor(sf::Color(50, 50, 50));
    m_titleText.setString("Vehicle Information");
    m_titleText.setStyle(sf::Text::Bold);

    // 当前状态信息
    m_motionStateText.setFont(m_font);
    m_motionStateText.setCharacterSize(12);
    m_motionStateText.setFillColor(sf::Color(70, 70, 70));

    m_currentSpeedText.setFont(m_font);
    m_currentSpeedText.setCharacterSize(12);
    m_currentSpeedText.setFillColor(sf::Color(70, 70, 70));

    m_accelerationText.setFont(m_font);
    m_accelerationText.setCharacterSize(12);
    m_accelerationText.setFillColor(sf::Color(70, 70, 70));

    m_positionText.setFont(m_font);
    m_positionText.setCharacterSize(12);
    m_positionText.setFillColor(sf::Color(70, 70, 70));

    // 加减速历史标题
    m_accelerationHistoryTitle.setFont(m_font);
    m_accelerationHistoryTitle.setCharacterSize(13);
    m_accelerationHistoryTitle.setFillColor(sf::Color(50, 50, 50));
    m_accelerationHistoryTitle.setString("Acceleration History:");
    m_accelerationHistoryTitle.setStyle(sf::Text::Bold);

    // 统计信息标题
    m_statisticsTitle.setFont(m_font);
    m_statisticsTitle.setCharacterSize(13);
    m_statisticsTitle.setFillColor(sf::Color(50, 50, 50));
    m_statisticsTitle.setString("Statistics:");
    m_statisticsTitle.setStyle(sf::Text::Bold);

    // 统计信息文本
    m_totalRunTimeText.setFont(m_font);
    m_totalRunTimeText.setCharacterSize(11);
    m_totalRunTimeText.setFillColor(sf::Color(70, 70, 70));

    m_stopCountText.setFont(m_font);
    m_stopCountText.setCharacterSize(11);
    m_stopCountText.setFillColor(sf::Color(70, 70, 70));

    m_averageSpeedText.setFont(m_font);
    m_averageSpeedText.setCharacterSize(11);
    m_averageSpeedText.setFillColor(sf::Color(70, 70, 70));
}

void VehicleInfoPanel::updateLayout()
{
    float currentY = m_padding;

    // 标题位置
    m_titleText.setPosition(m_padding, currentY);
    currentY += m_lineSpacing * 1.5f;

    // 当前状态信息位置
    m_motionStateText.setPosition(m_padding, currentY);
    currentY += m_lineSpacing;

    m_currentSpeedText.setPosition(m_padding, currentY);
    currentY += m_lineSpacing;

    m_accelerationText.setPosition(m_padding, currentY);
    currentY += m_lineSpacing;

    m_positionText.setPosition(m_padding, currentY);
    currentY += m_lineSpacing * 1.5f;

    // 加减速历史标题
    m_accelerationHistoryTitle.setPosition(m_padding, currentY);
    currentY += m_lineSpacing;

    // 更新加减速历史项目位置
    for (size_t i = 0; i < m_accelerationHistory.size(); ++i)
    {
        m_accelerationHistory[i].setPosition(m_padding + 10, currentY);
        currentY += m_lineSpacing * 0.8f;
    }

    currentY += m_lineSpacing * 0.5f;

    // 统计信息标题
    m_statisticsTitle.setPosition(m_padding, currentY);
    currentY += m_lineSpacing;

    // 统计信息位置
    m_totalRunTimeText.setPosition(m_padding + 10, currentY);
    currentY += m_lineSpacing * 0.8f;

    m_stopCountText.setPosition(m_padding + 10, currentY);
    currentY += m_lineSpacing * 0.8f;

    m_averageSpeedText.setPosition(m_padding + 10, currentY);
}

void VehicleInfoPanel::setVehicle(const Vehicle *vehicle)
{
    m_selectedVehicle = vehicle;
    if (!vehicle)
    {
        // 清空显示
        m_motionStateText.setString("No vehicle selected");
        m_currentSpeedText.setString("");
        m_accelerationText.setString("");
        m_positionText.setString("");
        return;
    }

    // 立即更新显示
    updateInfo(0.0f);
}

void VehicleInfoPanel::updateInfo(float currentTime)
{
    if (!m_selectedVehicle)
    {
        return;
    }

    // 简化版本 - 使用模拟数据而不是实际的Vehicle成员
    m_motionStateText.setString("Motion State: Cruising");
    m_currentSpeedText.setString("Current Speed: 1.5 m/s");
    m_accelerationText.setString("Acceleration: 0.0 m/s²");
    m_positionText.setString("Position: 1250 mm");

    // 更新统计信息显示
    m_totalRunTimeText.setString("Total Run Time: " + formatTime(m_statistics.totalRunTime));
    m_stopCountText.setString("Stop Count: " + std::to_string(m_statistics.stopCount));
    m_averageSpeedText.setString("Average Speed: " + formatSpeed(m_statistics.averageSpeed));
}

void VehicleInfoPanel::recordAccelerationEvent(int vehicleId, float startTime, float endTime,
                                               float startSpeed, float endSpeed,
                                               float acceleration, const std::string &type)
{
    AccelerationEvent event;
    event.vehicleId = vehicleId;
    event.startTime = startTime;
    event.endTime = endTime;
    event.startSpeed = startSpeed;
    event.endSpeed = endSpeed;
    event.acceleration = acceleration;
    event.type = type;

    m_accelerationEvents.push_back(event);

    // 限制历史记录数量，只保留最近的10条
    if (m_accelerationEvents.size() > 10)
    {
        m_accelerationEvents.erase(m_accelerationEvents.begin());
    }
    // 更新显示
    updateAccelerationHistoryDisplay();
}

void VehicleInfoPanel::updateAccelerationHistoryDisplay()
{
    // 清空现有显示
    m_accelerationHistory.clear();

    // 为每个加减速事件创建文本显示
    for (const auto &event : m_accelerationEvents)
    {
        sf::Text historyText;
        historyText.setFont(m_font);
        historyText.setCharacterSize(10);
        historyText.setFillColor(sf::Color(80, 80, 80));
        std::ostringstream oss;
        oss << "V" << event.vehicleId << " " << event.type << ": " << formatTime(event.startTime) << "-" << formatTime(event.endTime)
            << " | " << formatSpeed(event.startSpeed) << "→" << formatSpeed(event.endSpeed);
        historyText.setString(oss.str());

        m_accelerationHistory.push_back(historyText);
    }

    // 更新布局
    updateLayout();
}

void VehicleInfoPanel::updateStatistics(float deltaTime)
{
    if (!m_selectedVehicle)
    {
        return;
    }

    // 简化版本 - 使用模拟数据进行统计
    // 假设车辆在运行状态
    m_statistics.totalRunTime += deltaTime;

    // 模拟停车检测
    static float timeAccumulator = 0.0f;
    timeAccumulator += deltaTime;
    if (timeAccumulator > 5.0f)
    { // 每5秒模拟一次停车
        m_statistics.stopCount++;
        timeAccumulator = 0.0f;
    }

    // 模拟平均速度计算
    m_statistics.averageSpeed = 1.2f; // 固定值用于演示
    m_statistics.maxSpeed = 2.0f;     // 固定最大速度
}

void VehicleInfoPanel::clearHistory()
{
    m_accelerationEvents.clear();
    m_accelerationHistory.clear();
    m_statistics = VehicleStatistics(); // 重置统计信息
    updateLayout();
}

void VehicleInfoPanel::setSize(float width, float height)
{
    m_width = width;
    m_height = height;
    m_background.setSize(sf::Vector2f(m_width, m_height));
    updateLayout();
}

bool VehicleInfoPanel::handleEvent(const sf::Event &event, const sf::Vector2f &localMousePos)
{
    // 简单的边界检查
    sf::FloatRect bounds(0, 0, m_width, m_height);
    if (bounds.contains(localMousePos))
    {
        // 当前版本不处理特殊交互，仅消费事件
        return true;
    }
    return false;
}

std::string VehicleInfoPanel::formatTime(float seconds) const
{
    return UIUtils::formatSimulationTime(seconds);
}

std::string VehicleInfoPanel::formatSpeed(float speed) const
{
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << speed << " m/s";
    return oss.str();
}

void VehicleInfoPanel::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    // 应用变换
    states.transform *= getTransform();

    // 绘制背景
    target.draw(m_background, states);

    // 绘制所有文本元素
    target.draw(m_titleText, states);
    target.draw(m_motionStateText, states);
    target.draw(m_currentSpeedText, states);
    target.draw(m_accelerationText, states);
    target.draw(m_positionText, states);

    target.draw(m_accelerationHistoryTitle, states);
    for (const auto &historyText : m_accelerationHistory)
    {
        target.draw(historyText, states);
    }

    target.draw(m_statisticsTitle, states);
    target.draw(m_totalRunTimeText, states);
    target.draw(m_stopCountText, states);
    target.draw(m_averageSpeedText, states);
}
