#include "Mode_2.h"
#include "AllHeader.h"

void Mode_2_Setup(void)
{
    OLED_Clear();
}

void Mode_2_Loop(void)
{
    OLED_Printf(0, 0, OLED_6X8, "==== Mode 2 ====");
    // 测试电磁铁
    if (Key_Check(KEY_1 , KEY_SINGLE))
    {
        Elec_ON();
    }
    if (Key_Check(KEY_2 , KEY_SINGLE))
    {
        Elec_OFF();
    }

}

void Mode_2_Tick(void)
{
    
}

void Mode_2_Exit(void)
{
    OLED_Clear();
}
