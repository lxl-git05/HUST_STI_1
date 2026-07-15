#include "Con_Mode_6.h"

void Con_Mode_6_Setup(void)
{
    OLED_Clear();
}

void Con_Mode_6_Loop(void)
{
	OLED_Printf(0, 0, OLED_6X8, "=====Con_Mode_6=====") ;
}

void Con_Mode_6_Tick(void)
{
}

void Con_Mode_6_Exit(void)
{
    OLED_Clear();
}
