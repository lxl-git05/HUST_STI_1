#include "AllHeader.h"

void Mode_2_Setup(void)
{
    
}

int start = 0 ;

void Mode_2_Loop(void)
{
	OLED_Printf(0, 0, OLED_6X8, "=====Mode_2=====") ;
	if (Serial_GetNewPackageFlag_ABC(&Serial1))
	{
			Serial_SetFloatData(&Serial1, "Kp", "Kp=%f", &PID_Track.Kp);
			Serial_SetFloatData(&Serial1, "Ki", "Ki=%f", &PID_Track.Ki);
			Serial_SetFloatData(&Serial1, "Kd", "Kd=%f", &PID_Track.Kd);
			Serial_SetFloatData(&Serial1, "Goal", "Goal=%f", &PID_Track.goalPoint);
	}
	// 8路二进制 (1=白/0=黑)
	OLED_Printf(0, 16, OLED_8X16, "%d%d%d%d%d%d%d%d",
			Y8_Data[0], Y8_Data[1], Y8_Data[2], Y8_Data[3],
			Y8_Data[4], Y8_Data[5], Y8_Data[6], Y8_Data[7]);
	// 滤波后角度
	OLED_Printf(0, 40, OLED_6X8, "Angle:%.1f deg   ", Y8_Bias);
	if (Key_Check(KEY_1,KEY_SINGLE))
	{
		start = 1 ;
	}
	if (Key_Check(KEY_2,KEY_SINGLE))
	{
		if (Track_Base_Speed > 1)
		{
			Track_Base_Speed = 0 ;
		}
		else
		{
			Track_Base_Speed = 60 ;
		}
	}
}

void Mode_2_Tick(void)
{
//   Serial_printf(&Serial1 , "%.2f,%.2f\n",IMU_Get_Ax() , IMU_Get_Ay()) ;
	if (start == 0 )
	{
		Track_Base_Speed = 0 ;
	}
	Y8_PID_Update() ;
	Serial_printf(&Serial1 , "%.2f,%.2f,%.2f,%.2f\n",PID_Track.goalPoint,PID_Track.realPoint_Now,PID_Track.setPoint,Y8_Bias) ;
}

void Mode_2_Exit(void)
{
		
}
