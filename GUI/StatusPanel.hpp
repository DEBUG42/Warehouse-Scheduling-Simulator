#include <SFML/Graphics.hpp>
#include <memory>

class StatusPanel {
private:
    // 布局参数
    const float m_panelWidth = 300.0f;         // 面板宽度
    const sf::Color m_backgroundColor {35, 40, 45}; // 背景色
    
    // 内容元素
    std::unique_ptr<TaskListView> m_taskList;  // 任务队列视图
    std::unique_ptr<ObjectInspector> m_inspector; // 对象详细信息
    
public:
    /**
     * @brief 更新面板显示内容
     * @param selectedObject 当前选中的对象（可为nullptr）
     * @param pendingTasks 全局待处理任务队列
     */
    void refreshContent(const SimObject* selectedObject,
                       const std::vector<Task>& pendingTasks);

    /**
     * @brief 渲染面板界面
     * @param target 渲染目标
     * @param position 面板左上角位置
     */
    void render(sf::RenderTarget& target, const sf::Vector2f& position);
};