#include "Mode_4.h"
#include "AllHeader.h"
// 小车置于A点，按键启动后沿黑线顺时针行驶一圈并停到A点，
// 计时停止并显示行驶总时间，要求行驶总时间≤20s，停车偏差≤2cm。
void Mode_4_Setup(void)
{
		Y8U_Init();          // 启动 USART3 DMA 接收 + 发送 $0,1,0#
    Y8U_PID_Init();      // PID 初始化
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
    OLED_Printf(0, 54, OLED_6X8, "KEY1:Go KEY2:Stop");
    if (Key_Check(KEY_1, KEY_SINGLE)) 
		{
        Y8U_SetSpeed(120.0f);
    }
    // OLED展示
		OLED_Printf(0, 16, OLED_6X8, "yaw:%.2f",IMU_Yaw_Abs_Get());
}

void Mode_4_Tick(void)
{
		Y8U_PID_Update();

		// Serial1 输出: goal=0, real=偏移, set=PID输出
		Serial_printf(&Serial1, "%.2f,%.2f,%.2f,%.2f\r\n",Y8U_PID.goalPoint, Y8U_PID.realPoint_Now, Y8U_PID.setPoint,IMU_Yaw_Abs_Get());
}

void Mode_4_Exit(void)
{
    
}
