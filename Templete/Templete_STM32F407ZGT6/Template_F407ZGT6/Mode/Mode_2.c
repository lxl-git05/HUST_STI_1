#include "Mode_2.h"
#include "AllHeader.h"

void Mode_2_Setup(void)
{
    OLED_Clear();
		MPU6050_Angle_Init() ;
}

void Mode_2_Loop(void)
{
	OLED_Printf(0, 0, OLED_6X8, "=====Mode_2=====") ;
	OLED_Printf(0,20,OLED_6X8,"%f",MPU_Real.yaw) ;
}

void Mode_2_Tick(void)
{
  MPU6050_Angle_Update_Tick() ;
	Serial_printf(&Serial1, "%.2f,%.2f,%.2f\n",
                      MPU_Real.yaw,
                      MPU_Real.roll,
                      MPU_Real.pitch);
}

void Mode_2_Exit(void)
{
   
}
