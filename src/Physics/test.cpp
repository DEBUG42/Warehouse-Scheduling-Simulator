//引入头文件
//使用方法：根据确定的车辆数目 在你的对应的update下面加入车辆的控制代码（车辆控制代码加在渲染前面）
#include "CollisionDetector.hpp"

//控制三辆车
/*
	MotionController3 controller;
	//deltatime放你的刷新频率对应的那个deltatime变量就行
	controller.vsm1.update(Timescale,deltaTime,&controller.myvehicle[2]);
    controller.vsm2.update(Timescale,deltaTime,&controller.myvehicle[0]);
	controller.vsm3.update(Timescale,deltaTime,&controller.myvehicle[1]);
*/
//控制五辆车
/*
	MotionController5 controller;
	//deltatime放你的刷新频率对应的那个deltatime变量就行
	controller.vsm1.update(Timescale,deltaTime,&controller.myvehicle[4]);
	controller.vsm2.update(Timescale,deltaTime,&controller.myvehicle[0]);
	controller.vsm3.update(Timescale,deltaTime,&controller.myvehicle[1]);
	controller.vsm4.update(Timescale,deltaTime,&controller.myvehicle[2]);
	controller.vsm5.update(Timescale,deltaTime,&controller.myvehicle[3]);
*/
//控制七辆车
/*
	MotionController7 controller;
	//deltatime放你的刷新频率对应的那个deltatime变量就行
	controller.vsm1.update(Timescale,deltaTime,&controller.myvehicle[6]);
	controller.vsm2.update(Timescale,deltaTime,&controller.myvehicle[0]);
	controller.vsm3.update(Timescale,deltaTime,&controller.myvehicle[1]);
	controller.vsm4.update(Timescale,deltaTime,&controller.myvehicle[2]);
	controller.vsm5.update(Timescale,deltaTime,&controller.myvehicle[3]);
	controller.vsm6.update(Timescale,deltaTime,&controller.myvehicle[4]);
	controller.vsm7.update(Timescale,deltaTime,&controller.myvehicle[5]);
*/

int main(){
MotionController3 controller;
controller.vsm1.VehiclePosition;
controller.myvehicle[1]
}
