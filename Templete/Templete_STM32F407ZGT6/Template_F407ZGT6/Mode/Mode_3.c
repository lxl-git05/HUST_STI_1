// ========================== 云台PID调参模式 ==========================
#include "Mode_3.h"
#include "AllHeader.h"

bool IsS1 = 1 ;

void Mode_3_Setup(void)
{
   OLED_Clear() ;
}

void Mode_3_Loop(void)
{
  OLED_Printf(0, 0, OLED_6X8, "=====Mode_Stepper=====") ;
	if (Key_Check(KEY_1 , KEY_SINGLE))	// 按下按键1就切换调试的Stepper
	{
		IsS1 = !IsS1 ;
	}
	if (Key_Check(KEY_2 , KEY_SINGLE))	// 按下按键2就急停
	{
		Stepper_PWM_Stop(&Stepper1) ;
		Stepper_PWM_Stop(&Stepper2) ;
	}
	
	if (Serial_GetNewPackageFlag_ABC(&Serial1)) 
	{
		// 处理Serial1数据
		if (IsS1)
		{
			Serial_SetFloatData(&Serial1, "Kp", "Kp=%f", &Stepper1.PID_Angle.Kp);
			Serial_SetFloatData(&Serial1, "Ki", "Ki=%f", &Stepper1.PID_Angle.Ki);
			Serial_SetFloatData(&Serial1, "Kd", "Kd=%f", &Stepper1.PID_Angle.Kd);
			Serial_SetFloatData(&Serial1, "Goal", "Goal=%f", &x_real);
		}
		else
		{
			Serial_SetFloatData(&Serial1, "Kp", "Kp=%f", &Stepper2.PID_Angle.Kp);
			Serial_SetFloatData(&Serial1, "Ki", "Ki=%f", &Stepper2.PID_Angle.Ki);
			Serial_SetFloatData(&Serial1, "Kd", "Kd=%f", &Stepper2.PID_Angle.Kd);
			Serial_SetFloatData(&Serial1, "Goal", "Goal=%f", &y_real);
		}
	}
	OLED_Printf(0, 20, OLED_6X8 , "%s",IsS1 ? "Stepper 1" : "Stepper 2") ;
	OLED_Printf(0, 30, OLED_6X8 , "Pos1=%.2f Pos2=%.2f",Stepper1.Pos_Now,Stepper2.Pos_Now) ;
}

// 云台PID
void Mode_3_Tick(void)
{
	Stepper_PID_Tick(20) ;
	if (IsS1)
	{
		Serial_printf(&Serial1, "%.2f,%.2f,%.2f\n",Stepper1.PID_Angle.goalPoint ,Stepper1.PID_Angle.realPoint_Now ,Stepper1.PID_Angle.setPoint );
	}
	else
	{
		Serial_printf(&Serial1, "%.2f,%.2f,%.2f\n",Stepper2.PID_Angle.goalPoint ,Stepper2.PID_Angle.realPoint_Now ,Stepper2.PID_Angle.setPoint );		
	}
}

void Mode_3_Exit(void)
{
    OLED_Clear() ;
}
