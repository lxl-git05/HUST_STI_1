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
    // 恢复角度环: 速度/位置/直行调参任务会临时关闭,退出 Mode_3 必须还原
    Motor_A.Angle_Ring_Enable = 1;
    Motor_B.Angle_Ring_Enable = 1;
    OLED_Clear();
}
