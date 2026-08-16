#include "Mode_4.h"
#include "AllHeader.h"

void Mode_4_Setup(void)
{
	OLED_Clear();
	Con_Task_Init(Control_TaskTable, TASK_COUNT);   // 注册全局任务表
}

void Mode_4_Loop(void)
{
//	OLED_Printf(0,5,OLED_8X16,"================") ;
//	OLED_ShowChinese(0,25,"欢迎使用有衣有靠") ;
//	OLED_Printf(0,45,OLED_8X16,"================") ;
	OLED_Printf(0,0,OLED_8X16,"====Mode_Main====") ;
	// 逻辑
	if (Key_Check(KEY_1 , KEY_SINGLE) || Serial_CheckCmd(&Serial2 , "Hanger_Start"))
	{
		Serial_Clear_ABC(&Serial2) ;
		// 晾衣服
		Robot_Hang_Enqueue() ;
	}
	if (Key_Check(KEY_2 , KEY_SINGLE)|| Serial_CheckCmd(&Serial2 , "Hanger_Back"))
	{
		Serial_Clear_ABC(&Serial2) ;
		// 复位
		Robot_Reset_Start() ;
	}
	if (Key_Check(KEY_1 , KEY_LONG)|| Serial_CheckCmd(&Serial2 , "Hanger_Shou"))
	{
		Serial_Clear_ABC(&Serial2) ;
		// 收衣服
		Robot_Shou_Start();
	}
	// 校准
	Robot_Cmd_Handle(&Serial4) ;  
	
	// 执行
  Con_Task_Loop();  
}

void Mode_4_Tick(void)
{
}

void Mode_4_Exit(void)
{
    
}
