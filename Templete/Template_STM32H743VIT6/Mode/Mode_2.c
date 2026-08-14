#include "Mode_2.h"
#include "AllHeader.h"

void Mode_2_Setup(void)
{
    OLED_Clear();
}


// ============== Mode_2 主循环 ==============
void Mode_2_Loop(void)
{
		OLED_Printf(0,0,OLED_6X8,"===Mode_2===") ;
    // 按键1
    if (Key_Check(KEY_1, KEY_SINGLE)) 
		{
			
    }
}

void Mode_2_Tick(void)
{
	
}

void Mode_2_Exit(void)
{
	
}
