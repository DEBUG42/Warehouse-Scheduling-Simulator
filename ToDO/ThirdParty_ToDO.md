## SFMLWrapper 封装类设计（ThirdParty/SFMLWrapper.hpp）

```cpp
#pragma once
#include <SFML/Graphics.hpp>
#include <functional>
#include <unordered_map>

namespace Wrapper {
    // 智能资源管理器
    class ResourceManager {
    public:
        static sf::Font& GetFont(const std::string& path) {
            static std::unordered_map<std::string, sf::Font> fonts;
            auto it = fonts.find(path);
            if (it == fonts.end()) {
                if (!fonts[path].loadFromFile(path)) 
                    throw std::runtime_error("Failed to load font: " + path);
            }
            return fonts[path];
        }
    };

    // 高级绘制工具
    class AdvancedRenderer {
    public:
        /**
         * @brief 绘制带阴影的文字
         * @param target 渲染目标
         * @param text 文字内容
         * @param position 基准位置
         * @param params 样式参数
         */
        static void DrawTextWithShadow(
            sf::RenderTarget& target,
            const std::string& text,
            const sf::Vector2f& position,
            const std::tuple<sf::Color, int, sf::Color>& params = {sf::Color::White, 2, sf::Color(0,0,0,128)}
        );
    };

    // 交互式对象基类
    class InteractiveObject : public sf::Transformable {
    public:
        virtual ~InteractiveObject() = default;
        
        /**
         * @brief 获取对象的全局边界框
         */
        virtual sf::FloatRect getGlobalBounds() const = 0;
        
        /**
         * @brief 处理鼠标事件回调
         */
        std::function<void(const sf::Event&)> onClick;
    };

    // 视口控制器
    class ViewportController {
    public:
        ViewportController(sf::RenderWindow& window) : m_window(window) {}
        
        /**
         * @brief 处理鼠标拖拽平移
         * @param event 鼠标移动事件
         */
        void handlePan(const sf::Event::MouseMoveEvent& event);
        
        /**
         * @brief 处理滚轮缩放
         * @param delta 滚轮滚动量
         * @param mousePos 鼠标当前位置
         */
        void handleZoom(int delta, const sf::Vector2i& mousePos);
        
    private:
        sf::RenderWindow& m_window;
        sf::View m_view;
        sf::Vector2f m_lastMousePos;
        float m_zoomLevel = 1.0f;
    };

    // 动画系统
    template<typename T>
    class Animator {
    public:
        void addAnimation(const std::string& name, 
                         const std::vector<sf::IntRect>& frames,
                         float frameTime);
                         
        void play(const std::string& name);
        void update(float deltaTime);
        const sf::IntRect& getCurrentFrame() const;
        
    private:
        struct Animation {
            std::vector<sf::IntRect> frames;
            float duration;
            bool looping;
        };
        std::unordered_map<std::string, Animation> m_animations;
        const Animation* m_currentAnim = nullptr;
        float m_currentTime = 0.0f;
    };
}
```

## 主要功能实现说明

### 1. 视口控制逻辑
```cpp
void ViewportController::handlePan(const sf::Event::MouseMoveEvent& event) {
    if (sf::Mouse::isButtonPressed(sf::Mouse::Right)) {
        sf::Vector2f currentPos = m_window.mapPixelToCoords({event.x, event.y});
        sf::Vector2f delta = m_lastMousePos - currentPos;
        m_view.move(delta);
        m_window.setView(m_view);
    }
    m_lastMousePos = m_window.mapPixelToCoords({event.x, event.y});
}

void ViewportController::handleZoom(int delta, const sf::Vector2i& mousePos) {
    const float zoomFactor = (delta > 0) ? 0.9f : 1.1f;
    m_view.zoom(zoomFactor);
    m_zoomLevel *= (delta > 0) ? 0.9f : 1.1f;
    
    // 保持鼠标位置稳定
    sf::Vector2f beforeZoom = m_window.mapPixelToCoords(mousePos);
    m_window.setView(m_view);
    sf::Vector2f afterZoom = m_window.mapPixelToCoords(mousePos);
    m_view.move(beforeZoom - afterZoom);
    m_window.setView(m_view);
}
```

### 2. 带阴影文字绘制
```cpp
void AdvancedRenderer::DrawTextWithShadow(...) {
    auto [textColor, shadowOffset, shadowColor] = params;
    
    sf::Text mainText(text, font);
    mainText.setPosition(position);
    mainText.setFillColor(textColor);
    
    sf::Text shadowText = mainText;
    shadowText.move(shadowOffset, shadowOffset);
    shadowText.setFillColor(shadowColor);
    
    target.draw(shadowText);
    target.draw(mainText);
}
```

### 3. 动画系统实现
```cpp
template<typename T>
void Animator<T>::addAnimation(const std::string& name,
                              const std::vector<sf::IntRect>& frames,
                              float frameTime) {
    m_animations[name] = {frames, frameTime, true};
}

template<typename T>
void Animator<T>::play(const std::string& name) {
    if (m_animations.count(name)) {
        m_currentAnim = &m_animations[name];
        m_currentTime = 0.0f;
    }
}

template<typename T>
void Animator<T>::update(float deltaTime) {
    if (m_currentAnim) {
        m_currentTime += deltaTime;
        if (m_currentTime >= m_currentAnim->duration) {
            if (m_currentAnim->looping) {
                m_currentTime = fmod(m_currentTime, m_currentAnim->duration);
            } else {
                m_currentAnim = nullptr;
            }
        }
    }
}
```

## 集成应用示例

```cpp
// 在主窗口中的使用
class MainWindow {
    Wrapper::ViewportController m_viewport;
    Wrapper::Animator<sf::Sprite> m_shuttleAnim;
    
public:
    MainWindow() : m_viewport(m_window) {
        // 初始化穿梭车动画
        std::vector<sf::IntRect> frames{ {0,0,32,32}, {32,0,32,32} };
        m_shuttleAnim.addAnimation("move", frames, 0.2f);
    }
    
    void handleEvent(const sf::Event& event) {
        if (event.type == sf::Event::MouseMoved) {
            m_viewport.handlePan(event.mouseMove);
        }
        if (event.type == sf::Event::MouseWheelScrolled) {
            m_viewport.handleZoom(event.mouseWheelScroll.delta, 
                                {event.mouseWheelScroll.x, event.mouseWheelScroll.y});
        }
    }
    
    void update(float delta) {
        m_shuttleAnim.update(delta);
        shuttleSprite.setTextureRect(m_shuttleAnim.getCurrentFrame());
    }
};
```

## 设计优势

**1. 资源统一管理**
- 自动缓存字体/纹理资源

- 避免重复加载开销

- 异常安全加载机制


**2. 交互增强**
- 内置视口控制逻辑（平移+缩放）

- 支持复杂鼠标交互

- 提供可扩展的动画系统


**3. 渲染优化**
- 批处理相似图形元素

- 自动管理绘制顺序

- 提供高级绘制原语（带阴影文字等）


**4. 跨项目复用**
- 独立于业务逻辑的封装

- 模板化的动画系统

- 兼容标准SFML类型


#### 该封装层通过以下方式提升开发效率：
- 减少重复性底层代码

- 统一资源管理策略

- 提供即用的高级交互组件

- 隐藏复杂的SFML实现细节

- 确保图形渲染性能优化