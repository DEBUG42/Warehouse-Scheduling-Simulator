#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>
#include <vector>
#include <memory>
#include <string>
#include <random>
#include <thread>
#include <chrono>

// 定义M_PI（如果cmath没有提供）
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/**
 * @file GUITest_Fixed.cpp
 * @brief 物流仓库GUI系统测试程序
 *
 * 该文件是一个独立的GUI测试程序，用于验证物流仓库仿真系统的图形界面功能
 * 不依赖于核心模块，使用模拟数据进行显示
 * 修复了VehicleState和DeviceState的定义与实际项目代码的不一致问题
 */

// 车辆状态结构体
struct VehicleState
{
    int id;            // 车辆ID
    float position;    // 轨道位置（毫米，从起点开始计算）
    float speed;       // 当前速度（毫米/秒）
    bool isLoaded;     // 载货状态（true=载货，false=空载）
    int currentTaskId; // 当前任务ID（-1表示无任务）

    // 添加缺失的字段
    enum class MotionState
    {
        STOPPED,
        MOVING,
        ACCELERATING,
        DECELERATING
    };
    MotionState motionState; // 运动状态

    // 构造函数，设置默认值
    VehicleState() : id(0), position(0.0f), speed(0.0f),
                     isLoaded(false), currentTaskId(-1),
                     motionState(MotionState::STOPPED) {}
};

// 设备类型枚举
enum class DeviceType
{
    INPUT_STATION,   // 入库口
    OUTPUT_STATION,  // 出库口
    STORAGE_STATION, // 仓储区
    WORK_STATION     // 工作站
};

// 设备状态枚举
enum class DeviceStatus
{
    IDLE, // 空闲
    BUSY, // 忙碌
    FAULT // 故障
};

// 设备状态信息结构
struct DeviceState
{
    int id;              // 设备ID
    DeviceType type;     // 设备类型
    float trackPosition; // 轨道位置（毫米）
    DeviceStatus status; // 设备状态
    int capacity;        // 最大容量
    int currentLoad;     // 当前负载
    int materialId;      // 当前处理的物料ID（-1表示无物料）

    // 添加缺失的字段
    float processingProgress; // 处理进度(0.0-1.0)
    int queuedTaskCount;      // 排队任务数量

    // 构造函数，设置默认值
    DeviceState() : id(0), type(DeviceType::STORAGE_STATION),
                    trackPosition(0.0f), status(DeviceStatus::IDLE),
                    capacity(1), currentLoad(0), materialId(-1),
                    processingProgress(0.0f), queuedTaskCount(0) {}
};

// 任务类型枚举
enum class TaskType
{
    INPUT,  // 入库任务
    OUTPUT, // 出库任务
    MOVE    // 移动任务
};

// 任务状态结构体
struct Task
{
    int taskId;            // 任务ID
    TaskType type;         // 任务类型
    int materialId;        // 物料ID
    int startDeviceId;     // 起点设备ID
    int endDeviceId;       // 终点设备ID
    float createTime;      // 创建时间(s)
    float startTime;       // 开始时间(s)
    float completeTime;    // 完成时间(s)
    int assignedVehicleId; // 分配的车辆ID(-1表示未分配)
    bool isCompleted;      // 是否已完成

    // 构造函数
    Task() : taskId(0), type(TaskType::MOVE), materialId(-1),
             startDeviceId(-1), endDeviceId(-1), createTime(0.0f),
             startTime(0.0f), completeTime(0.0f), assignedVehicleId(-1),
             isCompleted(false) {}
};

// 轨道渲染器
class TrackRenderer
{
private:
    sf::VertexArray m_straightSegments; // 直线段顶点
    sf::VertexArray m_curveSegments;    // 弯道段顶点
    sf::Color m_straightColor;          // 直线颜色
    sf::Color m_curveColor;             // 弯道颜色
    float m_radius;                     // 弯道半径（像素）
    float m_straightLength;             // 直道长度（像素）
    float m_trackWidth;                 // 轨道宽度（像素）
    float m_mmToPixel;                  // 毫米到像素的转换比例

public:
    TrackRenderer() : m_straightSegments(sf::Lines),
                      m_curveSegments(sf::Lines),
                      m_straightColor(sf::Color(180, 180, 180)),
                      m_curveColor(sf::Color(160, 160, 160)),
                      m_radius(125.0f),          // 弯道半径2500mm对应125像素
                      m_straightLength(2000.0f), // 直道长度40000mm对应2000像素
                      m_trackWidth(12.0f),       // 轨道宽度1200mm对应12像素
                      m_mmToPixel(0.05f)         // 1mm = 0.05像素
    {
        generateTrack();
    }

