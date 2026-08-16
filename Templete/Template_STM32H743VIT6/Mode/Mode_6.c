#include "Mode_6.h"
#include "AllHeader.h"

void Mode_6_Setup(void)
{
    OLED_Clear();
    Con_Task_Init(Control_TaskTable, TASK_COUNT);   // 注册全局任务表
}

void Mode_6_Loop(void)
{
	OLED_Printf(0,0,OLED_6X8,"===Mode_6===") ;
	// 单击: 晾衣    双击: 收衣服
	if (Key_Check(KEY_1 , KEY_SINGLE))
	{
		Robot_Hang_Enqueue() ;
	}
	if (Key_Check(KEY_1 , KEY_DOUBLE))
	{
		Robot_Shou_Start() ;
	}
	// LCD 命令（Hanger_Start / Hanger_Shou / 示教 / 直接运动）
	Robot_Cmd_Handle(&Serial4) ;
	// 执行
	Con_Task_Loop();
}

void Mode_6_Tick(void)
{
}

void Mode_6_Exit(void)
{
}
