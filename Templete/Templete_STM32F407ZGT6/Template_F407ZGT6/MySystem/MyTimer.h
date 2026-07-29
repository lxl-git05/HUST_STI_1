#ifndef __TIMER_H
#define __TIMER_H

#include "MySystem.h"

void Timer_Initial(void) ;
void Timer_1ms_Callback(void) ;
void Timer_20ms_Callback(void) ;
void Timer_Stepper1_Pulse_Callback(void) ;
void Timer_Stepper2_Pulse_Callback(void) ;
void Timer_DisableIRQ(void);   // 关闭所有中断（标定/Flash写入独占资源用）
void Timer_EnableIRQ(void);    // 恢复所有中断

#endif