    void generateTrack()
    {
        // 清除现有顶点
        m_straightSegments.clear();
        m_curveSegments.clear();

        // 计算几何参数
        float width = 2 * m_radius + m_straightLength;
        float height = 2 * m_radius + m_straightLength;

        // 轨道中心
        float centerX = 0.0f;
        float centerY = 0.0f;

        // 添加四条直线段（上、右、下、左）
        // 上边
        addLine(centerX - width / 2 + m_radius, centerY - height / 2,
                centerX + width / 2 - m_radius, centerY - height / 2);

        // 右边
        addLine(centerX + width / 2, centerY - height / 2 + m_radius,
                centerX + width / 2, centerY + height / 2 - m_radius);

        // 下边
        addLine(centerX + width / 2 - m_radius, centerY + height / 2,
                centerX - width / 2 + m_radius, centerY + height / 2);

        // 左边
        addLine(centerX - width / 2, centerY + height / 2 - m_radius,
                centerX - width / 2, centerY - height / 2 + m_radius);

        // 添加四个弯道（左上、右上、右下、左下）
        // 左上角弯道
        addArc(centerX - width / 2 + m_radius, centerY - height / 2 + m_radius,
               m_radius, 180.0f, 270.0f, 20);

        // 右上角弯道
        addArc(centerX + width / 2 - m_radius, centerY - height / 2 + m_radius,
               m_radius, 270.0f, 360.0f, 20);

        // 右下角弯道
        addArc(centerX + width / 2 - m_radius, centerY + height / 2 - m_radius,
               m_radius, 0.0f, 90.0f, 20);

        // 左下角弯道
        addArc(centerX - width / 2 + m_radius, centerY + height / 2 - m_radius,
               m_radius, 90.0f, 180.0f, 20);
    }

    void addLine(float x1, float y1, float x2, float y2)
    {
        m_straightSegments.append(sf::Vertex(sf::Vector2f(x1, y1), m_straightColor));
        m_straightSegments.append(sf::Vertex(sf::Vector2f(x2, y2), m_straightColor));
    }

    void addArc(float cx, float cy, float r, float startAngle, float endAngle, int segments)
    {
        float angleStep = (endAngle - startAngle) * M_PI / 180.0f / segments;
        float startRad = startAngle * M_PI / 180.0f;

        for (int i = 0; i < segments; ++i)
        {
            float angle1 = startRad + i * angleStep;
            float angle2 = startRad + (i + 1) * angleStep;

            float x1 = cx + r * cosf(angle1);
            float y1 = cy + r * sinf(angle1);
            float x2 = cx + r * cosf(angle2);
            float y2 = cy + r * sinf(angle2);

            m_curveSegments.append(sf::Vertex(sf::Vector2f(x1, y1), m_curveColor));
            m_curveSegments.append(sf::Vertex(sf::Vector2f(x2, y2), m_curveColor));
        }
    }

    void draw(sf::RenderTarget &target)
    {
        target.draw(m_straightSegments);
        target.draw(m_curveSegments);
    }

