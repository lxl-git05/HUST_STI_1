#ifndef __Y8_TRACK_H
#define __Y8_TRACK_H

#include "main.h"
#include "MyI2C.h"
#include "i2c.h"
#include "stdbool.h"
#include "MyPID.h"
#include "Task.h"

extern uint8_t Y8_Line_Array[9] ;	// 8路传感器数据包

// 寻迹模块初始化,其实就是PID初始化
void Y8_Line_Init(float kp, float ki, float kd , float OutMax , float OutMin , float ioutMax );
	
// 读取8路数据,并转化为数组,为了方便起见,数组有9位,1-8为有效数据
void Y8_LineSensor_Update(void) ;

// 巡线核心控制函数
void Y8_Line_Control(void) ;

// 第一题针对性函数
//void Y8_Task1(void) ;
#endif
