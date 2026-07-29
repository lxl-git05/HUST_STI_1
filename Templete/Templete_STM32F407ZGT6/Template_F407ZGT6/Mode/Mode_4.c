#include "Mode_4.h"
#include "AllHeader.h"

void Mode_4_Setup(void)
{
    
}

float Speed = 0 ;

void Mode_4_Loop(void)
{
	OLED_Printf(0, 0, OLED_6X8, "=====Mode_4=====") ;
	OLED_Printf(0,20,OLED_6X8,"cmd:%d",Serial_GetHexData(&Serial2,0)) ;
	OLED_Printf(0,30,OLED_6X8,"len:%d",Serial_GetHexLen(&Serial2)) ;
	OLED_Printf(0,40,OLED_6X8,"data1:%d",Serial_GetHexData(&Serial2,1)) ;
	
	// 步进电机调试
	if (Key_Check(KEY_1 , KEY_SINGLE))
	{
		Stepper_PWM_Speed_Set(&Stepper1 , Speed , 0) ;	
	}
}

void Mode_4_Tick(void)
{
    
}

void Mode_4_Exit(void)
{
    
}
