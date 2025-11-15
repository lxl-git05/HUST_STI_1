#ifndef __Y8_TRACK_H
#define __Y8_TRACK_H

#include "main.h"
#include "MyI2C.h"
#include "i2c.h"
#include "stdbool.h"
#include "MyPID.h"
#include "Task.h"

// ************外部变量声明************

extern uint8_t Y8_Line_Array[9] ;	// 8路传感器数据包
extern bool is_Car_Init_Pos;			// 判断小车是否到达准备进入岔路口的起点线(也就是遇到了停止点位就更新为true)

// ************函数声明************

// 寻迹模块初始化,其实就是PID初始化
void Y8_Line_Init(float kp, float ki, float kd , float OutMax , float OutMin , float ioutMax );
	
// 读取8路数据,并转化为数组,为了方便起见,数组有9位,1-8为有效数据
void Y8_LineSensor_Update(void) ;

// 巡线核心控制函数
void Y8_Line_Control(void) ;

// Y8巡线岔路口判断
bool Y8_is_LR(bool *is_Car_Init_Position) ;

// Y8巡线停止标识判断
bool Y8_is_Init(bool *is_Car_Init_Position) ;

#endif
