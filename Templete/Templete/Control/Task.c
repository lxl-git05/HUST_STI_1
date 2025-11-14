#include "Task.h"

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
