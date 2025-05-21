#pragma once
#include <SFML/Graphics.hpp>

namespace Physics
{

    class TrackSegment
    {
    public:
        // 轨道段类型
        enum class Type
        {
            STRAIGHT,    // 直线段
            CURVE,       // 弯道段
            INTERSECTION // 交叉口
        };

        // 构造函数
        TrackSegment(Type type, float length, float radius = 0.0f)
            : m_type(type), m_length(length), m_radius(radius)
        {
        }

        // 获取轨道段类型
        Type getType() const { return m_type; }

        // 获取轨道段长度（米）
        float getLength() const { return m_length; }

        // 获取弯道半径（米），仅对弯道段有效
        float getRadius() const { return m_radius; }

    private:
        Type m_type;    // 轨道段类型
        float m_length; // 轨道段长度（米）
        float m_radius; // 弯道半径（米），直线段为0
    };

} // namespace Physics