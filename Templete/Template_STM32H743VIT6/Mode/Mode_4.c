#include "Mode_4.h"
#include "AllHeader.h"

void Mode_4_Setup(void)
{
	OLED_Clear();
	Con_Task_Init(Control_TaskTable, TASK_COUNT);   // 注册全局任务表
}

void Mode_4_Loop(void)
{
	OLED_Printf(0,0,OLED_6X8,"===Mode_4===") ;
	// 逻辑
	if (Key_Check(KEY_1 , KEY_SINGLE))
	{
		Robot_Hang_Enqueue() ;
	}
	if (Key_Check(KEY_2 , KEY_SINGLE))
	{
		Robot_Reset_Start() ;
	}
	
	// 执行
  Con_Task_Loop();  
}

void Mode_4_Tick(void)
{
}

void Mode_4_Exit(void)
{
    
}