    // 计算轨道上的位置（从路程计算坐标）
    sf::Vector2f calculatePositionOnTrack(float position_mm, float &angle)
    {
        // 轨道参数
        float straightLength = 40000.0f; // 直道长度(mm)
        float curveRadius = 2500.0f;     // 弯道半径(mm)

        // 计算轨道各段长度
        float bottomStraightLength = straightLength;                  // 下方直道长度
        float rightCurveLength = M_PI * curveRadius;                  // 右弯道长度
        float topStraightLength = straightLength;                     // 上方直道长度
        float leftCurveLength = M_PI * curveRadius;                   // 左弯道长度
        float totalLength = bottomStraightLength + rightCurveLength + // 轨道总长度
                            topStraightLength + leftCurveLength;

        // 标准化位置（确保在0~totalLength范围内）
        position_mm = fmod(position_mm, totalLength);
        if (position_mm < 0)
            position_mm += totalLength;

        // 转换参数（毫米到像素）
        float pos_x = 0.0f, pos_y = 0.0f;
        float curve_radius_px = curveRadius * m_mmToPixel;
        float straight_length_px = straightLength * m_mmToPixel;
        float width = 2 * curve_radius_px + straight_length_px;
        float height = 2 * curve_radius_px + straight_length_px;

        // 段1: 下方直道
        if (position_mm < bottomStraightLength)
        {
            float t = position_mm / bottomStraightLength;
            pos_x = -width / 2 + curve_radius_px + t * straight_length_px;
            pos_y = height / 2 - curve_radius_px;
            angle = 0.0f; // 向右
        }
        // 段2: 右侧弯道
        else if (position_mm < bottomStraightLength + rightCurveLength)
        {
            float t = (position_mm - bottomStraightLength) / rightCurveLength;
            float angle_rad = t * M_PI;
            pos_x = width / 2 - curve_radius_px + curve_radius_px * cos(angle_rad);
            pos_y = height / 2 - curve_radius_px - curve_radius_px * sin(angle_rad);
            angle = -t * 180.0f; // 弯道旋转角度
        }
        // 段3: 上方直道
        else if (position_mm < bottomStraightLength + rightCurveLength + topStraightLength)
        {
            float t = (position_mm - bottomStraightLength - rightCurveLength) / topStraightLength;
            pos_x = width / 2 - curve_radius_px - t * straight_length_px;
            pos_y = -height / 2 + curve_radius_px;
            angle = 180.0f; // 向左
        }
        // 段4: 左侧弯道
        else
        {
            float t = (position_mm - bottomStraightLength - rightCurveLength - topStraightLength) / leftCurveLength;
            float angle_rad = t * M_PI;
            pos_x = -width / 2 + curve_radius_px - curve_radius_px * cos(angle_rad);
            pos_y = -height / 2 + curve_radius_px + curve_radius_px * sin(angle_rad);
            angle = 180.0f - t * 180.0f; // 弯道旋转角度
        }

        return sf::Vector2f(pos_x, pos_y);
    }

    // 获取轨道参数
    float getRadiusPixels() const { return m_radius; }
    float getStraightLengthPixels() const { return m_straightLength; }
    float getTrackWidthPixels() const { return m_trackWidth; }
    float getMmToPixelRatio() const { return m_mmToPixel; }
};

// 车辆渲染器
class VehicleRenderer
{
private:
    sf::Font m_font;
    sf::Color m_colorEmpty{80, 130, 200};     // 空载状态颜色
    sf::Color m_colorLoaded{200, 90, 40};     // 载货状态颜色
    sf::Color m_colorAssigned{140, 80, 160};  // 已分配任务状态颜色
    sf::Color m_shadowColor{50, 50, 50, 150}; // 阴影颜色

    float m_mmToPixel;     // 毫米到像素的转换比例
    float m_vehicleLength; // 车辆长度(像素)
    float m_vehicleWidth;  // 车辆宽度(像素)
    float m_vehicleHeight; // 车辆高度(像素)

public:
    VehicleRenderer(float mmToPixel = 0.05f) : m_mmToPixel(mmToPixel),
                                               m_vehicleLength(100.0f), // 2000mm车长对应100像素
                                               m_vehicleWidth(40.0f),   // 800mm车宽对应40像素
                                               m_vehicleHeight(20.0f)   // 车辆高度(像素)
    {
        // 加载字体
        if (!m_font.loadFromFile("resources/fonts/Arial.ttf"))
        {
            // 尝试其他可能的路径
            if (!m_font.loadFromFile("GUI/resources/fonts/Arial.ttf"))
            {
                if (!m_font.loadFromFile("fonts/Arial.ttf"))
                {
                    std::cerr << "警告：无法加载字体文件，车辆标签可能无法正确显示" << std::endl;
                }
            }
        }
    }

