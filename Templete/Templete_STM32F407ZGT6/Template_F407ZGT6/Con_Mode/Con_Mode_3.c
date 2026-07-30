#include "Con_Mode_3.h"

void Con_Mode_3_Setup(void)
{
    Oran_PID_Init() ;
}

void Con_Mode_3_Loop(void)
{
	OLED_Printf(0, 0, OLED_6X8, "=====Con_Mode_3=====") ;
	Serial_SetFloatData(&Serial1, "Kp", "Kp=%f", &PID_Oran.Kp);
	Serial_SetFloatData(&Serial1, "Ki", "Ki=%f", &PID_Oran.Ki);
	Serial_SetFloatData(&Serial1, "Kd", "Kd=%f", &PID_Oran.Kd);
	Serial_SetFloatData(&Serial1, "Goal", "Goal=%f", &PID_Oran.goalPoint);
}

void Con_Mode_3_Tick(void)
{
	// 位置PID → 位置
	Oran_PID_Update() ;         
	// 位置PID CSV: Goal, Real, Set
	Serial_printf(&Serial1 , "%.2f,%.2f,%.2f\n", PID_Oran.goalPoint, PID_Oran.realPoint_Now, PID_Oran.setPoint);
}

void Con_Mode_3_Exit(void)
{
	Stepper_PWM_Stop(&Stepper1);
}
