#include "Mode_2.h"
#include "AllHeader.h"

void Mode_2_Setup(void)
{
	OLED_Clear() ;
}

void Mode_2_Loop(void)
{
		OLED_Printf(0, 0, OLED_6X8, "=====Mode_2=====") ;
    if (Key_Check(KEY_2 , KEY_SINGLE))
		{
			Stepper_PWM_Speed_Set(&Stepper1 , -20) ;
			Stepper_PWM_Speed_Set(&Stepper2 , 20) ;
		}
		if (Key_Check(KEY_3 , KEY_SINGLE))
		{
			Stepper_PWM_Stop(&Stepper1) ;
			Stepper_PWM_Stop(&Stepper2) ;
		}
		OLED_Printf(0,20,OLED_6X8 , "%.2f",Stepper1.Pos_Now) ;
}

void Mode_2_Tick(void)
{
	
}

void Mode_2_Exit(void)
{
    
}
