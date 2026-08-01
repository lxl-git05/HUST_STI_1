// ==================== Mode_5 球速PID调参 ====================
#include "Mode_5.h"
#include "AllHeader.h"

void Mode_5_Setup(void)
{
	Oran_Speed_PID_Init() ;
	Stepper_PWM_Stop(&Stepper1);
}

void Mode_5_Loop(void)
{
	OLED_Printf(0, 0, OLED_6X8, "===Speed PID===") ;
	OLED_Printf(0,10,OLED_6X8,"Kp:%.3f Ki:%.3f", PID_Oran_Speed.Kp, PID_Oran_Speed.Ki) ;
	OLED_Printf(0,20,OLED_6X8,"Kd:%.2f c:%d r:%d", PID_Oran_Speed.Kd, Oran_Speed_Calc, Oran_Speed) ;
	OLED_Printf(0,30,OLED_6X8,"set:%.1f goal:%.0f", PID_Oran_Speed.setPoint, PID_Oran_Speed.goalPoint) ;
	OLED_Printf(0,40,OLED_6X8,"P:%.1f I:%.1f D:%.1f",
		PID_Oran_Speed.pout, PID_Oran_Speed.iout, PID_Oran_Speed.dout) ;
	OLED_Printf(0,50,OLED_6X8,"Alp:%.2f Df:%.2f", Oran_Speed_Filt_Alpha, PID_Oran_Speed.d_filter) ;
	// Serial ABC在线调参
	if (Serial_GetNewPackageFlag_ABC(&Serial1))
	{
			Serial_SetFloatData(&Serial1, "Kp",  "Kp=%f",  &PID_Oran_Speed.Kp);
			Serial_SetFloatData(&Serial1, "Ki",  "Ki=%f",  &PID_Oran_Speed.Ki);
			Serial_SetFloatData(&Serial1, "Kd",  "Kd=%f",  &PID_Oran_Speed.Kd);
			Serial_SetFloatData(&Serial1, "Goal","Goal=%f",&PID_Oran_Speed.goalPoint);
			Serial_SetFloatData(&Serial1, "Alp", "Alp=%f",&Oran_Speed_Filt_Alpha);
			Serial_SetFloatData(&Serial1, "Df",  "Df=%f", &PID_Oran_Speed.d_filter);
	}
}

void Mode_5_Tick(void)
{
	// 20ms CSV输出: Goal,CalcSpd(自解),CamSpd(Serial2),Set,Pout,Iout,Dout
	Serial_printf(&Serial1 , "%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",
		PID_Oran_Speed.goalPoint, PID_Oran_Speed.realPoint_Now,
		PID_Oran_Speed.setPoint, PID_Oran_Speed.pout, PID_Oran_Speed.iout, PID_Oran_Speed.dout);
}

void Mode_5_10ms_Tick(void)
{
	Oran_Speed_PID_Update() ;
}

void Mode_5_Exit(void)
{
	Stepper_PWM_Stop(&Stepper1);
}
