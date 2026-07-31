#include "Mode_6.h"
#include "AllHeader.h"

void Mode_6_Setup(void)
{
	Oran_Speed_PID_Init() ;
}

void Mode_6_Loop(void)
{
	OLED_Printf(0, 0, OLED_6X8, "===Mode_6===") ;
	if (Serial_GetNewPackageFlag_ABC(&Serial1))
	{
			Serial_SetFloatData(&Serial1, "Kp",  "Kp=%f",  &PID_Oran_Speed.Kp);
			Serial_SetFloatData(&Serial1, "Ki",  "Ki=%f",  &PID_Oran_Speed.Ki);
			Serial_SetFloatData(&Serial1, "Kd",  "Kd=%f",  &PID_Oran_Speed.Kd);
			Serial_SetFloatData(&Serial1, "Goal","Goal=%f",&PID_Oran_Speed.goalPoint);
	}
}

void Mode_6_Tick(void)
{
}

void Mode_6_10ms_Tick(void)
{
	Oran_Speed_PID_Update() ;
}

void Mode_6_Exit(void)
{
	Stepper_PWM_Stop(&Stepper1);
}
