#include "Mode_4.h"
#include "AllHeader.h"

void Mode_4_Setup(void)
{
    OLED_Clear();
}

void Mode_4_Loop(void)
{
	OLED_Printf(0,0,OLED_6X8,"===Mode_4===") ;
}

void Mode_4_Tick(void)
{
}

void Mode_4_Exit(void)
{
}
