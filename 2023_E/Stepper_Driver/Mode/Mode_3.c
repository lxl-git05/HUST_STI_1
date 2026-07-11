#include "Mode_3.h"
#include "AllHeader.h"
#include "Emm_V5.h"
#include "Stepper.h"

void Mode_3_Setup(void)
{
   OLED_Clear() ;
}

void Mode_3_Loop(void)
{
	OLED_Printf(0, 0 , OLED_6X8, "Mode_3->us:%.2f fc:%.2f", time_us, time_Func_us);
	OLED_Printf(0, 20, OLED_6X8, "M1 vel:%d M2 vel:%d", Stepper1.Speed_Now,Stepper2.Speed_Now);
	OLED_Printf(0, 30, OLED_6X8, "M1 pos:%.2f", Stepper1.Pos_Now);
	OLED_Printf(0, 40, OLED_6X8, "M2 pos:%.2f", Stepper2.Pos_Now);
	
	// 电机参数串口配置
	// Serial1接收
	if (Serial_GetNewPackageFlag_ABC(&Serial1)) 
	{
		// 处理Serial1数据
		Serial_SetFloatData(&Serial1, "Kp", "Kp=%f", &Stepper1.PID_Angle.Kp);
    Serial_SetFloatData(&Serial1, "Ki", "Ki=%f", &Stepper1.PID_Angle.Ki);
    Serial_SetFloatData(&Serial1, "Kd", "Kd=%f", &Stepper1.PID_Angle.Kd);
	}
	if (Key_Check(KEY_2 , KEY_SINGLE))
	{
		Emm_V5_Pos_Control(&huart6 , 1, 0 , 200 , 0 , 3600 , 2 , 0) ;
		Emm_V5_Pos_Control(&huart3 , 1, 0 , 200 , 0 , 3600 , 2 , 0) ;
	}
}

void Mode_3_Exit(void)
{
    OLED_Clear() ;
}

void Mode_3_Tick(void)
{
	Stepper_PID_Tick(20) ;
	Serial_printf(&Serial1, "%.2f,%.2f,%.2f\n",Stepper1.PID_Angle.goalPoint ,Stepper1.PID_Angle.realPoint_Now ,Stepper1.PID_Angle.setPoint );
}
