#ifndef __SERVO_H
#define __SERVO_H
// 引脚:
// PB0  ------> TIM3_CH3
#include "PWM.h"

// 舵机相关宏定义
#define Servo_htim 	  htim3
#define Servo_Channel TIM_CHANNEL_3

// *******************函数*******************

// 舵机初始化
void Servo_Init(void) ;

// 舵机调节角度:0度-180度
void Servo_Set_Angle(int Angle) ;

#endif
