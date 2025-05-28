// #include "Physics/TaskManager.hpp"
// #include "Physics/DeviceManager.hpp"
// #include "Physics/AssignTaskToVehicles.hpp"
 #include "Core/Logger.hpp"
 #include "Core/Scheduler.hpp"
#include <iostream>
#include <chrono>
#include <thread>



int main() {
    Scheduler scheduler;
	scheduler.vehicle_manager.initializeVehicles(3);
	auto& vehicles = scheduler.vehicle_manager.getVehicles();
//while(1){
    scheduler.run(100);
    
    
    // 正确获取三辆车的信息
    if (vehicles.size() >= 3) {
		if (vehicles[0].m_state.motionState==Vehicle::MotionState::Stopped)
			std::cout << "第一辆车停车" << std::endl;
    //     // 第一辆车 (索引0, ID 0)
    //     printf("\n第一辆车(ID:%d): 位置=%.2fm 速度=%.2fm/s\n", 
    //           vehicles[0].id, vehicles[0].position_m, vehicles[0].velocity_mps);
        
         // 第二辆车 (索引1, ID 1)
         printf("第二辆车(ID:%d): 位置=%.2fm 速度=%.2fm/s\n", 
               vehicles[1].id, vehicles[1].position_m, vehicles[1].velocity_mps);
        const int target_fps = 1;
    // 计算每次循环应该间隔的时间
    const std::chrono::duration<double> frame_duration(1.0 / target_fps);

    while (true) {
        auto start_time = std::chrono::high_resolution_clock::now();

        // 这里放置你想要重复执行的操作
        std::cout << "执行操作" << std::endl;
		
        // 计算当前时间与开始时间的差值
        auto elapsed_time = std::chrono::high_resolution_clock::now() - start_time;
        // 如果差值小于每帧的时间，则休眠剩余时间
        if (elapsed_time < frame_duration) {
            std::this_thread::sleep_for(frame_duration - elapsed_time);
        }
    }
    //     // 第三辆车 (索引2, ID 2)
    //     printf("第三辆车(ID:%d): 位置=%.2fm 速度=%.2fm/s\n", 
    //           vehicles[2].id, vehicles[2].position_m, vehicles[2].velocity_mps);
        
    //     // 验证车辆运动状态
    //     if(vehicles[0].velocity_mps == 0 && 
    //        vehicles[1].velocity_mps == 0 && 
    //        vehicles[2].velocity_mps == 0) {
    //         printf("\n警告：所有车辆速度为零，运动可能未启动\n");
    //     }
    // }
}

    return 0;

}
// Scheduler scheduler;
// scheduler.vehicle_manager.initializeVehicles(3)

// scheduler.run(100/Timescale,timescale);