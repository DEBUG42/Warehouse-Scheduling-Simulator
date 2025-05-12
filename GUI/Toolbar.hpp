#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>



class Toolbar {
private:
    // 控件集合
    std::vector<std::unique_ptr<Button>> m_buttons; // 功能按钮
    std::unique_ptr<TimeDisplay> m_timeDisplay;    // 时间显示组件
    std::unique_ptr<SpeedControl> m_speedControl;  // 速度调节滑块

public:
    /**
     * @brief 处理工具栏区域输入事件
     * @param event SFML事件对象
     * @param mousePos 鼠标位置（相对窗口坐标）
     * @return 是否消耗了该事件
     */
    bool handleEvent(const sf::Event& event, const sf::Vector2f& mousePos);

    /**
     * @brief 更新时间显示数值
     * @param simTime 当前仿真时间
     * @param realTime 程序运行时间
     */
    void updateTimeDisplay(float simTime, float realTime);
};