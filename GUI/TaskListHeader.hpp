#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "../Core/Task.hpp"

// 任务列表显示类
class TaskListView
{
private:
    sf::Font &m_font;
    float m_width;
    std::vector<Task> m_tasks;
    sf::RectangleShape m_background;
    sf::Text m_headerText;

public:
    TaskListView(sf::Font &font, float width);
    void updateTasks(const std::vector<Task> &tasks);
    void render(sf::RenderTarget &target, const sf::Vector2f &position);
};

// 对象检查器类型
enum class ObjectType
{
    None,
    Vehicle,
    Device
};

// 对象信息显示类
class ObjectInspector
{
private:
    sf::Font &m_font;
    float m_width;
    ObjectType m_currentType = ObjectType::None;

    // 存储当前选中对象的数据
    union {
        struct {
            int id;
            float position;
            float speed;
            bool isLoaded;
            int taskId;
        } vehicle;

        struct {
            int id;
            DeviceType type;
            DeviceStatus status;
            int queueCount;
            float progress;
        } device;
    } m_data;

    sf::RectangleShape m_background;
    sf::Text m_headerText;

public:
    ObjectInspector(sf::Font &font, float width);
    void updateObject(const SimObject *obj);
    void render(sf::RenderTarget &target, const sf::Vector2f &position);

private:
    void renderVehicleDetails(sf::RenderTarget &target, const sf::Vector2f &position);
    void renderDeviceDetails(sf::RenderTarget &target, const sf::Vector2f &position);
};
