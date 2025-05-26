#pragma once
#include <SFML/Graphics.hpp>
#include <map>
#include <queue>
#include <Taskscheduler.hpp>
/*
p
/*
pi=3.14159265358979323846
18 8000
17 11000
16 14000
15 26000
14 29000
13 32000
12 40000+2500pi+3800=43800+7500pi=   51635.981634
11 40000+2500pi+6200=46200+7500pi=   54035.981634
10 40000+2500pi+9800=49800+7500pi=   57635.981634
9 40000+2500pi+12200=52200+7500pi=   60035.981634
8 40000+2500pi+15800=55800+7500pi=   63635.981634
7 40000+2500pi+18200=58200+7500pi=   66035.981634
6 40000+2500pi+21800=61800+7500pi=   69635.981634
5 40000+2500pi+24200=64200+7500pi=   72035.981634
4 40000+2500pi+27800=67800+7500pi=   75635.981634
3 40000+2500pi+30200=70200+7500pi=   78035.981634
2 40000+2500pi+33800=73800+7500pi=   81635.981634
1 40000+2500pi+36200=76200+7500pi=   84035.981634
*/
// 设备类型枚举
enum class DeviceType {
    StorageIn,      // 入库接口设备（1,3,5,7,9,11）
    StorageOut,     // 出库接口设备（2,4,6,8,10,12）
    WorkstationIn,  // 入库作业口（16,17,18）
    WorkstationOut  // 出库作业口（13,14,15）
};

enum class DeviceStatus {
    working,        // 工作中
    idle,           // 空闲中
    preparing,      // 准备中
};

// 设备基类
class DeviceBase {
public:
    const int m_id;                   // 设备唯一标识
    std::queue<Task> m_taskQueue;     // 任务等待队列

    float m_storageIn =30.0; // 入库时间（秒）
    float m_storageOut =25.0; // 出库时间（秒）
	DeviceBase(int id, DeviceType type);
    DeviceStatus m_status;
};


//设备管理函数类
class DeviceManager {
public:
    void updateDevice(double current_time);
    const DeviceState& getState(int device_id) const;
    void reserveDevice(int device_id, int task_id, double until_time);
    void releaseDevice(int device_id, int task_id);
    void handleEvent(const Event& e); // 在 Scheduler 中转发事件时使用

private:
    std::map<int, DeviceState> devices;
};
