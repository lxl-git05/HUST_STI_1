#include "Mode_2.h"
#include "AllHeader.h"

void Mode_2_Setup(void)
{
    OLED_Clear();
}

// ============== Mode_2 主循环 ==============
void Mode_2_Loop(void)
{
    // 按键1：Serial1发送
    if (Key_Check(KEY_1, KEY_SINGLE)) 
		{
        Serial_printf(&Serial2 , "T_Junction:120\n") ;
				Serial_printf(&Serial1 , "T_Junction:120\n") ;
    }

    // OLED显示HEX接收的数据
    OLED_Printf(0, 0, OLED_6X8, "R:%d,%d,%d,%d", 
		Serial2.HEX_Data.data[0] , Serial2.HEX_Data.data[1] , Serial2.HEX_Data.data[2] , Serial2.HEX_Data.data[3]);
}

void Mode_2_Tick(void)
{
	
}

void Mode_2_Exit(void)
{
	
}
