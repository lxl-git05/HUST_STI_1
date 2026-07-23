#include "AllHeader.h"

// ========================== ICM42688 互补滤波角度测试 ==========================
// Setup：ICM42688硬件初始化（跳过标定）
// Tick： ICM42688_Angle_Update_Tick() → 仅更新数据
// Loop： OLED显示 + 串口每100ms输出
// =================================================================

static uint32_t last_print_tick = 0;

void Mode_2_Setup(void)
{
    Serial_printf(&Serial1, "Mode2 ICM42688 Angle\r\n");
    OLED_Clear();
    OLED_Printf(0, 0, OLED_6X8, "ICM42688 Angle");

    ICM42688_Angle_Init();

    OLED_Printf(0, 20, OLED_6X8, "R P Y");
    Serial_printf(&Serial1, "Roll,Pitch,Yaw\r\n");
}

void Mode_2_Loop(void)
{
    OLED_Printf(0, 12, OLED_6X8, "R:%.1f", ICM_Real.roll);
    OLED_Printf(0, 24, OLED_6X8, "P:%.1f", ICM_Real.pitch);
    OLED_Printf(0, 36, OLED_6X8, "Y:%.1f", ICM_Real.yaw);

    if (HAL_GetTick() - last_print_tick >= 100)
    {
        last_print_tick = HAL_GetTick();
        
    }
}

void Mode_2_Tick(void)
{
	Timer_Counter_Begin() ;
    ICM42688_Angle_Update_Tick();
	Serial_printf(&Serial1, "%.2f,%.2f,%.2f\r\n",
                      ICM_Real.roll, ICM_Real.pitch, ICM_Real.yaw);
	Timer_Counter_End() ;
}

void Mode_2_Exit(void)
{

}
