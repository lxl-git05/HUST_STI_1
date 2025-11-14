#ifndef __RGB_H
#define __RGB_H
// 引脚:
//R:A0 ,TIM2_CH1  
//G:A1 ,TIM2_CH2  
//B:A2 ,TIM2_CH3

#include "PWM.h"
#include "stdbool.h"	// bool
#include "Task.h"			// 任务调度
#include "Key.h"			// 按键控制RGB

// RGB初始化
void RGB_Init(void) ;

// RGB调色
void RGB_Set_Color(int R_Color , int G_Color , int B_Color ) ;

// RGB闪烁任务,Mode为1代表自动挡,Mode为0代表手动挡
void RGB_Control(bool Mode) ;

// RGB自动档执行任务
void RGB_Auto_Task__Possess(void) ;

#endif
