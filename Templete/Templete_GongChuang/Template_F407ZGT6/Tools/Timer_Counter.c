#include "Timer_Counter.h"

uint32_t Timer_Counter_Start_count ;
uint32_t Timer_Counter_End_count	 ;
float time_us ;
float time_Func_us ;
float time_ms ;				// 纯粹是看的清楚加入的
float time_Func_ms ;	// 纯粹是看的清楚加入的

void Timer_Counter_Init(void)
{
	// 初始化
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	DWT->CYCCNT = 0;     // 清零
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

void Timer_Counter_Begin(void)
{
	Timer_Counter_Start_count = DWT->CYCCNT;	// 起始时间
}

void Timer_Counter_End()
{
	Timer_Counter_End_count = DWT->CYCCNT ;	// 终止时间
	uint32_t cycles = Timer_Counter_End_count - Timer_Counter_Start_count;	// 间隔时间
	time_us = (float)cycles / (SystemCoreClock / 1000000.0f);			// 得到间隔时间(us)
	time_ms = time_us / 1000 ;
}

void Timer_Counter_Func(void)
{
	static uint32_t Timer_Counter_Before = 0;	// 上次时间

	uint32_t Timer_Counter_Now = DWT->CYCCNT;	// 当前时间

	uint32_t cycles = Timer_Counter_Now - Timer_Counter_Before;					// 计算间隔时间

	time_Func_us = (float)cycles / (SystemCoreClock / 1000000.0f);			// 得到以us为单位的间隔时间
	
	time_Func_ms = time_Func_us / 1000 ;

	Timer_Counter_Before = Timer_Counter_Now ;	// 更新上次时间
}