    void drawVehicle(sf::RenderTarget &target, const VehicleState &vehicle, const sf::Vector2f &position, float angle)
    {
        // 绘制阴影（增强3D效果）
        sf::ConvexShape shadow;
        shadow.setPointCount(4);
        shadow.setPoint(0, sf::Vector2f(-m_vehicleLength / 2, -m_vehicleWidth / 2 + 5));
        shadow.setPoint(1, sf::Vector2f(m_vehicleLength / 2, -m_vehicleWidth / 2 + 5));
        shadow.setPoint(2, sf::Vector2f(m_vehicleLength / 2, m_vehicleWidth / 2 + 5));
        shadow.setPoint(3, sf::Vector2f(-m_vehicleLength / 2, m_vehicleWidth / 2 + 5));
        shadow.setFillColor(m_shadowColor);
        shadow.setPosition(position);
        shadow.setRotation(angle);
        target.draw(shadow);

        // 根据车辆状态选择颜色
        sf::Color vehicleColor;
        if (vehicle.currentTaskId >= 0)
        {
            vehicleColor = m_colorAssigned; // 已分配任务
        }
        else if (vehicle.isLoaded)
        {
            vehicleColor = m_colorLoaded; // 载货
        }
        else
        {
            vehicleColor = m_colorEmpty; // 空载
        }

        // 绘制车身
        sf::ConvexShape vehicleBody;
        vehicleBody.setPointCount(4);
        vehicleBody.setPoint(0, sf::Vector2f(-m_vehicleLength / 2, -m_vehicleWidth / 2));
        vehicleBody.setPoint(1, sf::Vector2f(m_vehicleLength / 2, -m_vehicleWidth / 2));
        vehicleBody.setPoint(2, sf::Vector2f(m_vehicleLength / 2, m_vehicleWidth / 2));
        vehicleBody.setPoint(3, sf::Vector2f(-m_vehicleLength / 2, m_vehicleWidth / 2));
        vehicleBody.setFillColor(vehicleColor);
        vehicleBody.setOutlineThickness(2);
        vehicleBody.setOutlineColor(sf::Color(30, 30, 30));
        vehicleBody.setPosition(position);
        vehicleBody.setRotation(angle);
        target.draw(vehicleBody);

        // 绘制车辆ID标签
        sf::Text idText;
        idText.setFont(m_font);
        idText.setString(std::to_string(vehicle.id));
        idText.setCharacterSize(16);
        idText.setFillColor(sf::Color::White);

        // 居中显示ID
        sf::FloatRect textBounds = idText.getLocalBounds();
        idText.setOrigin(textBounds.width / 2.0f, textBounds.height / 2.0f);

        // 应用与车辆相同的变换
        idText.setPosition(position);
        idText.setRotation(angle);

        target.draw(idText);
    }
};

// 设备/仓库渲染器
class DeviceRenderer
{
private:
    sf::Font m_font;
    sf::Color m_inputColor{60, 180, 75};    // 入库口颜色
    sf::Color m_outputColor{230, 126, 34};  // 出库口颜色
    sf::Color m_storageColor{70, 130, 180}; // 仓储区颜色
    sf::Color m_workColor{120, 120, 120};   // 工作站颜色
    sf::Color m_busyColor{200, 50, 50};     // 忙碌状态颜色
    sf::Color m_faultColor{200, 50, 200};   // 故障状态颜色

    float m_deviceWidth;  // 设备宽度(像素)
    float m_deviceHeight; // 设备高度(像素)
    float m_deviceDepth;  // 设备深度(像素)

public:
    DeviceRenderer() : m_deviceWidth(60.0f),
                       m_deviceHeight(80.0f),
                       m_deviceDepth(50.0f)
    {
        // 加载字体
        if (!m_font.loadFromFile("resources/fonts/Arial.ttf"))
        {
            // 尝试其他可能的路径
            if (!m_font.loadFromFile("GUI/resources/fonts/Arial.ttf"))
            {
                if (!m_font.loadFromFile("fonts/Arial.ttf"))
                {
                    std::cerr << "警告：无法加载字体文件，设备标签可能无法正确显示" << std::endl;
                }
            }
        }
    }

