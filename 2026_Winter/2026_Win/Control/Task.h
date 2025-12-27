#ifndef __TASK_H
#define __TASK_H

#include "main.h"

// 任务管理结构体
typedef struct 
{
	uint8_t Flag;
	uint32_t cnt; 
	uint32_t cycle;
	uint8_t Enable;
	void (*callback)(void);   // 新增：回调函数指针
}mytask;

// 任务初始化(setup)
void taskInit(mytask* task,uint32_t cnt_init,uint32_t cycle_init ,void (*callback_func)(void) );
// 任务周期函数(放在定时器)
void task_possess(mytask* task);
// 任务注销函数(deinit)
void taskDeinit(mytask* task);
// 任务暂停函数
void taskStop(mytask* task);
// 任务重新开始函数,基于任务暂停函数
void taskContinue(mytask* task);

// 单次任务处理函数,放在1ms中断
void task_Once_Cnt_Tick(void);
// 单次任务执行函数(含执行完毕回调函数),无需初始化,第一个参数为任务序列号,具有唯一性!
void task_Once_Possess( uint8_t Task_Seq , uint32_t Delay_Time_ms , void (*callback_func)(void)) ;

#endif
