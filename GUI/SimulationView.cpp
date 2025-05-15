#include "SimulationView.hpp"

void SimulationView::updateViewTransforms(float deltaTime){
    //进阶功能暂未实现 等待后续更新        
}

SimulationView::SimulationView() {
    // 初始化视图变换参数
    m_worldView.reset(sf::FloatRect(0, 0, 800, 600));
    m_uiView.reset(sf::FloatRect(0, 0, 800, 600));
    m_viewCenter = m_worldView.getCenter();

    // 初始化纹理
    if (!m_trackTexture.loadFromFile("path/to/track_texture.png")) {
        std::cerr << "Failed to load track texture" << std::endl;
    }

    // 初始化着色器
    if (!m_trackShader.loadFromFile("path/to/track_shader.vert", "path/to/track_shader.frag")) {
        std::cerr << "Failed to load track shader" << std::endl;
    }
}




void SimulationView::renderWorld(sf::RenderTarget&target){
    target.setView(m_worldView);
    sf::RenderStates states;
    states.transform = m_worldView.getTransform();// 应用视图的变换矩阵
    states.texture = &m_trackTexture;// 设置要使用的纹理
    states.shader = &m_trackShader;// 设置要使用的着色器
    //m_trackRenderer.draw(target,states);


}

void SimulationView::handleViewEvent(const sf::Event& event, const sf::Vector2f& mousePos){
    // 根据事件类型处理视图事件
    switch (event.type) {
        // 处理鼠标滚轮事件，调整缩放级别
            case sf::Event::MouseWheelMoved:
                m_zoomLevel += event.mouseWheel.delta * 0.05f;
                m_zoomLevel = std::max(0.1f, std::min(m_zoomLevel, 2.0f));
                m_worldView.zoom(1.0f - event.mouseWheel.delta * 0.05f);
                break;
            // 处理鼠标按钮按下事件，开始拖拽
            case sf::Event::MouseButtonPressed:
                if (event.mouseButton.button == sf::Mouse::Left) {
                    m_isDragging = true;
                    m_lastMousePos = mousePos;
                }
                break;
            // 处理鼠标按钮释放事件，结束拖拽
            case sf::Event::MouseButtonReleased:
                if (event.mouseButton.button == sf::Mouse::Left) {
                    m_isDragging = false;
                }
                break;
            // 处理鼠标移动事件，更新视图中心位置
            case sf::Event::MouseMoved:
                if (m_isDragging) {
                    sf::Vector2f delta = m_lastMousePos - sf::Vector2f(mousePos.x, mousePos.y);
                    m_viewCenter -= delta*0.5f;
                    m_worldView.setCenter(m_viewCenter);
                    m_lastMousePos = sf::Vector2f(mousePos.x, mousePos.y);
                }
                break;
            // 默认情况，忽略其他事件
            default:
                break;
        }
}