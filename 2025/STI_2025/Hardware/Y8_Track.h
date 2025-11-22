#ifndef __Y8_TRACK_H
#define __Y8_TRACK_H

#include "main.h"
#include "MyI2C.h"
#include "i2c.h"
#include "stdbool.h"
#include "MyPID.h"
#include "Task.h"

// 枚举
typedef enum
{
	Y8_Init_Pos ,	// 小车在初始位置
	Y8_LR_Pos   ,	// 小车在岔路口
	Y8_Other_Pos 	// 小车在岔路口与初始位置外的其他位置
}Y8_Position_Typedef ;

// ************外部变量声明************

extern uint8_t Y8_Line_Array[9] ;				// 8路传感器数据包
extern Y8_Position_Typedef Y8_Pos ;			// 小车方位参数
extern mytask Y8_Line_Status ;
extern Pid_Typedef Y8_Line_PID ;
extern bool Y8_Lose_Line_isOK;					// 巡线丢线包容度,true为允许丢线,并使4号识别到线
extern float Y8_JQ[9];
extern bool Car_LR_Speed_Mode ;	// 小车的岔路专门处理函数
// ************函数声明************

// 寻迹模块初始化,其实就是PID初始化
void Y8_Line_Init(float kp, float ki, float kd , float OutMax , float OutMin , float ioutMax );
	
// 读取8路数据,并转化为数组,为了方便起见,数组有9位,1-8为有效数据
void Y8_LineSensor_Update(void) ;

// 巡线核心控制函数
void Y8_Line_Control(void) ;

// Y8巡线岔路口判断
bool Y8_is_LR(void) ;

// Y8巡线停止标识判断
bool Y8_is_Init(void) ;

// 电工基地试题
void Car_Task(int Car_Task_Seq) ;

#endif
