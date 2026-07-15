#include "Con_Mode_3.h"

void Con_Mode_3_Setup(void)
{
    OLED_Clear();
}

void Con_Mode_3_Loop(void)
{
	OLED_Printf(0, 0, OLED_6X8, "=====Con_Mode_3=====") ;
}

void Con_Mode_3_Tick(void)
{
}

void Con_Mode_3_Exit(void)
{
    OLED_Clear();
}