    void drawDevice(sf::RenderTarget &target, const DeviceState &device, const sf::Vector2f &position, float trackWidth)
    {
        // 根据设备类型选择颜色
        sf::Color deviceColor;
        switch (device.type)
        {
        case DeviceType::INPUT_STATION:
            deviceColor = m_inputColor;
            break;
        case DeviceType::OUTPUT_STATION:
            deviceColor = m_outputColor;
            break;
        case DeviceType::STORAGE_STATION:
            deviceColor = m_storageColor;
            break;
        case DeviceType::WORK_STATION:
            deviceColor = m_workColor;
            break;
        default:
            deviceColor = sf::Color::White;
            break;
        }

        // 根据设备状态调整颜色
        if (device.status == DeviceStatus::BUSY)
        {
            deviceColor = m_busyColor;
        }
        else if (device.status == DeviceStatus::FAULT)
        {
            deviceColor = m_faultColor;
        }

        // 确定设备在轨道的哪一侧
        bool isTop = (position.y < 0); // 在轨道上方
        float offsetY = isTop ? -(trackWidth + m_deviceHeight) : (trackWidth);

        // 绘制设备阴影（简单的3D效果）
        sf::RectangleShape shadow;
        shadow.setSize(sf::Vector2f(m_deviceWidth, m_deviceHeight));
        shadow.setFillColor(sf::Color(30, 30, 30, 100));
        shadow.setOrigin(m_deviceWidth / 2, isTop ? m_deviceHeight : 0);
        shadow.setPosition(position.x + 5, position.y + offsetY + 5);
        target.draw(shadow);

        // 绘制设备主体
        sf::RectangleShape deviceBody;
        deviceBody.setSize(sf::Vector2f(m_deviceWidth, m_deviceHeight));
        deviceBody.setFillColor(deviceColor);
        deviceBody.setOutlineThickness(2);
        deviceBody.setOutlineColor(sf::Color(30, 30, 30));
        deviceBody.setOrigin(m_deviceWidth / 2, isTop ? m_deviceHeight : 0);
        deviceBody.setPosition(position.x, position.y + offsetY);
        target.draw(deviceBody);

        // 绘制设备ID标签
        sf::Text idText;
        idText.setFont(m_font);
        idText.setString(std::to_string(device.id));
        idText.setCharacterSize(16);
        idText.setFillColor(sf::Color::White);

        // 在设备中心显示ID
        sf::FloatRect textBounds = idText.getLocalBounds();
        idText.setOrigin(textBounds.width / 2.0f, textBounds.height / 2.0f);
        idText.setPosition(
            position.x,
            position.y + offsetY + (isTop ? -m_deviceHeight / 2 : m_deviceHeight / 2));
        target.draw(idText);

        // 如果有队列任务，显示数量
        if (device.queuedTaskCount > 0)
        {
            sf::Text queueText;
            queueText.setFont(m_font);
            queueText.setString("+" + std::to_string(device.queuedTaskCount));
            queueText.setCharacterSize(14);
            queueText.setFillColor(sf::Color::Yellow);

            // 在设备右上角显示
            sf::FloatRect queueBounds = queueText.getLocalBounds();
            queueText.setOrigin(queueBounds.width, isTop ? queueBounds.height : 0);
            queueText.setPosition(
                position.x + m_deviceWidth / 2 - 5,
                position.y + offsetY + (isTop ? -m_deviceHeight + 5 : 5));
            target.draw(queueText);
        }

        // 如果正在处理，显示进度条
        if (device.status == DeviceStatus::BUSY && device.processingProgress > 0)
        {
            float barWidth = m_deviceWidth * 0.8f;
            float barHeight = 6.0f;

            // 进度条背景
            sf::RectangleShape progressBg;
            progressBg.setSize(sf::Vector2f(barWidth, barHeight));
            progressBg.setFillColor(sf::Color(50, 50, 50));
            progressBg.setOrigin(barWidth / 2, 0);
            progressBg.setPosition(
                position.x,
                position.y + offsetY + (isTop ? -10 : m_deviceHeight + 10));
            target.draw(progressBg);

            // 进度条前景
            sf::RectangleShape progressFg;
            progressFg.setSize(sf::Vector2f(barWidth * device.processingProgress, barHeight));
            progressFg.setFillColor(sf::Color(0, 255, 0));
            progressFg.setOrigin(barWidth / 2, 0);
            progressFg.setPosition(
                position.x - (barWidth * (1.0f - device.processingProgress) / 2),
                position.y + offsetY + (isTop ? -10 : m_deviceHeight + 10));
            target.draw(progressFg);
        }
    }
};

// 模拟数据生成器
class MockDataGenerator
{
private:
    std::mt19937 m_rng;
    std::vector<VehicleState> m_vehicles;
    std::vector<DeviceState> m_devices;

public:
    MockDataGenerator(int vehicleCount = 5)
    {
        // 初始化随机数生成器
        std::random_device rd;
        m_rng = std::mt19937(rd());

        // 创建模拟车辆
        createVehicles(vehicleCount);

        // 创建模拟设备
        createDevices();
    }

