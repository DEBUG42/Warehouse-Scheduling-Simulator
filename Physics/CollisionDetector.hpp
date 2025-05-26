#include <vector>
#include <../Core/Vehicle.hpp>
#include <cmath>
#include <Task.hpp>
//三辆车的运动控制


class VehicleStateMachine {
public:
	//当前小车
    Vehicle* vehicle;
	//当前主小车相对原点位置
	float VehiclePosition;
	//前车相对原点位置
	float LeadingVehiclePosition;
	//前车与后车相对距离
	float distance;
	float epsilon; // 防止浮点数误差
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
	int  towards_device=0;
	float device_position[19]={
		-1000.0f,
		84.035981634f,
		81.635981634f,
		78.035981634f,
		75.635981634f,
		72.035981634f,
		69.635981634f,
		66.035981634f,
		63.635981634f,
		60.035981634f,
		57.635981634f,
		54.035981634f,
		51.635981634f,
		32.000f,
		29.000f,
		26.000f,
		14.000f,
		11.000f,
		8.000f;
	};

public:

     /*
     * @brief 构造函数
     * @param v 车辆对象
     */
    VehicleStateMachine(Vehicle* v) : vehicle(v) {}

    void update(float Timescale,float deltaTime, Vehicle* leadingVehicle) {
	//当前主小车相对原点位置
	VehiclePosition =std::fmod(vehicle->m_state.position,95.671963268f);
	//leadingVehicle相对原点位置
	LeadingVehiclePosition =std::fmod(leadingVehicle->m_state.position,95.671963268f);
	//前车与后车相对距离
	distance = LeadingVehiclePosition - VehiclePosition;
	if(distance<0.f){
		distance += 95.671963268f;
	}
	//防止浮点数误差
	epsilon = 0.002f;

	vehicle->m_state.operationTimer=0.0f;
	
//到车库停车的判断和处理
	if(vehicle->m_state.motionState == Vehicle::MotionState::Stopped && fabs(VehiclePosition - device_position[towards_device])<epsilon){
		vehicle->m_state.operationTimer+=deltaTime;
		
		if (vehicle->m_state.operationTimer >= vehicle->m_loadTime) {
                    vehicle->m_state.motionState = Vehicle::MotionState::Accelerating;
                    vehicle->m_state.currentSpeed = 0.0f;
					vehicle->m_state.operationTimer=0.0f;
		}
	}
//防碰撞减速
    else if (((vehicle->m_state.currentSpeed)*(vehicle->m_state.currentSpeed)/(2*vehicle->m_acceleration))<=(distance+vehicle->m_length+0.002)){
            vehicle->m_state.motionState = Vehicle::MotionState::Decelerating;
        }
//到车库提前减速	
	else if((vehicle->m_state.currentSpeed)*(vehicle->m_state.currentSpeed)/(2*vehicle->m_acceleration)<= ((device_position[towards_device]-VehiclePosition))){
			vehicle->m_state.motionState = Vehicle::MotionState::Decelerating;
		}
//弯道减速	
	//处理下面那个弯道的减速
	else if((VehiclePosition>=0.f) && (VehiclePosition<=40.0f)){
            if((40.0f-VehiclePosition)<=(((vehicle->m_state.currentSpeed)*(vehicle->m_state.currentSpeed))-(vehicle->m_maxCurveSpeed)*(vehicle->m_maxCurveSpeed))/(2*vehicle->m_acceleration)){
				vehicle->m_state.motionState = Vehicle::MotionState::Decelerating;

			}
	}
	//处理上面那个弯道的减速
	else if((VehiclePosition>=47.835981634)&&(VehiclePosition<=87.835981634)){
            if((87.835981634f-VehiclePosition)<=(((vehicle->m_state.currentSpeed)*(vehicle->m_state.currentSpeed))-(vehicle->m_maxCurveSpeed)*(vehicle->m_maxCurveSpeed))/(2*vehicle->m_acceleration)){
				vehicle->m_state.motionState = Vehicle::MotionState::Decelerating;
			}
	}
//不减速即设定为加速，更快运动
	else{
		vehicle->m_state.motionState = Vehicle::MotionState::Accelerating;
	}
//根据状态确定下一步的操作
	switch (vehicle->m_state.motionState) {
            case Vehicle::MotionState::Accelerating:
			//判断上一辆车的距离
			accelerate(Timescale,deltaTime);
			break;
            case Vehicle::MotionState::Decelerating:
                decelerate(Timescale,deltaTime);
                if (vehicle->m_state.currentSpeed <= 0.0f) {
                    vehicle->m_state.currentSpeed = 0.0f;
                    vehicle->m_state.motionState= Vehicle::MotionState::Stopped;
                }
            break;
            case Vehicle::MotionState::Cruising:
                break;
            case Vehicle::MotionState::Stopped:
                break;
            default:
                // 处理未知状态
                break;
        }
//位置的更新
	vehicle->m_state.position += vehicle->m_state.currentSpeed *Timescale* deltaTime;
    }
private:
    void accelerate(float Timescale,float deltaTime) {
        vehicle->m_state.currentSpeed += vehicle->m_acceleration    *Timescale * deltaTime;
//直线上且超过最大速度
		if((VehiclePosition>=0.f)&&(VehiclePosition<=40.0f)&&(vehicle->m_state.currentSpeed > vehicle->m_maxStraightSpeed)){
            vehicle->m_state.currentSpeed = vehicle->m_maxStraightSpeed;
		}
		else if((VehiclePosition>=47.835981634)&&(VehiclePosition<=87.835981634)&&(vehicle->m_state.currentSpeed > vehicle->m_maxStraightSpeed)){
			vehicle->m_state.currentSpeed = vehicle->m_maxStraightSpeed;
		}
//弯道上且超过最大速度		
		else if((VehiclePosition>40.0f)&&(VehiclePosition<47.835981634f)&&(vehicle->m_state.currentSpeed > vehicle->m_maxCurveSpeed)){
			vehicle->m_state.currentSpeed = vehicle->m_maxCurveSpeed;
		}
		else if((VehiclePosition>87.835981634f)&&(VehiclePosition<95.671963268f)&&(vehicle->m_state.currentSpeed > vehicle->m_maxCurveSpeed)){
			vehicle->m_state.currentSpeed = vehicle->m_maxCurveSpeed;
		}
    }

