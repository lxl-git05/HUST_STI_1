#include "AllHeader.h"

void Mode_2_Setup(void)
{
    
}

void Mode_2_Loop(void)
{
    OLED_Printf(0, 0, OLED_6X8, "=====Mode_2=====") ;
}

void Mode_2_Tick(void)
{
   Serial_printf(&Serial1 , "%.2f,%.2f\n",IMU_Get_Ax() , IMU_Get_Ay()) ;
}

void Mode_2_Exit(void)
{
		
}