    // 创建模拟车辆
    void createVehicles(int count)
    {
        m_vehicles.clear();
        std::uniform_real_distribution<float> posDist(0.0f, 90000.0f);
        std::uniform_real_distribution<float> speedDist(500.0f, 2000.0f);

        for (int i = 0; i < count; ++i)
        {
            VehicleState vehicle;
            vehicle.id = i;
            vehicle.position = posDist(m_rng);
            vehicle.speed = speedDist(m_rng);
            vehicle.motionState = VehicleState::MotionState::MOVING;
            vehicle.isLoaded = (i % 3 == 0);
            vehicle.currentTaskId = (i % 4 == 0) ? i : -1;
            m_vehicles.push_back(vehicle);
        }
    }

    // 创建模拟设备
    void createDevices()
    {
        m_devices.clear();

        // 上方12个设备
        float basePositionTopX = 625.0f; // 第一个设备X坐标
        float spacing = 1250.0f;         // 设备间距

        for (int i = 1; i <= 12; ++i)
        {
            DeviceState device;
            device.id = i;
            // 奇数为入库口，偶数为出库口
            device.type = (i % 2 == 1) ? DeviceType::INPUT_STATION : DeviceType::OUTPUT_STATION;
            device.trackPosition = basePositionTopX + (i - 1) * spacing;
            device.status = (i % 5 == 0) ? DeviceStatus::BUSY : DeviceStatus::IDLE;
            device.capacity = 5;
            device.currentLoad = (i % 3);
            device.materialId = (device.currentLoad > 0) ? 1000 + i : -1;
            device.processingProgress = (device.status == DeviceStatus::BUSY) ? 0.5f : 0.0f;
            device.queuedTaskCount = i % 3;

            m_devices.push_back(device);
        }

        // 下方6个设备
        float basePositionBottomX = 8050.0f; // 第一个设备X坐标

        // 13-15号为出库口
        for (int i = 13; i <= 15; ++i)
        {
            DeviceState device;
            device.id = i;
            device.type = DeviceType::OUTPUT_STATION;

            if (i == 13)
                device.trackPosition = basePositionBottomX;
            else if (i == 14)
                device.trackPosition = basePositionBottomX + 1200.0f;
            else // i == 15
                device.trackPosition = basePositionBottomX + 12100.0f;

            device.status = (i == 14) ? DeviceStatus::BUSY : DeviceStatus::IDLE;
            device.capacity = 8;
            device.currentLoad = (i % 5);
            device.materialId = (device.currentLoad > 0) ? 2000 + i : -1;
            device.processingProgress = (device.status == DeviceStatus::BUSY) ? 0.7f : 0.0f;
            device.queuedTaskCount = (i == 13) ? 2 : 0;

            m_devices.push_back(device);
        }

        // 16-18号为入库口
        for (int i = 16; i <= 18; ++i)
        {
            DeviceState device;
            device.id = i;
            device.type = DeviceType::INPUT_STATION;

            if (i == 16)
                device.trackPosition = basePositionBottomX + 23000.0f;
            else if (i == 17)
                device.trackPosition = basePositionBottomX + 24300.0f;
            else // i == 18
                device.trackPosition = basePositionBottomX + 25600.0f;

            device.status = (i == 17) ? DeviceStatus::FAULT : DeviceStatus::IDLE;
            device.capacity = 8;
            device.currentLoad = (i % 4);
            device.materialId = (device.currentLoad > 0) ? 3000 + i : -1;
            device.processingProgress = 0.0f;
            device.queuedTaskCount = (i == 18) ? 1 : 0;

            m_devices.push_back(device);
        }
    }

    // 更新模拟数据
    void update(float deltaTime)
    {
        // 更新车辆位置
        float trackLength = 90000.0f; // 轨道总长度(mm)

        for (auto &vehicle : m_vehicles)
        {
            if (vehicle.motionState == VehicleState::MotionState::MOVING)
            {
                vehicle.position += vehicle.speed * deltaTime;
                if (vehicle.position > trackLength)
                    vehicle.position -= trackLength;
            }
        }

        // 更新设备状态
        for (auto &device : m_devices)
        {
            if (device.status == DeviceStatus::BUSY)
            {
                device.processingProgress += deltaTime * 0.1f;
                if (device.processingProgress >= 1.0f)
                {
                    device.processingProgress = 0.0f;
                    device.status = DeviceStatus::IDLE;
                    if (device.queuedTaskCount > 0)
                        device.queuedTaskCount--;
                }
            }
        }
    }

