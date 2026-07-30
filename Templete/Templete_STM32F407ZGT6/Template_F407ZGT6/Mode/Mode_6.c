// ==================== Mode_6 串级PID(位置环→速度环) ====================
#include "Mode_6.h"
#include "AllHeader.h"

void Mode_6_Setup(void)
{
	Oran_Cascade_Init() ;
	Stepper_PWM_Stop(&Stepper1);
}

void Mode_6_Loop(void)
{
	OLED_Printf(0, 0, OLED_6X8, "===Cascade===") ;
	OLED_Printf(0,10,OLED_6X8,"Pos:%d Spd:%d", Oran_real, Oran_Speed) ;
	OLED_Printf(0,20,OLED_6X8,"Pout:%.1f Sout:%.1f", PID_Oran.setPoint, PID_Oran_Speed.setPoint) ;
	// 在线调外环(位置环)Kp/Kd; 内环(速度环)已在Mode_5调好, 一般不动
	if (Serial_GetNewPackageFlag_ABC(&Serial1))
	{
			Serial_SetFloatData(&Serial1, "Kp", "Kp=%f", &PID_Oran.Kp);
			Serial_SetFloatData(&Serial1, "Kd", "Kd=%f", &PID_Oran.Kd);
//			Serial_SetFloatData(&Serial1, "Kp", "Kp=%f", &PID_Oran_Speed.Kp);
//			Serial_SetFloatData(&Serial1, "Kd", "Kd=%f", &PID_Oran_Speed.Kd);
			Serial_SetFloatData(&Serial1, "Goal", "Goal=%f", &PID_Oran.goalPoint);
	}
	OLED_Printf(0,30,OLED_6X8,"PD:%.2f,%.2f",PID_Oran.Kp , PID_Oran.Kd) ;
}

void Mode_6_Tick(void)
{
	Serial_printf(&Serial1 , "%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",
		PID_Oran.goalPoint, PID_Oran.realPoint_Now, PID_Oran.setPoint,
		PID_Oran_Speed.goalPoint , PID_Oran_Speed.realPoint_Now, PID_Oran_Speed.setPoint);
}

void Mode_6_10ms_Tick(void)
{
	Oran_Cascade_Update() ;
}

void Mode_6_Exit(void)
{
	Stepper_PWM_Stop(&Stepper1);
}
