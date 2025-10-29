#include "Mymain.h"




void Mymain(void)
{
	
		
}



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
