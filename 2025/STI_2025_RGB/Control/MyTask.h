#ifndef __MYTASK_H
#define __MYTASK_H

#include "main.h"
#include "stdbool.h"

/*
任务处理系统:
1. 定时执行类任务(包含标志位状态 or 回调函数状态)
2. 只执行一次的任务(延迟 + 后置回调任务) , 想要再次执行需要重启

要求:
1. 简化任务建立和执行逻辑
2. 函数表达清晰明了
*/

// ====================== 周期任务函数 ======================

// 周期任务 : 增加任务到任务总表,放在 setup 
void MyTask_Cycle_Add_New_Task(uint32_t cnt_init,uint32_t cycle_init , void (*callback_func)(void) , bool is_Task_GO_Now , int *Task_Seq) ;

// 周期任务 : 开启任务
void MyTask_Cycle_Start_Task(int Task_Seq) ;

// 周期任务 : 暂停任务
void MyTask_Cycle_Stop_Task(int Task_Seq) ;

// ====================== 单次任务函数 ======================

// 单次任务 : 执行一次 前置任务 + 延迟 + 后置任务 后注销 , *即使在while反复调用*
// 执行码:必须和其他的单次任务的执行码不同,否则无法再次执行
void MyTask_Once_exe(int id , uint32_t wait_ms , void (*pre_func)(void), void (*post_func)(void)) ;

// ====================== 任务执行总函数 ======================
void MyTask_Possess(void) ;	// 放在1ms中断

#endif
