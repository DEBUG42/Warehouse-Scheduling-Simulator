// 包含必要的标准库头文件
#include <string>
#include <map>
#include <queue>
#include <vector>
#include <SFML/System.hpp>
// 定义任务类型枚举，包括入库和出库两种类型
enum TaskType { INBOUND, OUTBOUND };

struct Task {
    int id;                     // 任务唯一编号
    TaskType type;                   // 入库/出库任务
    int materialId;                  // 物料编号
    int startDeviceId;               // 起始设备ID
    int endDeviceId;                 // 目标设备ID
    double assign_time, pick_time, drop_time, complete_time; // 任务开始时间、取件时间、放件时间、完成时间
    int assignedVehicleId = -1;      // 分配的车辆ID
    bool is_assigned;                // 是否已分配
};


//最新的类
class TaskManager {
public:
    void loadFromFile(const std::string& path);
    std::vector<Task*> getReadyTasks(double current_time, const DeviceManager& device_manager);
    bool allTasksCompleted() const;
    void markTaskAssigned(int task_id, int car_id, double current_time);

private:
    std::vector<Task> tasks;
    std::map<int, int> next_task_id; // 设备编号 → 当前应执行任务编号
};
