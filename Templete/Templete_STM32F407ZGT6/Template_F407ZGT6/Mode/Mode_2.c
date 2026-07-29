#include "AllHeader.h"

void Mode_2_Setup(void)
{
	Oran_PID_Init() ;
}

void Mode_2_Loop(void)
{
	OLED_Printf(0, 0, OLED_6X8, "=====Oran=====") ;
	// 钢球平衡
	OLED_Printf(0,10,OLED_6X8,"Spd:%.0f Pos:%.0f",Stepper1.Speed_Now , Stepper1.Pos_Now) ;
	// Serial调参
	if (Serial_GetNewPackageFlag_ABC(&Serial1))
	{
			Serial_SetFloatData(&Serial1, "Kp", "Kp=%f", &PID_Oran.Kp);
			Serial_SetFloatData(&Serial1, "Ki", "Ki=%f", &PID_Oran.Ki);
			Serial_SetFloatData(&Serial1, "Kd", "Kd=%f", &PID_Oran.Kd);
			Serial_SetFloatData(&Serial1, "Goal", "Goal=%f", &PID_Oran.goalPoint);
	}
	
	// 
}

void Mode_2_Tick(void)
{
	// 打印信息
	Serial_printf(&Serial1 , "%.2f,%.2f,%.2f\n", PID_Oran.goalPoint,PID_Oran.realPoint_Now,PID_Oran.setPoint );
}

// 10ms定时器（球平衡 PID + IMU 前馈）
void Timer_10ms_Callback(void)
{
    Oran_PID_Update() ;
}

void Mode_2_Exit(void)
{
	
}
