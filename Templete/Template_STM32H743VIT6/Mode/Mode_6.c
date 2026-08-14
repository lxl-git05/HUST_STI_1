#include "Mode_6.h"
#include "AllHeader.h"

void Mode_6_Setup(void)
{
    OLED_Clear();
}

void Mode_6_Loop(void)
{
	OLED_Printf(0,0,OLED_6X8,"===Mode_6===") ;
}

void Mode_6_Tick(void)
{
}

void Mode_6_Exit(void)
{
}
