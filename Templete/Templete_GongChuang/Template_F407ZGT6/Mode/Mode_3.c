// ==================== Mode_3-PWM驱动步进电机 ====================
#include "Mode_3.h"
#include "AllHeader.h"

void Mode_3_Setup(void)
{
	Stepper_Init(&Stepper1) ;
	Stepper_Enable(&Stepper1) ;
}

void Mode_3_Loop(void)
{
	if (Key_Check(KEY_1, KEY_LONG))
	{
		Stepper_StopImmediate(&Stepper1) ;
	}
	else if (Key_Check(KEY_1, KEY_DOUBLE))
	{
		Stepper_RunImmediate(&Stepper1, -1000) ;
	}
	else if (Key_Check(KEY_1, KEY_SINGLE))
	{
		Stepper_RunImmediate(&Stepper1, 1000) ;
	}

	if (Key_Check(KEY_2, KEY_LONG))
	{
		Stepper_StopRamp(&Stepper1, 6000) ;
	}
	else if (Key_Check(KEY_2, KEY_DOUBLE))
	{
		Stepper_RunRamp(&Stepper1, -3000, 6000, 6000) ;
	}
	else if (Key_Check(KEY_2, KEY_SINGLE))
	{
		Stepper_RunRamp(&Stepper1, 3000, 6000, 6000) ;
	}

	if (Key_Check(KEY_3, KEY_LONG))
	{
		if (!Stepper_IsBusy(&Stepper1))
		{
			Stepper_SetPosition(&Stepper1, 0) ;
		}
	}
	else if (Key_Check(KEY_3, KEY_SINGLE ))
	{
		Stepper_MoveByTrapezoid(&Stepper1, -3200, 3000, 6000, 6000) ;
	}
	else if (Key_Check(KEY_3, KEY_DOUBLE ))
	{
		Stepper_MoveByConstant(&Stepper1, 800, 800) ;
	}

	OLED_Printf(0,  0, OLED_6X8, "===Mode_3 PWM===") ;
	OLED_Printf(0, 15, OLED_6X8, "S:%d V:%ld", (int)Stepper_GetState(&Stepper1),
				(long)Stepper_GetSpeed(&Stepper1)) ;
	OLED_Printf(0, 30, OLED_6X8, "P:%ld T:%ld", (long)Stepper_GetPosition(&Stepper1),
				(long)Stepper1.target_position) ;
	OLED_Printf(0, 45, OLED_6X8, "R:%lu E:%d", (unsigned long)Stepper1.move_remaining_pulses,
				(int)Stepper1.last_status) ;
}

void Mode_3_Tick(void)
{

}

void Mode_3_1ms_Tick(void)
{
	Stepper_Tick1ms(&Stepper1) ;
}

void Mode_3_Exit(void)
{
	Stepper_StopImmediate(&Stepper1) ;
	Stepper_Disable(&Stepper1) ;
}
