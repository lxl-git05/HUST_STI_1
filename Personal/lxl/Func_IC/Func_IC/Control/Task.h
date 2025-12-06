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

#endif

