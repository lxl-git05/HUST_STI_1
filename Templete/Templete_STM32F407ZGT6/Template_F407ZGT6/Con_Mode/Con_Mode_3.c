#include "Con_Mode_3.h"

void Con_Mode_3_Setup(void)
{
    Oran_PID_Init() ;
    PID_Param_Reset(&PID_Oran) ;  // 清空误差/积分历史,防止上一模式残留
    PID_Oran.goalPoint = 0.0f ;   // goal恒为0, 偏移通过RealOff模拟
}

void Con_Mode_3_Loop(void)
{
	OLED_Printf(0, 0, OLED_6X8, "=====Con_Mode_3=====") ;
	Serial_SetFloatData(&Serial1, "Kp", "Kp=%f", &PID_Oran.Kp);
	Serial_SetFloatData(&Serial1, "Ki", "Ki=%f", &PID_Oran.Ki);
	Serial_SetFloatData(&Serial1, "Kd", "Kd=%f", &PID_Oran.Kd);
	Serial_SetFloatData(&Serial1, "Goal", "Goal=%f", &Oran_Real_Offset);
	Serial_SetFloatData(&Serial1, "KpHi", "KpHi=%f", &Oran_KpHi);
	// OLED展示
	OLED_Printf(0,10,OLED_6X8,"%.1f,%.1f,%.1f",PID_Oran.Kp , PID_Oran.Ki , PID_Oran.Kd) ;
	OLED_Printf(0,20,OLED_6X8,"%.1f,%.1f,%.1f", PID_Oran.goalPoint, PID_Oran.realPoint_Now, PID_Oran.setPoint);
}

void Con_Mode_3_Tick(void)
{
	Oran_Update() ;              // 刷新位置数据,确保PID用最新值
	// 位置PID → 位置
	Oran_PID_Update() ;
	// RGB_R指示灯: |real|>30 亮红灯
	{
		float r = PID_Oran.realPoint_Now ;
		RGB_Set_Color((r > 30.0f || r < -30.0f) ? 1 : 0, 0, 0) ;
	}
	// 位置PID CSV: Goal, RawPos, Pout, Iout, Dout, Output
	Serial_printf(&Serial1 , "%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%d\n",
		PID_Oran.goalPoint, PID_Oran.realPoint_Now,
		PID_Oran.pout, PID_Oran.iout, PID_Oran.dout, PID_Oran.setPoint,Oran_Speed);
}

void Con_Mode_3_Exit(void)
{
	Stepper_PWM_Stop(&Stepper1);
}
