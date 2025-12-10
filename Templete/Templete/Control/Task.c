#include "Task.h"

// 只执行一次的任务
#define Task_Once_Max_Num 10	
// 任务序列号,初始在各个位置都是0,执行一次之后置1,代表单次任务已经实现,不再重复执行
int Task_Once_Seq[Task_Once_Max_Num] ;	
// 单次任务注册
typedef struct 
{
	int task_Cnt ;								// 计数器
	void (*callback_func)(void) ;	// 回调函数
}task_Once_Typedef ;
// 单次任务执行数组
task_Once_Typedef task_Once_Array[Task_Once_Max_Num] ;

// 任务初始化(setup)
void taskInit(mytask* task,uint32_t cnt_init,uint32_t cycle_init , void (*callback_func)(void) )  
{
	task->Flag=0;							
	task->cnt=cnt_init;				// 计数器
	task->cycle=cycle_init;		// 计数时长(周期)
	task->Enable=1;						// 任务启动标志位,初始化之后就打开
	task->callback = callback_func;  // 注册任务函数
}

// 任务周期函数(放在定时器)
void task_possess(mytask* task)
{
	// 任务一旦启动开始进行process判断
	if(task->Enable == 1)
	{
		task->cnt++;
		if(task->cnt >= task->cycle)
		{
			task->cnt = 0;
			task->Flag = 1;
			// 自动调用任务回调函数（若存在）
			if(task->callback != NULL)
			{
					task->callback();
					task->Flag = 0;  // 任务执行后自动清零
			}
		}
	}
}

// 任务注销函数(deinit)
void taskDeinit(mytask* task)
{
    if (task == NULL) return;  // 防止空指针访问

    task->Enable = 0;          // 停止任务
    task->Flag = 0;            // 清除任务标志位
    task->cnt = 0;             // 计数清零
    task->cycle = 0;           // 任务周期清零（可选）
    task->callback = NULL;     // 清空回调函数指针，防止误调用
}

// 任务暂停函数
void taskStop(mytask* task)
{
	if (task == NULL) return;  // 防止空指针访问

	task->Enable = 0;          // 停止任务
	task->Flag = 0;            // 清除任务标志位
	task->cnt = 0;             // 计数清零
}

// 任务重新开始函数,基于任务暂停函数
void taskContinue(mytask* task)
{
	if (task == NULL) return;  // 防止空指针访问
	task->Enable = 1;          // 停止任务
}

// 单次任务执行回调函数执行倒计时
void task_Once_Cnt_Tick(void)
{
	// 查询有无忙碌单次任务处理数组
	for (int i = 0 ; i < Task_Once_Max_Num ; i ++ )
	{
		if (task_Once_Array[i].task_Cnt > 0)
		{
			// 在1ms中断中自减
			task_Once_Array[i].task_Cnt -- ;
			if (task_Once_Array[i].task_Cnt == 0)
			{
				if (task_Once_Array[i].callback_func != NULL)
				{
					task_Once_Array[i].callback_func() ;
					task_Once_Array[i].callback_func = NULL ;
				}
			}
		}
	}
}

// 单次任务执行函数(含执行完毕回调函数),无需初始化,第一个参数为任务序列号,具有唯一性!
void task_Once_Possess( uint8_t Task_Seq , uint32_t Delay_Time_ms , void (*callback_func)(void))
{
	if (Task_Once_Seq[Task_Seq] == 1)
	{
		return ;
	}
	else if (Task_Once_Seq[Task_Seq] == 0)
	{
		// 查询空闲单次任务处理函数
		for (int i = 0 ; i < Task_Once_Max_Num ; i ++ )
		{
			if (task_Once_Array[i].task_Cnt == 0)
			{
				task_Once_Array[i].task_Cnt = Delay_Time_ms ;
				task_Once_Array[i].callback_func = callback_func ;
				break ;
			}
		}
		Task_Once_Seq[Task_Seq] = 1 ;	// 注销单次任务
	}
}
