#pragma once #include < string> #include < SFML / Graphics.hpp> namespace gui{/**     * @brief 模拟对象类型枚举     */ enum class SimObjectType{Vehicle, // 车辆        Device,     // 设备        Unknown     // 未知    };    /**     * @brief 模拟对象基类     *      * 用于在GUI中表示可选择和检查的对象     */    class SimObject    {    public:        /**         * @brief 构造函数         * @param type 对象类型         * @param id 对象ID         * @param position 对象位置         */        SimObject(SimObjectType type, const std::string& id, const sf::Vector2f& position)            : m_type(type), m_id(id), m_position(position)        {        }        /**         * @brief 虚析构函数         */        virtual ~SimObject() = default;        /**         * @brief 获取对象类型         */        SimObjectType getType() const { return m_type; }        /**         * @brief 获取对象ID         */        const std::string& getId() const { return m_id; }        /**         * @brief 获取对象位置         */        const sf::Vector2f& getPosition() const { return m_position; }        /**         * @brief 设置对象位置         */        void setPosition(const sf::Vector2f& position) { m_position = position; }        /**         * @brief 获取对象描述（虚函数，子类可重写）
*/
    virtual std::string getDescription() const
{
    return "SimObject[" + m_id + "]";
}

protected:
SimObjectType m_type;    // 对象类型
std::string m_id;        // 对象ID
sf::Vector2f m_position; // 对象位置
}
;

} // namespace gui
