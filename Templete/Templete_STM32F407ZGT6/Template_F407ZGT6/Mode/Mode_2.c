// ==================== Mode_2 实验模式 ====================
// 子模式:
//   sub=0: 位置PID（Oran_real → Stepper_PWM_Speed_Set）— 默认
//   sub=1: 角度跟踪测试（串口设目标 → 1ms角PID → 速度）— 串口"Mode=1"切换
#include "AllHeader.h"

static uint8_t mode2_sub = 0 ;  // 0=位置PID, 1=角度跟踪测试

void Mode_2_Setup(void)
{
	Oran_PID_Init() ;
	Oran_Angle_Test_Init() ;  // 预初始化角度跟踪PID默认值
	mode2_sub = 0 ;
}

void Mode_2_Loop(void)
{
	// ===== 统一串口命令处理 =====
	if (Serial_GetNewPackageFlag_ABC(&Serial1))
	{
		// 子模式切换: "Mode=1" 切换到角度跟踪, "Mode=0" 回到位置PID
		{
			float mode = (float)mode2_sub ;
			Serial_SetFloatData(&Serial1, "Mode", "Mode=%f", &mode);
			mode2_sub = (uint8_t)mode ;
		}

		if (mode2_sub == 1)  // 角度跟踪调参
		{
			Serial_SetFloatData(&Serial1, "Kp", "Kp=%f", &Stepper1.PID_Angle.Kp);
			Serial_SetFloatData(&Serial1, "Ki", "Ki=%f", &Stepper1.PID_Angle.Ki);
			Serial_SetFloatData(&Serial1, "Kd", "Kd=%f", &Stepper1.PID_Angle.Kd);
			Serial_SetFloatData(&Serial1, "Goal", "Goal=%f", &Oran_Angle_Test_Target);
		}
		else  // 位置PID调参
		{
			Serial_SetFloatData(&Serial1, "Kp", "Kp=%f", &PID_Oran.Kp);
			Serial_SetFloatData(&Serial1, "Ki", "Ki=%f", &PID_Oran.Ki);
			Serial_SetFloatData(&Serial1, "Kd", "Kd=%f", &PID_Oran.Kd);
			Serial_SetFloatData(&Serial1, "Goal", "Goal=%f", &PID_Oran.goalPoint);
		}
	}

	// ===== OLED显示 =====
	if (mode2_sub == 1)
	{
		// 角度跟踪测试
		OLED_Printf(0, 0, OLED_6X8, "==Angle Test[%d]==", mode2_sub) ;
		OLED_Printf(0,10,OLED_6X8,"Tar:%.1f Now:%.1f",
			Stepper1.Angle_Tar, Stepper1.Pos_Now) ;
		OLED_Printf(0,20,OLED_6X8,"K:%.3f,%.3f,%.2f",
			Stepper1.PID_Angle.Kp, Stepper1.PID_Angle.Ki, Stepper1.PID_Angle.Kd) ;
		OLED_Printf(0,30,OLED_6X8,"PID:%.1f Spd:%.0f",
			Stepper1.PID_Angle.setPoint, Stepper1.Speed_Now) ;
	}
	else
	{
		// 位置PID（钢球平衡）
		OLED_Printf(0, 0, OLED_6X8, "===Mode2 Pos===") ;
		OLED_Printf(0,10,OLED_6X8,"Kp:%.3f Ki:%.3f Kd:%.2f",
		PID_Oran.Kp, PID_Oran.Ki, PID_Oran.Kd) ;
		OLED_Printf(0,20,OLED_6X8,"P:%.1f I:%.1f D:%.1f",
		PID_Oran.pout, PID_Oran.iout, PID_Oran.dout) ;
		OLED_Printf(0,30,OLED_6X8,"Goal:%.0f Real:%.0f",
		PID_Oran.goalPoint, PID_Oran.realPoint_Now) ;
		OLED_Printf(0,40,OLED_6X8,"Set:%.1f Spd:%.0f",
		PID_Oran.setPoint, Stepper1.Speed_Now) ;
		OLED_Printf(0,50,OLED_6X8,"Pos:%.1f", Stepper1.Pos_Now) ;
}
	}

void Mode_2_Tick(void)
{
	if (mode2_sub == 1)
	{
		// 角度跟踪CSV: Target, Now, PID_Out, Speed_Now
		Serial_printf(&Serial1 , "%.2f,%.2f,%.2f,%.0f\n",
			Stepper1.Angle_Tar, Stepper1.Pos_Now,
			Stepper1.PID_Angle.setPoint, Stepper1.Speed_Now);
	}
	else
	{
		// 位置PID CSV: Goal, Real, Set
		Serial_printf(&Serial1 , "%.2f,%.2f,%.2f\n",
			PID_Oran.goalPoint, PID_Oran.realPoint_Now, PID_Oran.setPoint);
	}
	if (mode2_sub == 1)
		Oran_Angle_Test_Update() ;  // 写目标角度到 Stepper1
	else
		Oran_PID_Update() ;         // 位置PID → 速度
}

void Mode_2_10ms_Tick(void)
{
	
}

void Mode_2_Exit(void)
{
	Stepper_PWM_Stop(&Stepper1);
}
