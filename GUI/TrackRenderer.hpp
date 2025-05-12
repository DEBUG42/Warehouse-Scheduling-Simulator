#include <SFML/Graphics.hpp>
class TrackRenderer : public sf::Drawable {
private:
    // 轨道几何数据
    sf::VertexArray m_straightSegments; // 直轨顶点数组
    sf::VertexArray m_curveSegments;    // 弯轨顶点数组
    
    // 样式参数
    const float m_trackWidth = 4.0f;    // 轨道线宽
    sf::Color m_straightColor {180, 180, 180}; // 直轨颜色
    sf::Color m_curveColor {160, 160, 160};   // 弯轨颜色

public:
    /**
     * @brief 根据物理参数生成轨道几何形状
     * @param trackLength 轨道总长
     * @param curveRadius 弯道半径
     */
    void generateGeometry(float trackLength, float curveRadius);

protected:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
};