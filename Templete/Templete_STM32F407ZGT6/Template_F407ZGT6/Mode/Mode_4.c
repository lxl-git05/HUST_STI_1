#include "Mode_4.h"
#include "AllHeader.h"

float Mode4_Tar_Speed = 0 ;
int cnt = 5 ;

void Mode_4_Setup(void)
{
	Y8U_SetSpeed(0);
}

void Mode_4_Loop(void)
{
    OLED_Printf(0, 0, OLED_6X8, "Mode_4 Spd:%.0f", Y8U_GetSpeed());
		// 调试PID
		if (Serial_GetNewPackageFlag_ABC(&Serial1))
		{
				Serial_SetFloatData(&Serial1, "Kp", "Kp=%f", &Y8U_PID.Kp);
				Serial_SetFloatData(&Serial1, "Ki", "Ki=%f", &Y8U_PID.Ki);
				Serial_SetFloatData(&Serial1, "Kd", "Kd=%f", &Y8U_PID.Kd);
				Serial_SetFloatData(&Serial1, "Goal", "Goal=%f", &Y8U_PID.goalPoint);
		}
    // KEY1: 赋予基础速度 120
    OLED_Printf(0, 10, OLED_6X8, "KEY1:Go");
    if (Key_Check(KEY_1, KEY_SINGLE)) 
		{
        Mode4_Tar_Speed = 90 ;
    }
		// 新增分频
		if (Key_Check(KEY_2,KEY_SINGLE))
		{
			++cnt ;
		}
		OLED_Printf(0,20,OLED_6X8,"cnt:%d",cnt) ;
}

void Mode_4_Tick(void)
{
		Y8U_PID_Update();
		Motor_Pos_Update(&Motor_A);
		Motor_Pos_Update(&Motor_B);

		// 横线终点检测
		if (IMU_Yaw_Abs_Get() > 330.0f && Y8U_CheckFinishLine()) 
		{
				Y8U_SetSpeed(0);
				Motor_SetSpeed(&Motor_A, 0);
				Motor_SetSpeed(&Motor_B, 0);
		}
		
		// 小车速度更新(缓慢启停)
		Y8U_RampTick(Mode4_Tar_Speed , cnt) ;

		Serial_printf(&Serial1, "%.2f,%.2f,%.2f,%d\r\n", IMU_Yaw_Abs_Get(), Motor_A.PID_s.realPoint_Now, Motor_B.PID_s.realPoint_Now, Y8U_GetADC_Sum());
}

void Mode_4_Exit(void)
{
    
}
