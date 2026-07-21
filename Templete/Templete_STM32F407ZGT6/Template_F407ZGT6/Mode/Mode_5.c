// ========================== 业务逻辑模式 ==========================
/*
1. px:(640,360)  (930,1200)
2. px:(500,180)	 (1350,680)
3. px:(900,540)

x映射: f(x) = -3 * x + 2850
y映射: g(y) = 2.889 * y + 160

* "@Con_Mode_6:6$#"
*/


#include "Mode_5.h"
#include "AllHeader.h"

// 书写任务注册表
extern Task_Descriptor_Typedef Con_Mode_Table[TASK_COUNT] ;

void Mode_5_Setup(void)
{
	OLED_Clear();
	// 初始化注册表
	Con_Task_Init(Con_Mode_Table , TASK_COUNT) ;
	x_tar = 500 ;
	y_tar = 1000 ;
	// 入队任务
	HAL_Delay(500) ;
}

void Mode_5_Loop(void)
{
	OLED_Printf(0, 0, OLED_6X8, "=====Mode_5=====") ;
	// 任务调度
	Con_Task_Loop();
	// 在循环中还能再干别的任务或者入队新的任务
	if (Key_Check(KEY_1 , KEY_SINGLE))
	{
		Con_Task_Enqueue(Task_Tar_XY 	  , 0 	 , 0 , 0 , 0) ;
		Con_Task_Enqueue(TASK_WAIT_TIME , 1000 , 1 , 0 , 0) ;
	}
	if (Key_Check(KEY_2 , KEY_SINGLE))
	{
		Con_Task_Enqueue(Task_Back 	  , 0 	 , 0 , 0 , 0) ;
		Con_Task_Enqueue(TASK_WAIT_TIME , 1000 , 1 , 0 , 0) ;
	}
	if (Key_Check(KEY_3 , KEY_SINGLE))
	{
		static bool ch = 0 ;
		if (ch)
		{
			Elec_ON() ;
		}
		else
		{
			Elec_OFF() ;
		}
		ch = !ch ;
	}
}

void Mode_5_Tick(void)
{
}

void Mode_5_Exit(void)
{
}
