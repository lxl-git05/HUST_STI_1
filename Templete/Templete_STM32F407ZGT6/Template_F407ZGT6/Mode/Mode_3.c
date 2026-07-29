// ==================== Mode_3 统一调参菜单 ====================
// 基于 Menu_Param + Con_Task: KEY_1单击下一项, KEY_1长按入队/再次长按Skip
#include "Mode_3.h"
#include "AllHeader.h"

void Mode_3_Setup(void)
{
    OLED_Clear();
    Menu_Tune_Init();
}

void Mode_3_Loop(void)
{
    Menu_Tune_Loop();
}

void Mode_3_Tick(void)
{
    // Con_Task_Tick() 已在 Timer_20ms_Callback 中全局调用
}

void Mode_3_Exit(void)
{
    Con_Task_Clear();
    Motor_Stop(&Motor_A);
    Motor_Stop(&Motor_B);
    OLED_Clear();
}
