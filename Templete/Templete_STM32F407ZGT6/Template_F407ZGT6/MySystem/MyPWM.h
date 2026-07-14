#ifndef __MyPWM_H
#define __MyPWM_H

#include "MySystem.h"

typedef struct {
    TIM_HandleTypeDef *htimx;   // TIM外设句柄
    uint32_t Channel;           // 通道号
    float Compare_Max;          // SetCompare上限
    float Compare_Min;          // SetCompare下限
} MyPWM_Typedef;

// PWM外部实例（在MySystem.c中定义）
// extern MyPWM_Typedef MyPWM_Servo1;
// extern MyPWM_Typedef MyPWM_Servo2;
// extern MyPWM_Typedef MyPWM_Servo3;
// extern MyPWM_Typedef MyPWM_Servo4;
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

#endif
