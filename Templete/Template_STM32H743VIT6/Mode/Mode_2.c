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
        Serial_printf(&Serial2 , "@T_Junction:120$#") ;
				Serial_printf(&Serial1 , "@T_Junction:120$#") ;
    }

    // OLED显示HEX接收的数据
    OLED_Printf(0, 0, OLED_6X8, "R2:%d,%d,%d,%d", 
		Serial2.HEX_Data.data[0] , Serial2.HEX_Data.data[1] , Serial2.HEX_Data.data[2] , Serial2.HEX_Data.data[3]);
		OLED_Printf(0, 20, OLED_6X8, "R1:%d,%d,%d,%d", 
		Serial1.HEX_Data.data[0] , Serial1.HEX_Data.data[1] , Serial1.HEX_Data.data[2] , Serial1.HEX_Data.data[3]);
}

void Mode_2_Tick(void)
{
	
}

void Mode_2_Exit(void)
{
	
}