    const std::vector<VehicleState> &getVehicles() const { return m_vehicles; }
    const std::vector<DeviceState> &getDevices() const { return m_devices; }
};

// 主应用程序
class Application
{
private:
    sf::RenderWindow m_window;
    sf::View m_view;
    TrackRenderer m_trackRenderer;
    VehicleRenderer m_vehicleRenderer;
    DeviceRenderer m_deviceRenderer;
    MockDataGenerator m_mockData;
    sf::Clock m_clock;
    sf::Font m_font;
    bool m_isPaused;
    float m_simulationSpeed;

    // 视图控制
    float m_zoom;
    sf::Vector2f m_viewCenter;
    bool m_isDragging;
    sf::Vector2f m_dragStart;

    // 性能指标
    sf::Clock m_fpsTimer;
    int m_frameCounter;
    float m_fps;

public:
    Application() : m_window(sf::VideoMode(1280, 720), "物流仓库仿真系统测试", sf::Style::Default),
                    m_mockData(5),
                    m_isPaused(false),
                    m_simulationSpeed(1.0f),
                    m_zoom(1.0f),
                    m_isDragging(false),
                    m_frameCounter(0),
                    m_fps(0.0f)
    {
        // 初始化视图
        m_view = m_window.getDefaultView();
        m_viewCenter = sf::Vector2f(0.0f, 0.0f);
        m_view.setCenter(m_viewCenter);

        // 加载字体
        if (!m_font.loadFromFile("resources/fonts/Arial.ttf"))
        {
            // 尝试其他可能的路径
            if (!m_font.loadFromFile("GUI/resources/fonts/Arial.ttf"))
            {
                if (!m_font.loadFromFile("fonts/Arial.ttf"))
                {
                    std::cerr << "警告：无法加载字体文件，UI文本可能无法正确显示" << std::endl;
                }
            }
        }

        // 设置帧率限制
        m_window.setFramerateLimit(60);
    }

