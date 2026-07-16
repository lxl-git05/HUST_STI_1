#ifndef __CON_STEPPER_H
#define __CON_STEPPER_H

#include "MySystem.h"
#include "Stepper_PWM.h"

// 步进电机初始化
void Stepper_Init(void) ;
// 目标角度PID值更新
void Stepper_PID_Tick(uint32_t Gap_Time_ms) ;
// 检测是否到达目标位置
bool Stepper_PID_Is_OK(Stepper_PWM_Typedef *pStepper , int Tolerance_Angle , int Tolerance_Speed) ;

#endif
