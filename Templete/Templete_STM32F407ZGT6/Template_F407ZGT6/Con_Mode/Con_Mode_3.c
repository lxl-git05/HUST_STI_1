#include "Con_Mode_3.h"

// 全局变量
int Mode_3_Move = 0 ;

void Con_Mode_3_Setup(void)
{
    OLED_Clear();
		// 任务队列初始化
		Con_Task_Init(Con_Mode_Table , TASK_COUNT) ;
		// 打印日志
		Serial_printf(&Serial2 , "@Con_Mode_3:3$#");
}

void Con_Mode_3_Loop(void)
{
	OLED_Printf(0, 0, OLED_6X8, "=====Con_Mode_3=====") ;
	// 监听串口4(LCD)
	if (Serial_GetNewPackageFlag_ABC(&Serial4))
	{
		// 检测黑色棋子
		if (Serial_SetIntData(&Serial4 , "Move" , "Move=%d" , &Mode_3_Move))
		{
			// 发回给香橙派
			Serial_printf(&Serial2 , "@Move:%d$#",Mode_3_Move) ;
		}
	}
	// 监听串口2(香橙派)
	if (Serial_GetNewPackageFlag_ABC(&Serial2))
	{
		// 1. Tar任务
		if (Serial_Check_Str(&Serial2 , "TarXY"))
		{
			// 开始进行(x,y)位置定位
			Con_Task_Enqueue(Task_Tar_XY , Tar_XY_Tol_Distance , Tar_XY_Tol_Speed , 0 , 0) ;
		}
		// 2. Down任务(其实是拆分成了3个小任务:下降->取/放棋子->上升，只有在上升的时候会发OK)
		if (Serial_Check_Str(&Serial2 , "Down"))
		{
			// 开始进行棋子拿取or放置
			Con_Task_Enqueue(Task_Down , Down_Tar_Angle , Down_Tol_Angle , 0 , 0) ;
			Con_Task_Enqueue(Task_Elec , Elec_Wait , 0 , 0 , 0) ;
			Con_Task_Enqueue(Task_Up	 , Up_Tar_Angle , Up_Tol_Angle , 0 , 0) ;
		}
		// 3. Back任务
		if (Serial_Check_Str(&Serial2 , "Back"))
		{
			// 回家
			Con_Task_Enqueue(Task_Back , Back_Tar_Angle , Back_Speed_MAX , Back_Acc , Back_Tol_Angle) ;
		}
	}
	// OLED展示
	OLED_Printf(0,15,OLED_6X8,"Curr_Task:%d",Con_Task_CurrType()) ;
	
	// 任务调度
	Con_Task_Loop();
}

void Con_Mode_3_Tick(void)
{
}

void Con_Mode_3_Exit(void)
{
    OLED_Clear();
}