    void run()
    {
        while (m_window.isOpen())
        {
            handleEvents();
            update();
            render();
        }
    }

private:
    void handleEvents()
    {
        sf::Event event;
        while (m_window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                m_window.close();
            }
            else if (event.type == sf::Event::Resized)
            {
                sf::FloatRect visibleArea(0, 0, event.size.width, event.size.height);
                m_window.setView(sf::View(visibleArea));
                m_view.setSize(event.size.width, event.size.height);
                m_window.setView(m_view);
            }
            else if (event.type == sf::Event::MouseWheelScrolled)
            {
                if (event.mouseWheelScroll.delta > 0)
                    m_zoom *= 1.1f; // 放大
                else
                    m_zoom *= 0.9f; // 缩小

                // 限制缩放范围
                m_zoom = std::max(0.1f, std::min(m_zoom, 3.0f));

                m_view.setSize(m_window.getDefaultView().getSize());
                m_view.zoom(1.0f / m_zoom);
                m_window.setView(m_view);
            }
            else if (event.type == sf::Event::MouseButtonPressed)
            {
                if (event.mouseButton.button == sf::Mouse::Left)
                {
                    m_isDragging = true;
                    m_dragStart = m_window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
                }
            }
            else if (event.type == sf::Event::MouseButtonReleased)
            {
                if (event.mouseButton.button == sf::Mouse::Left)
                {
                    m_isDragging = false;
                }
            }
            else if (event.type == sf::Event::MouseMoved)
            {
                if (m_isDragging)
                {
                    sf::Vector2f currentPos = m_window.mapPixelToCoords(sf::Vector2i(event.mouseMove.x, event.mouseMove.y));
                    sf::Vector2f delta = m_dragStart - currentPos;

                    m_viewCenter += delta;
                    m_view.setCenter(m_viewCenter);
                    m_window.setView(m_view);

                    m_dragStart = m_window.mapPixelToCoords(sf::Vector2i(event.mouseMove.x, event.mouseMove.y));
                }
            }
            else if (event.type == sf::Event::KeyPressed)
            {
                handleKeyPress(event.key.code);
            }
        }
    }

    void handleKeyPress(sf::Keyboard::Key key)
    {
        switch (key)
        {
        case sf::Keyboard::Space:
            // 暂停/继续仿真
            m_isPaused = !m_isPaused;
            break;
        case sf::Keyboard::Num1:
            // 0.5倍速
            m_simulationSpeed = 0.5f;
            break;
        case sf::Keyboard::Num2:
            // 1倍速
            m_simulationSpeed = 1.0f;
            break;
        case sf::Keyboard::Num3:
            // 2倍速
            m_simulationSpeed = 2.0f;
            break;
        case sf::Keyboard::Num4:
            // 5倍速
            m_simulationSpeed = 5.0f;
            break;
        case sf::Keyboard::Num5:
            // 10倍速
            m_simulationSpeed = 10.0f;
            break;
        case sf::Keyboard::R:
            // 重置视图
            m_viewCenter = sf::Vector2f(0.0f, 0.0f);
            m_zoom = 1.0f;
            m_view.setCenter(m_viewCenter);
            m_view.setSize(m_window.getDefaultView().getSize());
            m_window.setView(m_view);
            break;
        default:
            break;
        }
    }

    void update()
    {
        // 计算帧率
        m_frameCounter++;
        if (m_fpsTimer.getElapsedTime().asSeconds() >= 1.0f)
        {
            m_fps = static_cast<float>(m_frameCounter) / m_fpsTimer.getElapsedTime().asSeconds();
            m_frameCounter = 0;
            m_fpsTimer.restart();
        }

        // 如果未暂停，更新模拟数据
        if (!m_isPaused)
        {
            float deltaTime = m_clock.restart().asSeconds() * m_simulationSpeed;
            m_mockData.update(deltaTime);
        }
        else
        {
            m_clock.restart();
        }
    }

    void render()
    {
        m_window.clear(sf::Color(40, 40, 40));

        // 绘制轨道
        m_trackRenderer.draw(m_window);

        // 绘制设备/仓库
        const auto &devices = m_mockData.getDevices();
        float angle = 0.0f;
        for (const auto &device : devices)
        {
            sf::Vector2f position = m_trackRenderer.calculatePositionOnTrack(device.trackPosition, angle);
            m_deviceRenderer.drawDevice(m_window, device, position, m_trackRenderer.getTrackWidthPixels());
        }

        // 绘制车辆
        const auto &vehicles = m_mockData.getVehicles();
        for (const auto &vehicle : vehicles)
        {
            float angle = 0.0f;
            sf::Vector2f position = m_trackRenderer.calculatePositionOnTrack(vehicle.position, angle);
            m_vehicleRenderer.drawVehicle(m_window, vehicle, position, angle);
        }

        // 绘制UI信息
        drawUI();

        m_window.display();
    }

    void drawUI()
    {
        // 保存当前视图
        sf::View currentView = m_window.getView();
        m_window.setView(m_window.getDefaultView());

        sf::Text infoText;
        infoText.setFont(m_font);
        infoText.setCharacterSize(16);
        infoText.setPosition(10, 10);

        // 构建状态文本
        std::string statusStr =
            "FPS: " + std::to_string(static_cast<int>(m_fps)) +
            " | 仿真状态: " + (m_isPaused ? "已暂停" : "运行中") +
            " | 仿真速度: " + std::to_string(m_simulationSpeed) + "x" +
            " | 车辆数: " + std::to_string(m_mockData.getVehicles().size()) +
            " | 缩放: " + std::to_string(static_cast<int>(m_zoom * 100)) + "%";

        infoText.setString(statusStr);
        infoText.setFillColor(sf::Color::White);
        infoText.setOutlineThickness(1);
        infoText.setOutlineColor(sf::Color::Black);

        m_window.draw(infoText);

        // 绘制控制帮助信息
        sf::Text helpText;
        helpText.setFont(m_font);
        helpText.setCharacterSize(14);
        helpText.setPosition(10, m_window.getSize().y - 60);

        std::string helpStr =
            "控制: 空格键=暂停/继续 | 1-5键=调整速度 | 滚轮=缩放 | 左键拖拽=平移 | R键=重置视图";

        helpText.setString(helpStr);
        helpText.setFillColor(sf::Color(200, 200, 200));
        helpText.setOutlineThickness(1);
        helpText.setOutlineColor(sf::Color::Black);

        m_window.draw(helpText);

        // 恢复视图
        m_window.setView(currentView);
    }
};

int main()
{
    try
    {
        Application app;
        app.run();
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "错误: " << e.what() << std::endl;
        return -1;
    }
    catch (...)
    {
        std::cerr << "未知错误" << std::endl;
        return -2;
    }
}