    void decelerate(float Timescale,float deltaTime) {
        vehicle->m_state.currentSpeed -= vehicle->m_acceleration  *Timescale  *Timescale * deltaTime;
        if (vehicle->m_state.currentSpeed < 0.0f) {
            vehicle->m_state.currentSpeed= 0.0f;
        }
    }
	
};


class MotionController3 {
public:
Vehicle myvehicle[3]={
	Vehicle(26.000f,0.0f,Vehicle::MotionState::Accelerating),
	Vehicle(23.800f,0.0f,Vehicle::MotionState::Accelerating),
	Vehicle(21.600f,0.0f,Vehicle::MotionState::Accelerating)
};
VehicleStateMachine vsm1,vsm2,vsm3;
MotionController3() {
	vsm1 = VehicleStateMachine(&myvehicle[0]);
	vsm2 = VehicleStateMachine(&myvehicle[1]);
	vsm3 = VehicleStateMachine(&myvehicle[2]);
}
//void updateVehicles(){
//	vsm1.update(1.0f/60.0f,&myvehicle[2]);
//	vsm2.update(1.0f/60.0f,&myvehicle[0]);
//	vsm3.update(1.0f/60.0f,&myvehicle[1]);
//}
};
class MotionController5 {
public:
    Vehicle myvehicle[5] = {
        Vehicle(26.000f, 0.0f, Vehicle::MotionState::Accelerating),
        Vehicle(23.800f, 0.0f, Vehicle::MotionState::Accelerating),
        Vehicle(21.600f, 0.0f, Vehicle::MotionState::Accelerating),
        Vehicle(19.400f, 0.0f, Vehicle::MotionState::Accelerating),
        Vehicle(17.200f, 0.0f, Vehicle::MotionState::Accelerating)
    };
    VehicleStateMachine vsm1, vsm2, vsm3, vsm4, vsm5;

    MotionController5() {
        vsm1 = VehicleStateMachine(&myvehicle[0]);
        vsm2 = VehicleStateMachine(&myvehicle[1]);
        vsm3 = VehicleStateMachine(&myvehicle[2]);
        vsm4 = VehicleStateMachine(&myvehicle[3]);
        vsm5 = VehicleStateMachine(&myvehicle[4]);
    }

//void updateVehicles(){
//	vsm1.update(1.0f/60.0f,&myvehicle[4]);
//	vsm2.update(1.0f/60.0f,&myvehicle[0]);
//	vsm3.update(1.0f/60.0f,&myvehicle[1]);
//	vsm4.update(1.0f/60.0f,&myvehicle[2]);
//	vsm5.update(1.0f/60.0f,&myvehicle[3]);
//}
};

class MotionController7{
public:
Vehicle myvehicle[7]={
	Vehicle(26.000f,0.0f,Vehicle::MotionState::Accelerating),
	Vehicle(23.800f,0.0f,Vehicle::MotionState::Accelerating),
	Vehicle(21.600f,0.0f,Vehicle::MotionState::Accelerating),
	Vehicle(19.400f,0.0f,Vehicle::MotionState::Accelerating),
	Vehicle(17.200f,0.0f,Vehicle::MotionState::Accelerating),
	Vehicle(15.000f,0.0f,Vehicle::MotionState::Accelerating),
	Vehicle(12.800f,0.0f,Vehicle::MotionState::Accelerating)
};
VehicleStateMachine vsm1,vsm2,vsm3,vsm4,vsm5,vsm6,vsm7;

MotionController7() {
	vsm1 = VehicleStateMachine(&myvehicle[0]);
	vsm2 = VehicleStateMachine(&myvehicle[1]);
	vsm3 = VehicleStateMachine(&myvehicle[2]);
	vsm4 = VehicleStateMachine(&myvehicle[3]);
	vsm5 = VehicleStateMachine(&myvehicle[4]);
	vsm6 = VehicleStateMachine(&myvehicle[5]);
	vsm7 = VehicleStateMachine(&myvehicle[6]);
}
//void updateVehicles(){
//	vsm1.update(1.0f/60.0f,&myvehicle[6]);
//	vsm2.update(1.0f/60.0f,&myvehicle[0]);
//	vsm3.update(1.0f/60.0f,&myvehicle[1]);
//	vsm4.update(1.0f/60.0f,&myvehicle[2]);
//	vsm5.update(1.0f/60.0f,&myvehicle[3]);
//	vsm6.update(1.0f/60.0f,&myvehicle[4]);
//	vsm7.update(1.0f/60.0f,&myvehicle[5]);
//}
};