// 包含必要的标准库头文件
#include <string>
#include <map>
#include <queue>
#include <vector>
#include <SFML/System.hpp>
// 定义任务类型枚举，包括入库和出库两种类型
enum TaskType { INBOUND, OUTBOUND };

// 定义任务结构体，包含任务的ID、物料ID、任务类型、起始设备ID和结束设备ID
struct Task {
    int id;
    std::string material_id; // 物料的唯一标识符
    TaskType type;     // 任务类型，可以是入库或出库
    int start_device;  // 任务起始的设备ID
    int end_device;    // 任务结束的设备ID
}

    //根据设备ID输出设备坐标和坐标状态传到device类里面
void outputDeviceStatus(int device_id) const {
    };

// 任务调度器类，用于管理和调度仓库中的任务
class TaskScheduler {
public:
    // 从文件中加载任务，文件名作为参数传入
    bool loadTasks(const std::string& filename);

    // 检查指定设备ID是否有待处理的任务
    bool hasTasks(int device_id) const;

    // 获取指定设备ID的下一个任务
    Task getNextTask(int device_id);

    //每个时间步或事件触发时调用的调度主控函数
    void onEvent(float m_simTime)



private:
    // 使用map存储每个设备的任务队列，map的键是设备ID，值是任务队列
    std::map<int, std::queue<Task>> task_queue_map;
};
