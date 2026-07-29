// ==================== Mode_3 Menu_Task模式====================
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

}

void Mode_3_Exit(void)
{
    Con_Task_Clear();
    Motor_Stop(&Motor_A);
    Motor_Stop(&Motor_B);
    OLED_Clear();
}
