#include <SFML/Graphics.hpp>
#include <map>

class SimulationEngine {
private:
    // 仿真核心组件
    sf::Clock m_realClock;            // 真实时间计时器
    float m_simTime = 0.0f;           // 累积仿真时间（秒）
    float m_timeScale = 1.0f;         // 时间缩放系数
    
    // 对象容器
    std::vector<Vehicle> m_vehicles;  // 车辆集合（按轨道顺序存储）
    std::map<int, DeviceBase*> m_devices; // 设备映射表（键值为设备ID）
    
    // 任务系统
    TaskQueue m_pendingTasks;        // 全局待处理任务
    std::unordered_map<int, std::queue<Task>> m_deviceTaskQueues; // 设备专属任务队列

    // 物理参数
    float m_trackLength;              // 轨道总长度（米）
    float m_safetyDistance;           // 安全距离（转换为米）

public:
    // 生命周期管理
    /**
     * @brief 初始化仿真环境
     * @param config 配置数据引用
     * @param initialTasks 初始任务列表
     * @throws std::runtime_error 当设备初始化失败时抛出
     */
    void initialize(const ConfigData& config, const std::vector<Task>& initialTasks);
    
    /**
     * @brief 执行单步仿真计算
     * @param realDelta 真实时间增量（秒）
     * @return 本步消耗的仿真时间（秒）
     */
    float step(float realDelta);
    
    // 任务管理
    /**
     * @brief 添加任务到指定设备队列
     * @param task 新任务对象
     * @param deviceId 目标设备ID
     * @return 是否成功添加（设备存在且符合任务类型）
     */
    bool addTask(const Task& task, int deviceId);
    
    /**
     * @brief 获取指定设备的首个待处理任务
     * @param deviceId 设备ID
     * @return 任务指针（nullptr表示无任务）
     */
    const Task* peekDeviceTask(int deviceId) const;
    
    // 状态查询
    /**
     * @brief 获取指定车辆的详细状态
     * @param vehicleId 车辆索引（非ID）
     * @return 包含位置、速度等信息的结构体
     */
    Vehicle getVehicleStatus(size_t vehicleId) const;
    
    /**
     * @brief 获取设备当前状态
     * @param deviceId 设备ID
     * @return 设备状态枚举（空闲、准备中、忙碌）
     */
    DeviceStatus getDeviceStatus(int deviceId) const;
};