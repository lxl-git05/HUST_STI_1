#include "Con_Mode_5.h"

void Con_Mode_5_Setup(void)
{
    
}

void Con_Mode_5_Loop(void)
{
	OLED_Printf(0, 0, OLED_6X8, "=====Con_Mode_5=====") ;
	
}

void Con_Mode_5_Tick(void)
{
	
}

void Con_Mode_5_Exit(void)
{
    OLED_Clear();
}
