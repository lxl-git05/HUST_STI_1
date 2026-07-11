#include "Mode_4.h"
#include "AllHeader.h"
#include "Emm_V5.h"
#include "Stepper.h"

bool Open = 1 ;

void Mode_4_Setup(void)
{
    OLED_Clear();
}

void Mode_4_Loop(void)
{
	OLED_Printf(0, 0 , OLED_6X8, "Mode_4->us:%.2f fc:%.2f", time_us, time_Func_us);
	if (Serial_GetNewPackageFlag_ABC(&Serial1)) 
	{
		// 处理Serial1数据
		Serial_SetFloatData(&Serial1, "Kp", "Kp=%f", &Stepper2.PID_Angle.Kp);
    Serial_SetFloatData(&Serial1, "Ki", "Ki=%f", &Stepper2.PID_Angle.Ki);
    Serial_SetFloatData(&Serial1, "Kd", "Kd=%f", &Stepper2.PID_Angle.Kd);
	}
	if (Key_Check(KEY_1 , KEY_SINGLE))
	{
		Open = 0 ;
//		Stepper_Angle_Abs_Set(&Stepper2 , 10 , 0 , 20) ;
		Emm_V5_En_Control(&huart6, 1 ,1 , 0) ;
		
	}
	if (Key_Check(KEY_2 , KEY_SINGLE))
	{
		Open = 0 ;
		Emm_V5_Pos_Control(&huart6 , 1, 0 , 200 , 0 , 3600 , 2 , 0) ;
	}
}

void Mode_4_Tick(void)
{
	if (Open == 0)
		return ;
	Stepper_PID_Tick(20) ;
	Serial_printf(&Serial1, "%.2f,%.2f,%.2f\n",Stepper2.PID_Angle.goalPoint ,Stepper2.PID_Angle.realPoint_Now ,Stepper2.PID_Angle.setPoint );
}

void Mode_4_Exit(void)
{
}
