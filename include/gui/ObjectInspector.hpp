#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <memory>
#include "SimObject.hpp"   // 包含 VehicleState 和 ObjectType
#include "DeviceState.hpp" // 包含 DeviceState

/**
 * @brief 对象检视器类
 *
 * 用于在状态面板中显示选中对象的详细信息。
 */
class ObjectInspector : public sf::Drawable, public sf::Transformable
{
private:
    sf::Font &m_font; // 外部传入的字体引用
    float m_width;    // 检视器宽度
    // float m_height;       // 检视器高度 (由 StatusPanel 控制)

    const gui::SimObject *m_currentObject = nullptr; // 当前显示的对象

    sf::Text m_titleText;                // 标题 "对象详情" 或 "未选中对象"
    std::vector<sf::Text> m_detailLines; // 用于显示多行详细信息

    const unsigned int m_characterSize = 12; // 文本字号
    const float m_lineSpacing = 16.0f;       // 行间距
    const float m_padding = 5.0f;            // 内部边距

    /**
     * @brief 根据当前对象更新显示的文本行
     */
    void rebuildDisplay();

    /**
     * @brief 辅助函数，添加一行文本到 m_detailLines
     * @param label 标签 (例如 "ID: ")
     * @param value 值 (例如 "V_001")
     * @param yOffset 当前行基于上一行的Y轴偏移引用，函数内部会更新它
     */
    void addDetailLine(const std::string &label, const std::string &value, float &currentY);

    // 将各种状态枚举转换为可读字符串的辅助函数
    std::string vehicleStatusToString(gui::VehicleStatus status) const;
    std::string deviceTypeToString(gui::DeviceType type) const;
    std::string deviceStatusToString(gui::DeviceStatus status) const;

public:
    /**
     * @brief 构造函数
     * @param font 字体引用
     * @param width 检视器宽度
     */
    ObjectInspector(sf::Font &font, float width);

    /**
     * @brief 更新当前要显示的对象
     * @param selectedObject 指向 SimObject 的指针，如果为 nullptr 则表示没有对象被选中
     */
    void updateObject(const gui::SimObject *selectedObject);

    /**
     * @brief 绘制对象检视器
     * @param target 渲染目标
     * @param states 渲染状态
     */
    virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

    // void setHeight(float height) { m_height = height; } // 如果需要显式高度控制

    /**
     * @brief 处理对象检视器范围内的输入事件
     * @param event SFML事件对象
     * @param localMousePos 相对检视器左上角的鼠标位置
     * @return 如果事件被消耗则返回 true，否则 false
     */
    bool handleEvent(const sf::Event &event, const sf::Vector2f &localMousePos);
};