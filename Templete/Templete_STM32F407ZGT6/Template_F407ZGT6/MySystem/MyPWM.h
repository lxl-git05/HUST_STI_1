#ifndef __MyPWM_H
#define __MyPWM_H

#include "MySystem.h"

typedef struct {
    TIM_HandleTypeDef *htimx;   // TIM外设句柄
    uint32_t Channel;           // 通道号
    float Compare_Max;          // SetCompare上限
    float Compare_Min;          // SetCompare下限
    uint32_t Tim_Clock;         // 定时器输入时钟频率(Hz)，0=使用MySystem_Fre兜底
    IRQn_Type Tim_IRQn;         // 定时器中断号（用于脉冲中断，电机PWM可不填）
} MyPWM_Typedef;

// PWM外部实例（在MySystem.c中定义）
// extern MyPWM_Typedef MyPWM_Servo1;
// extern MyPWM_Typedef MyPWM_Servo2;
// extern MyPWM_Typedef MyPWM_Servo3;
extern MyPWM_Typedef MyPWM_Servo4;
extern MyPWM_Typedef MyPWM_Motor_A_IN1;
extern MyPWM_Typedef MyPWM_Motor_B_IN1;

extern MyPWM_Typedef MyPWM_Stepper1 ;
extern MyPWM_Typedef MyPWM_Stepper2 ;

// PWM初始化
void MyPWM_Init(MyPWM_Typedef *pwm);
// 设置PWM比较值（自动限幅到[Compare_Min, Compare_Max]）
void MyPWM_SetCompare(MyPWM_Typedef *pwm, float compare);
// 获取PWM频率
int MyPWM_GetFre(MyPWM_Typedef *pwm);
// 设置定时器周期值（ARR），用于步进电机动态调速
void MyPWM_SetLoadValue(MyPWM_Typedef *pwm, uint32_t load);
// 获取定时器输入时钟频率(Hz)
uint32_t MyPWM_GetTimClock(MyPWM_Typedef *pwm);
// 使能定时器更新中断（用于脉冲计数）
void MyPWM_EnableIT(MyPWM_Typedef *pwm);

#endif
