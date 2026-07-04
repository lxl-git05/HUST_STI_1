#include "Mode_3.h"
#include "AllHeader.h"

void Mode_3_Setup(void)
{
   OLED_Clear() ;
}

void Mode_3_Loop(void)
{
  OLED_Printf(0, 0, OLED_6X8, "=====Mode_3=====") ;
	
}

void Mode_3_Exit(void)
{
    OLED_Clear() ;
}

void Mode_3_Tick(void)
{
	
}
