#ifndef __CON_TRACK_H
#define __CON_TRACK_H

#include "main.h"
#include "MyI2C.h"
#include "i2c.h"
#include "stdbool.h"
#include "MyPID.h"
#include "Task.h"
#include "Con_Car.h"
#include "Y8_Track.h"
#include "math.h"

#define M_PI 3.1415927f

extern mytask Y8_Line_Status ;				// 任务:寻迹
extern Pid_Typedef Y8_Line_PID ;
extern float Y8_JQ[9] ;
extern uint8_t Y8_Line_Num ;

// 丢线包容标志位
extern bool Y8_Lose_Line_isOK ;
// 小车的岔路专门处理操作标志位
extern bool Car_LR_Speed_Mode ;	

// 枚举-根据Y8确定小车位置
typedef enum
{
	Y8_Init_Pos ,	// 小车在初始位置
	Y8_LR_Pos   ,	// 小车在岔路口
	Y8_Other_Pos 	// 小车在岔路口与初始位置外的其他位置,暂时不使用
}Y8_Position_Typedef ;

extern Y8_Position_Typedef Y8_Pos ;	

// 枚举-小车

// 寻迹模块初始化,其实就是PID初始化
void Y8_Line_Init(float kp, float ki, float kd , float OutMax , float OutMin , float ioutMax );

// 巡线核心控制函数
void Y8_Line_Control(void) ;

// Y8巡线岔路口判断
bool Y8_is_LR(void) ;

// Y8巡线停止标识判断
bool Y8_is_Init(void) ;

// Y8巡线采样,放入中断1ms计次
void Y8_Error_Update(void) ;

// Y8巡线等停标识判断
bool Y8_is_Wait(void) ;

#endif
