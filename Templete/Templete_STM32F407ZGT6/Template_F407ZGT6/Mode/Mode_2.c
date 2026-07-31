#include "AllHeader.h"


void Mode_2_Setup(void)
{
	Oran_Cascade_Init() ;
}

float check = 90;

void Mode_2_Loop(void)
{
	OLED_Printf(0, 0, OLED_6X8, "===Mode_2===") ;
	if (Serial_GetNewPackageFlag_ABC(&Serial1))
	{
			Serial_SetFloatData(&Serial1, "Kp", "Kp=%f", &PID_Oran.Kp);
			Serial_SetFloatData(&Serial1, "Ki", "Ki=%f", &PID_Oran.Ki);
			Serial_SetFloatData(&Serial1, "Kd", "Kd=%f", &PID_Oran.Kd);
			Serial_SetFloatData(&Serial1, "Goal", "Goal=%f", &PID_Oran.goalPoint);
	}
	if (Key_Check(KEY_1,KEY_SINGLE))
	{
		Servo_SetAngle(90) ;
	}
	Servo_SetAngle(check) ;
}

void Mode_2_Tick(void)
{
	Oran_Cascade_Update() ;
	Serial_printf(&Serial1 , "%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n", PID_Oran.goalPoint,PID_Oran.realPoint_Now,PID_Oran.setPoint,
														PID_Oran_Speed.goalPoint , PID_Oran_Speed.realPoint_Now , PID_Oran_Speed.setPoint);
}

void Mode_2_10ms_Tick(void)
{
	
}

void Mode_2_Exit(void)
{
	
}
