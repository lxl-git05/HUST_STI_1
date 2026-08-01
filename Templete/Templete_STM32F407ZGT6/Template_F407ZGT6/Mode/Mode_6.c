// ==================== Mode_6 串级PID调参(位置环→速度环) ====================
#include "Mode_6.h"
#include "AllHeader.h"

void Mode_6_Setup(void)
{
	Oran_Cascade_Init() ;
	Stepper_PWM_Stop(&Stepper1);
}

void Mode_6_Loop(void)
{
	OLED_Printf(0, 0, OLED_6X8, "===Casc PID=== r:%d", Oran_real) ;
	OLED_Printf(0,10,OLED_6X8,"oKp:%.2f Ki:%.3f Kd:%.1f", PID_Oran.Kp, PID_Oran.Ki, PID_Oran.Kd) ;
	OLED_Printf(0,20,OLED_6X8,"sKp:%.3f Kd:%.2f c:%d", PID_Oran_Speed.Kp, PID_Oran_Speed.Kd, Oran_Speed_Calc) ;
	OLED_Printf(0,30,OLED_6X8,"Star:%.1f Set:%.1f", PID_Oran.setPoint, PID_Oran_Speed.setPoint) ;
	OLED_Printf(0,40,OLED_6X8,"P:%.1f I:%.1f D:%.1f",
		PID_Oran_Speed.pout, PID_Oran_Speed.iout, PID_Oran_Speed.dout) ;
	OLED_Printf(0,50,OLED_6X8,"Alp:%.2f Df:%.2f", Oran_Speed_Filt_Alpha, PID_Oran_Speed.d_filter) ;
	// Serial ABC在线调参
	if (Serial_GetNewPackageFlag_ABC(&Serial1))
	{
			// 命令与Mode_5同名, 但调节的是外环(位置环)
			Serial_SetFloatData(&Serial1, "Kp",  "Kp=%f",  &PID_Oran.Kp);
			Serial_SetFloatData(&Serial1, "Ki",  "Ki=%f",  &PID_Oran.Ki);
			Serial_SetFloatData(&Serial1, "Kd",  "Kd=%f",  &PID_Oran.Kd);
			Serial_SetFloatData(&Serial1, "Goal","Goal=%f",&PID_Oran.goalPoint);
			Serial_SetFloatData(&Serial1, "Alp", "Alp=%f",&Oran_Speed_Filt_Alpha);
			Serial_SetFloatData(&Serial1, "Df",  "Df=%f", &PID_Oran_Speed.d_filter);
	}
}

void Mode_6_Tick(void)
{
	// 20ms CSV: Pos, SpeedTar(外环输出), CalcSpd, StepperSet, sPout, sIout, sDout
	Serial_printf(&Serial1 , "%d,%.1f,%d,%.1f,%.1f,%.1f,%.1f\n",
		Oran_real, PID_Oran.setPoint, Oran_Speed_Calc,
		PID_Oran_Speed.setPoint, PID_Oran_Speed.pout, PID_Oran_Speed.iout, PID_Oran_Speed.dout);
}

void Mode_6_10ms_Tick(void)
{
	Oran_Cascade_Update() ;
}

void Mode_6_Exit(void)
{
	Stepper_PWM_Stop(&Stepper1);
}
