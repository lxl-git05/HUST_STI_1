#ifndef __SERVO_H
#define __SERVO_H
// 引脚:
// PB0  ------> TIM3_CH3
#include "PWM.h"
#include "stdbool.h"	// bool
#include "Task.h"			// 任务调度

extern int Servo_Manu_Num ;					// 舵机手动挡方向

// *******************函数*******************

// 舵机初始化
void Servo_Init(void) ;

// 舵机调节角度:0度-180度
void Servo_Set_Angle(int Angle) ;

// 舵机转动任务,Mode为1代表自动挡,Mode为0代表手动挡
void Servo_Control(bool Servo_Mode) ;

// 舵机自动档执行任务Possess
void Servo_Auto_Task__Possess(void) ;

#endif
