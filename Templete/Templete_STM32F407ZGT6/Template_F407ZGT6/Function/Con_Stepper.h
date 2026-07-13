#ifndef __CON_STEPPER_H
#define __CON_STEPPER_H

#include "MySystem.h"
#include "Stepper_PWM.h"

// 步进电机初始化
void Stepper_Init(void) ;
// 目标角度PID值更新
void Stepper_PID_Tick(uint32_t Gap_Time_ms) ;

#endif
