#ifndef __TIMER_COUNTER_H
#define __TIMER_COUNTER_H

// 示范:

//Timer_Counter_Begin() ;
// func()	
//Timer_Counter_End() ;

extern float time_us ;				// 代码之间的时间间隔
extern float time_Func_us ;		// 函数两次执行的时间间隔

#include "main.h"
// 计时器初始化
void Timer_Counter_Init(void);
// 计时器开始计时
void Timer_Counter_Begin(void);
// 计时器结束计时
void Timer_Counter_End(void);
// 计算一个多次执行的函数的每次执行间隔时间
void Timer_Counter_Func(void) ;

#endif
