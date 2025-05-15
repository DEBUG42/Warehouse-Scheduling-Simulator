#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>

/**
 * @brief 按钮控件类，支持文字显示和点击检测
 */
class Button {
private:
    sf::RectangleShape m_shape; ///< 按钮的矩形形状
    sf::Text m_text;           ///< 按钮上显示的文字
    
public:
    /**
     * @brief 构造函数，创建按钮对象
     * @param size 按钮的尺寸
     * @param font 按钮文字使用的字体
     * @param text 按钮初始显示文字
     */
    Button(const sf::Vector2f& size, const sf::Font& font, const std::string& text);
    
    /**
     * @brief 设置按钮显示文字
     * @param text 要设置的新文字内容
     */
    void setText(const std::string& text);
    
    /**
     * @brief 检测给定点是否在按钮范围内
     * @param point 要检测的点坐标（窗口坐标系）
     * @return 如果点在按钮范围内返回true，否则返回false
     */
    bool contains(const sf::Vector2f& point) const;
    
    /**
     * @brief 绘制按钮到指定渲染目标
     * @param target 要绘制到的渲染目标
     */
    void draw(sf::RenderTarget& target) const;
    
    /**
     * @brief 设置按钮位置
     * @param x X轴坐标
     * @param y Y轴坐标
     */
    void setPosition(float x, float y);
};

/**
 * @brief 时间显示组件，显示仿真时间和实际时间
 */
class TimeDisplay : public sf::Drawable, public sf::Transformable {
private:
    sf::Text m_text; ///< 显示时间的文本对象
    sf::Font m_font; ///< 使用的字体
    
public:
    /**
     * @brief 构造函数，初始化时间显示组件
     * @param font 要使用的字体
     */
    explicit TimeDisplay(const sf::Font& font);
    
    /**
     * @brief 更新时间显示内容
     * @param simTime 当前仿真时间（秒）
     * @param realTime 程序实际运行时间（秒）
     */
    void updateTime(float simTime, float realTime);
    
    /**
     * @brief 绘制时间显示组件
     * @param target 渲染目标
     * @param states 渲染状态
     */
    void draw(sf::RenderTarget& target, sf::RenderStates states) const;
};

/**
 * @brief 速度控制滑块组件
 */
class SpeedControl {
private:
    sf::RectangleShape m_track;   ///< 滑块的轨道
    sf::CircleShape m_thumb;      ///< 滑块的拖拽柄
    float m_minSpeed;             ///< 最小速度值
    float m_maxSpeed;             ///< 最大速度值
    float m_currentSpeed;         ///< 当前设置的速度值
    
public:
    /**
     * @brief 构造函数，初始化速度控制组件
     * @param position 组件位置（左上角坐标）
     * @param minSpeed 允许的最小速度
     * @param maxSpeed 允许的最大速度
     */
    SpeedControl(const sf::Vector2f& position, float minSpeed, float maxSpeed);
    
    /**
     * @brief 处理输入事件
     * @param event SFML事件对象
     * @param mousePos 鼠标当前位置
     * @return 如果事件被处理返回true，否则返回false
     */
    bool handleEvent(const sf::Event& event, const sf::Vector2f& mousePos);
    
    /**
     * @brief 获取当前设置的速度值
     * @return 当前速度值
     */
    float getSpeed() const;
    
    /**
     * @brief 绘制速度控制组件
     * @param target 要绘制到的渲染目标
     */
    void draw(sf::RenderTarget& target) const;
};

/**
 * @brief 工具栏主控类，整合各种界面控件
 */
class Toolbar {
private:
    std::vector<std::unique_ptr<Button>> m_buttons; ///< 功能按钮集合
    std::unique_ptr<TimeDisplay> m_timeDisplay;    ///< 时间显示组件
    std::unique_ptr<SpeedControl> m_speedControl;  ///< 速度控制组件

    sf::RectangleShape m_background; ///< 工具栏背景
    sf::Font m_font;                 ///< 全局使用的字体
    sf::Text m_speedText;            ///< 速度倍率显示文本

    bool m_isPaused = false;         ///< 暂停状态标志
    
public:
    /**
     * @brief 获取当前暂停状态
     * @return 如果处于暂停状态返回true，否则返回false
     */
    bool isPaused() const { return m_isPaused; }

    /**
     * @brief 工具栏构造函数
     * @param font 要使用的字体
     */
    explicit Toolbar(const sf::Font& font);

    /**
     * @brief 处理工具栏区域输入事件
     * @param event SFML事件对象
     * @param mousePos 鼠标位置（相对窗口坐标）
     * @return 是否消耗了该事件
     */
    bool handleEvent(const sf::Event& event, const sf::Vector2f& mousePos);

    /**
     * @brief 更新时间显示数值
     * @param simTime 当前仿真时间（秒）
     * @param realTime 程序实际运行时间（秒）
     */
    void updateTimeDisplay(float simTime, float realTime);

    /**
     * @brief 绘制整个工具栏
     * @param target 要绘制到的渲染目标
     */
    void draw(sf::RenderTarget& target) const;
};