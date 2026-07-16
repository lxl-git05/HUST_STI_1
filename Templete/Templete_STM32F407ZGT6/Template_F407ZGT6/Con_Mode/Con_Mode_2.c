#include "Con_Mode_2.h"
// 全局变量
int Mode_2_Move = 0 ;

void Con_Mode_2_Setup(void)
{
    OLED_Clear();
	// 任务队列初始化
	Con_Task_Init(Con_Mode_Table , TASK_COUNT) ;
	// 打印日志
	Serial_printf(&Serial2 , "@Con_Mode_2:2$#");
}



void Con_Mode_2_Loop(void)
{
	OLED_Printf(0, 0, OLED_6X8, "=====Con_Mode_2=====") ;
	// 监听串口4(LCD)
	if (Serial_GetNewPackageFlag_ABC(&Serial4))
	{
		// 检测黑色棋子
		if (Serial_SetIntData(&Serial4 , "Move" , "Move=%d" , &Mode_2_Move))
		{
			// 发回给香橙派
			Serial_printf(&Serial2 , "@Move:%d$#",Mode_2_Move) ;
		}
	}
	// 监听串口2(香橙派)
	if (Serial_GetNewPackageFlag_ABC(&Serial2))
	{
		// 1. Tar任务
		if (Serial_Check_Str(&Serial2 , "TarXY"))
		{
			// 开始进行(x,y)位置定位
			Con_Task_Enqueue(Task_Tar_XY , 1000 , 1 , 0 , 0) ;
		}
		// 2. Down任务(其实是拆分成了3个小任务:下降->取/放棋子->上升，只有在上升的时候会发OK)
		if (Serial_Check_Str(&Serial2 , "Down"))
		{
			// 开始进行棋子拿取or放置
			Con_Task_Enqueue(Task_Down , 2000 , 1 , 0 , 0) ;
			Con_Task_Enqueue(Task_Elec , 1000 , 1 , 0 , 0) ;
			Con_Task_Enqueue(Task_Up	 , 2000 , 1 , 0 , 0) ;
		}
		// 3. Back任务
		if (Serial_Check_Str(&Serial2 , "Back"))
		{
			// 回家
			Con_Task_Enqueue(Task_Back , 3000 , 1 , 0 , 0) ;
		}
	}
	// OLED展示
	OLED_Printf(0,15,OLED_6X8,"Curr_Task:%d",Con_Task_CurrType()) ;
	
	// 任务调度
	Con_Task_Loop();
}

void Con_Mode_2_Tick(void)
{
}

void Con_Mode_2_Exit(void)
{
    OLED_Clear();
}
