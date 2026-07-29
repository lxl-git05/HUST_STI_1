// ========================== Con_Mode_2 ==========================
// 小车置于A点，按键启动后沿黑线顺时针行驶一圈并停到A点，
// 计时停止并显示行驶总时间，要求行驶总时间≤20s，停车偏差≤2cm。
#include "Con_Mode_2.h"

static uint8_t  race_state = 0;   // 0=待命, 1=比赛中, 2=结束
static uint32_t race_start = 0;
static float    race_time  = 0;

void Con_Mode_2_Setup(void)
{
	Y8U_SetSpeed(0);
	race_state = 0;
}

void Con_Mode_2_Loop(void)
{
    OLED_Printf(0, 0, OLED_6X8, "Con_Mode_2");
		// 调试PID
		if (Serial_GetNewPackageFlag_ABC(&Serial1))
		{
				Serial_SetFloatData(&Serial1, "Kp", "Kp=%f", &Y8U_PID.Kp);
				Serial_SetFloatData(&Serial1, "Ki", "Ki=%f", &Y8U_PID.Ki);
				Serial_SetFloatData(&Serial1, "Kd", "Kd=%f", &Y8U_PID.Kd);
				Serial_SetFloatData(&Serial1, "Goal", "Goal=%f", &Y8U_PID.goalPoint);
		}

		// OLED展示
		if (race_state == 0)
		{
				OLED_Printf(0, 10, OLED_8X16, "KEY1:Go");
				OLED_Printf(0, 30, OLED_8X16, "Time:0");
				if (Key_Check(KEY_1, KEY_SINGLE))
				{
						race_state = 1;
						race_start = HAL_GetTick();
						Y8U_SetSpeed(150.0f);
				}
		}
		else if (race_state == 1)
		{
				OLED_Printf(0, 10, OLED_6X8, "Racing...");
				OLED_Printf(0, 30, OLED_8X16, "Time:%.1f", (HAL_GetTick() - race_start) / 1000.0f);
		}
		else
		{  // race_state == 2
				OLED_Printf(0, 10, OLED_6X8, "Finished!");
				OLED_Printf(0, 30, OLED_8X16, "Time:%.1f", race_time);
		}
}

void Con_Mode_2_Tick(void)
{
		if (race_state == 1)
		{
				// 入弯减速
				if (IMU_Yaw_Abs_Get() > 300.0f && Y8U_GetSpeed() > 90.0f)
				{
						Y8U_SetSpeed(90.0f);
				}
				Y8U_PID_Update();

				// 横线终点检测（转满330° + 滑动窗口异常检测）
				if (IMU_Yaw_Abs_Get() > 330.0f && Y8U_CheckFinishLine())
				{
						race_state = 2;
						race_time  = (HAL_GetTick() - race_start) / 1000.0f;
						Y8U_SetSpeed(0);
						Motor_SetSpeed(&Motor_A, 0);
						Motor_SetSpeed(&Motor_B, 0);
				}
		}

		Serial_printf(&Serial1, "%.2f,%.2f,%.2f,%.2f\r\n", Y8U_PID.goalPoint, Y8U_PID.realPoint_Now, Y8U_PID.setPoint, IMU_Yaw_Abs_Get());
}

void Con_Mode_2_Exit(void)
{
    race_state = 0;
}
