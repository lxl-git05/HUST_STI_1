#include "AllHeader.h"

int cmd = 0 ;	// 使用debug模式进行参数修改

void Mode_2_Setup(void)
{
	
}

void Mode_2_Loop(void)
{
	OLED_Printf(0, 0, OLED_6X8, "===Mode_2===") ;
	// 指令实现
	if (Key_Check(KEY_1 , KEY_SINGLE) || cmd == 1)
	{
		Emm_V5_Vel_Control(1, 0, 500, 10, 0);
	}
	if (Key_Check(KEY_2 , KEY_SINGLE) || cmd == 2)
	{
		Emm_V5_Vel_Control(1, 0, 0, 10, 0);
	}
	
	
	// 指令只允许发送一次
	if (cmd != 0)
		cmd = 0 ;
}

void Mode_2_Tick(void)
{
	Serial_printf(&Serial1 , "%.2f\n",IMU_Yaw_Abs_Get()) ;
}

void Mode_2_Exit(void)
{
	
}
