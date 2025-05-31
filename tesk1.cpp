#include "Core/Vehicle.hpp"
#include "Core/Task.hpp"
#include "Core/Device.hpp"
#include "Core/Logger.hpp"
#include "Core/Scheduler.hpp"
#include <iostream>
#include <cmath>
#include <limits>

#include <chrono>
#include <thread>

#include <cstdlib>
#include <ctime>
float getDistance(float from, float to);
int main(){
Scheduler scheduler;
scheduler.vehicle_manager.initializeVehicles(3);
auto& vehicles = scheduler.vehicle_manager.getVehicles();
srand(0);
int random0 = 1 + rand() % 18;; 
int random1 = 1 + rand() % 18;;
int random2 = 1 + rand() % 18;;
while(random0 ==15){
	random0 = 1 + rand() % 18;
} 
while(random1 ==15){
	random1 = 1 + rand() % 18;
}
while(random2 ==15){
	random2 = 1 + rand() % 18;
}
void updateVehicle1(float current_time, float deltaTime, Vehicle* vehicle, Vehicle* leadingVehicle);

vehicles[0].towards_device=random0;
vehicles[1].towards_device=random1;
vehicles[2].towards_device=random2;
float updateInterval = 0.01;
sf::Clock updateClock;
scheduler.current_time = 0.0; 
std::cout << "start" << std::endl;
std::cout<<"vehicle0 "<<vehicles[0].towards_device<<std::endl;
std::cout<<"vehicle1 "<<vehicles[1].towards_device<<std::endl;
std::cout<<"vehicle2 "<<vehicles[2].towards_device<<std::endl;
while(1){
if(updateClock.getElapsedTime().asSeconds() >= updateInterval){
	updateVehicle1(scheduler.current_time,0.01, &vehicles[0], &vehicles[2]);
	updateVehicle1(scheduler.current_time,0.01, &vehicles[1], &vehicles[0]);
	updateVehicle1(scheduler.current_time,0.01, &vehicles[2], &vehicles[1]);
	std::cout<<vehicles[0].m_state.operationTimer<<std::endl;
	std::cout <<"vehicle0 "<<vehicles[0].m_state.currentSpeed << std::endl;
	std::cout <<"vheicle1 "<<vehicles[1].m_state.currentSpeed << std::endl;
	std::cout <<"vehicle2 "<<vehicles[2].m_state.currentSpeed << std::endl;
	updateClock.restart();
	}
}
}
void updateVehicle1(float current_time, float deltaTime, Vehicle* vehicle, Vehicle* leadingVehicle){

	//前车与后车相对距离
	float distance;
	float epsilon = 0.05f; // 防止浮点数误差
	float device_position[19]={
		-1000.0f,
		85.9209372261538,
		83.5209372261538,
		79.9209372261538,
		77.5209372261538,
		73.9209372261538,
		71.5209372261538,
		67.9209372261538,
		65.5209372261538,
		61.9209372261538,
		59.5209372261538,
		55.9209372261538,
		53.5209372261538,
		32.000,
		29.000,
		26.000,
		14.0,
		11.000,
		8.000,
	};	
    float VehiclePosition = std::fmod(vehicle->m_state.position, 99.47787445225672);
    float LeadingVehiclePosition = std::fmod(leadingVehicle->m_state.position, 99.47787445225672);
	float distancetodevice = VehiclePosition-device_position[vehicle->towards_device];
	if(distancetodevice<0.f){
		distancetodevice += 99.47787445225672;
	}
	//对接任务的接口
    vehicle->position_m = VehiclePosition;
    vehicle->velocity_mps = vehicle->m_state.currentSpeed;
	

	//前车与后车相对距离
	distance = LeadingVehiclePosition - VehiclePosition;
	if(distance<0.f){
		distance += 99.47787445225672;
	}
	//防止浮点数误差
	epsilon = 0.05f;

	
//到车库停车的判断和处理
	// 添加调试输出
	std::cout << "Vehicle " << vehicle->id << " position: " << VehiclePosition 
	          << ", target: " << device_position[vehicle->towards_device] 
	          << ", distance: " << fabs(VehiclePosition - device_position[vehicle->towards_device]) << std::endl;


	if(vehicle->m_state.motionState == Vehicle::MotionState::Stopped && fabs(VehiclePosition - device_position[vehicle->towards_device]) < epsilon) {

		if (vehicle->m_state.operationTimer >= vehicle->m_loadTime) {
			vehicle->m_state.motionState = Vehicle::MotionState::Accelerating;
			vehicle->m_state.operationTimer = 0.0f;
			vehicle->towards_device = 15;
		}
		else {
			vehicle->m_state.operationTimer += deltaTime;
		}

	}
//防碰撞减速
    else if (((vehicle->m_state.currentSpeed)*(vehicle->m_state.currentSpeed)/(2*vehicle->m_acceleration))>=(distance+vehicle->m_length+0.002)){
    
		vehicle->m_state.motionState = Vehicle::MotionState::Decelerating;
        }
//到车库提前减速	
	else if((vehicle->m_state.currentSpeed)*(vehicle->m_state.currentSpeed)/(2*vehicle->m_acceleration)>= (distancetodevice)){

		vehicle->m_state.motionState = Vehicle::MotionState::Decelerating;
										
		}
//弯道减速	
	//处理下面那个弯道的减速
	else if((VehiclePosition>=0.f) && (VehiclePosition<=40.0f)&&((40.0f-VehiclePosition)<=(((vehicle->m_state.currentSpeed)*(vehicle->m_state.currentSpeed))-(vehicle->m_maxCurveSpeed)*(vehicle->m_maxCurveSpeed))/(2*vehicle->m_acceleration))){
				vehicle->m_state.motionState = Vehicle::MotionState::Decelerating;
	}
	//处理上面那个弯道的减速
	else if((VehiclePosition>=49.5209372261538)&&(VehiclePosition<=89.5209372261538)&&((87.835981634f-VehiclePosition)<=(((vehicle->m_state.currentSpeed)*(vehicle->m_state.currentSpeed))-(vehicle->m_maxCurveSpeed)*(vehicle->m_maxCurveSpeed))/(2*vehicle->m_acceleration))){
				vehicle->m_state.motionState = Vehicle::MotionState::Decelerating;
	}
//不减速即设定为加速，更快运动
	else{
		vehicle->m_state.motionState = Vehicle::MotionState::Accelerating;
	}
//根据状态确定下一步的操作
	switch (vehicle->m_state.motionState) {
            case Vehicle::MotionState::Accelerating:
		//判断上一辆车的距离
		vehicle->m_state.currentSpeed += vehicle->m_acceleration  * deltaTime;
		//直线上且超过最大速度
				if((VehiclePosition>=0.f)&&(VehiclePosition<=40.0f)&&(vehicle->m_state.currentSpeed > vehicle->m_maxStraightSpeed)){
					vehicle->m_state.currentSpeed = vehicle->m_maxStraightSpeed;
				}
				else if((VehiclePosition>=49.5209372261538)&&(VehiclePosition<=89.5209372261538)&&(vehicle->m_state.currentSpeed > vehicle->m_maxStraightSpeed)){
					vehicle->m_state.currentSpeed = vehicle->m_maxStraightSpeed;
				}
		//弯道上且超过最大速度		
				else if((VehiclePosition>40.0f)&&(VehiclePosition<49.5209372261538)&&(vehicle->m_state.currentSpeed > vehicle->m_maxCurveSpeed)){
					vehicle->m_state.currentSpeed = vehicle->m_maxCurveSpeed;
				}
				else if((VehiclePosition>89.5209372261538)&&(VehiclePosition<99.47787445225672)&&(vehicle->m_state.currentSpeed > vehicle->m_maxCurveSpeed)){
					vehicle->m_state.currentSpeed = vehicle->m_maxCurveSpeed;
				}
			break;
            case Vehicle::MotionState::Decelerating:
                vehicle->m_state.currentSpeed -= vehicle->m_acceleration   * deltaTime;
				if (vehicle->m_state.currentSpeed < 0.0f) {
					vehicle->m_state.currentSpeed= 0.0f;
					}
                if (vehicle->m_state.currentSpeed <= 0.0f) {
                    vehicle->m_state.currentSpeed = 0.0f;
                    vehicle->m_state.motionState= Vehicle::MotionState::Stopped;
                }
            break;
            case Vehicle::MotionState::Cruising:
                break;
            case Vehicle::MotionState::Stopped:
				std::cout<<vehicle->id<<" stopped"<<std::endl;
                break;
            default:
                // 处理未知状态
                break;
        }
//位置的更新
	vehicle->m_state.position += vehicle->m_state.currentSpeed * deltaTime;

}
float getDistance(float from, float to) {
    float d = to - from;
    if (d < 0) d += 99.47787445225672;
    return d;
}