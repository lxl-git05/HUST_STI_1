// ========================== Con_Mode_1 ==========================
#include "Con_Mode_1.h"

// 书写任务注册表
Task_Descriptor_Typedef Con_Mode_Table[TASK_COUNT] =
{
	[TASK_Motor_Speed] = {.Setup = Task_Motor_Speed_Setup, .IsExit = Task_Motor_Speed_IsExit},
	[TASK_Motor_Angle] = {.Setup = Task_Motor_Angle_Setup, .IsExit = Task_Motor_Angle_IsExit , 
												.Tick  = Task_Motor_Angle_Tick},
	[Task_Tar_XY] 	   = {.Setup = Task_Tar_XY_Setup , .IsExit = Task_Tar_XY_IsExit , 
												.Run   = Task_Tar_XY_Run   , .Tick   = Task_Tar_XY_Tick},
	[Task_Down] 	  	 = {.Setup = Task_Down_Setup   , .IsExit = Task_Down_IsExit   ,
												.Run   = Task_Down_Run   	 , .Tick   = Task_Down_Tick},
	[Task_Back] 	  	 = {.Setup = Task_Back_Setup   , .IsExit = Task_Back_IsExit   ,
												.Run   = Task_Back_Run   	 , .Tick   = Task_Back_Tick},
	[Task_Up]					 = {.Setup = Task_Up_Setup  	 , .IsExit = Task_Up_IsExit   ,
												.Run   = Task_Up_Run   		 , .Tick   = Task_Up_Tick},
	[Task_Elec]				 = {.Setup = Task_Elec_Setup	 , .IsExit = Task_Elec_IsExit}
};

// 全局变量
int Mode_1_Move = 0 ;

void Con_Mode_1_Setup(void)
{
	// 任务队列初始化
	Con_Task_Init(Con_Mode_Table , TASK_COUNT) ;
	// 打印日志
//	Serial_printf(&Serial1 , "=====Con_Mode_1=====\r\n");
	Serial_printf(&Serial2 , "@Con_Mode_1:1$#");
}

void Con_Mode_1_Loop(void)
{
  OLED_Printf(0, 0, OLED_6X8, "=====Con_Mode_1=====");
	
	// 监听串口4(LCD)
	if (Serial_GetNewPackageFlag_ABC(&Serial4))
	{
		// 检测黑色棋子
		if (Serial_SetIntData(&Serial4 , "Move" , "Move=%d" , &Mode_1_Move))
		{
			// 发回给香橙派
			Serial_printf(&Serial2 , "@Move:%d$#",Mode_1_Move) ;
		}
	}
	// 监听串口2(香橙派)
	if (Serial_GetNewPackageFlag_ABC(&Serial2) || 1)
	{
		// 1. Tar任务
		if (Serial_Check_Str(&Serial2 , "TarXY") || Key_Check(KEY_1 , KEY_SINGLE))
		{
			// 开始进行(x,y)位置定位
			Con_Task_Enqueue(Task_Tar_XY , Tar_XY_Tol_Distance , Tar_XY_Tol_Speed , 0 , 0) ;
		}
		// 2. Down任务(其实是拆分成了3个小任务:下降->取/放棋子->上升，只有在上升的时候会发OK)
		if (Serial_Check_Str(&Serial2 , "Down") || Key_Check(KEY_2 , KEY_SINGLE) )
		{
			// 开始进行棋子拿取or放置
			Con_Task_Enqueue(Task_Down , Down_Tar_Angle , Down_Tol_Angle , 0 , 0) ;
			Con_Task_Enqueue(Task_Elec , Elec_Wait , 0 , 0 , 0) ;
			Con_Task_Enqueue(Task_Up	 , Up_Tar_Angle , Up_Tol_Angle , 0 , 0) ;
		}
		// 3. Back任务
		if (Serial_Check_Str(&Serial2 , "Back") || Key_Check(KEY_3 , KEY_SINGLE) )
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

void Con_Mode_1_Tick(void)
{
	
}

void Con_Mode_1_Exit(void)
{
    
}
