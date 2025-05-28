// #include "Physics/TaskManager.hpp"
// #include "Physics/DeviceManager.hpp"
// #include "Physics/AssignTaskToVehicles.hpp"
#include "Core/Logger.hpp"
#include "Core/Scheduler.hpp"
#include <iostream>



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

}
return 0;
}
