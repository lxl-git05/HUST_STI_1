#include "Mode_2.h"
#include "AllHeader.h"

void Mode_2_Setup(void)
{
    OLED_Clear();
}

void Mode_2_Loop(void)
{
    OLED_Printf(0, 0, OLED_6X8, "=====Mode_2=====");
    OLED_Printf(0, 10, OLED_6X8, "PWM:%dHz", MyPWM_GetFre(&MyPWM_Motor_A_IN1));

    if (Key_Check(KEY_1, KEY_SINGLE)) 
		{
        Flash_Mode_Set(Flash_Mode_Slow);
        MyPWM_SetCompare(&MyPWM_Motor_A_IN1, 500);  // 占空比50%
    }
    if (Key_Check(KEY_1, KEY_DOUBLE))
		{
        Flash_Mode_Set(Flash_Mode_Fast);
        MyPWM_SetCompare(&MyPWM_Motor_A_IN1, 0);    // 关闭
    }
}

void Mode_2_Tick(void)
{
	
}

void Mode_2_Exit(void)
{
    MyPWM_SetCompare(&MyPWM_Motor_A_IN1, 0);
}
