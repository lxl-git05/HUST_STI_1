// ========================== 业务逻辑模式 ==========================
#include "Mode_5.h"
#include "AllHeader.h"

// 书写任务注册表
static const Task_Descriptor_Typedef Mode5_Table[TASK_COUNT] =
{
	[TASK_Motor_Speed] = {.Setup = Task_Motor_Speed_Setup, .IsExit = Task_Motor_Speed_IsExit},
	[TASK_Motor_Angle] = {.Setup = Task_Motor_Angle_Setup, .IsExit = Task_Motor_Angle_IsExit, .Tick = Task_Motor_Angle_Tick},
	[TASK_WAIT_TIME]   = {.Setup = Task_Wait_Time_Setup  , .IsExit = Task_Wait_Time_IsExit} ,
};

void Mode_5_Setup(void)
{
	OLED_Clear();
	// 初始化注册表
	Con_Task_Init(Mode5_Table , TASK_COUNT) ;
	// 入队任务
	Con_Task_Enqueue(TASK_WAIT_TIME , 3000 , 1 , 0 , 0) ;
}

void Mode_5_Loop(void)
{
	OLED_Printf(0, 0, OLED_6X8, "=====Mode_5=====") ;
	// 任务调度
	Con_Task_Loop();
	// 在循环中还能再干别的任务或者入队新的任务
	if (Key_Check(KEY_1 , KEY_SINGLE))
	{
		Con_Task_Enqueue(TASK_Motor_Angle , 360 , 30 , 0 , 0) ;
	}
	if (Key_Check(KEY_2 , KEY_SINGLE))
	{
		Con_Task_Enqueue(TASK_Motor_Speed , 40 , 5000 , 0 , 0) ;
	}
}

void Mode_5_Tick(void)
{
}

void Mode_5_Exit(void)
{
}
