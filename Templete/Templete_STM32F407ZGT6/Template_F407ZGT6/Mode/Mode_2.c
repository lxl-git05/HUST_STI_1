#include "Mode_2.h"
#include "AllHeader.h"

void Mode_2_Setup(void)
{
	
}

void Mode_2_Loop(void)
{
		OLED_Printf(0, 0, OLED_6X8, "=====Mode_2=====") ;
    if (Key_Check(KEY_2 , KEY_SINGLE))
		{
			Serial_printf(&Serial1 , "Hello\r\n") ;
			Serial_printf(&Serial2 , "Hello\r\n") ;
		}
}

void Mode_2_Tick(void)
{
}

void Mode_2_Exit(void)
{
    
}
