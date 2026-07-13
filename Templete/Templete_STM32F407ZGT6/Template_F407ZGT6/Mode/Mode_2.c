#include "Mode_2.h"
#include "AllHeader.h"

// ========================== 步进电机限位测试 ==========================
// 测试流程:
//   KEY1按住 → Stepper1 正向 30RPM → 接近 +120° 自动停止（验证正向限位）
//   KEY2按住 → Stepper1 反向 -30RPM → 接近 -120° 自动停止（验证反向限位）
//   KEY3按住 → Stepper2 正向 30RPM → 接近 +50° 自动停止（验证竖直限位）
// 预期现象:
//   OLED 上 Pos_Now 到达限位角后不再变化，电机停转；松手按反向键可退回

void Mode_2_Setup(void)
{
	OLED_Clear() ;
}

float acc = 0.0f ;

void Mode_2_Loop(void)
{
	OLED_Printf(0, 0, OLED_6X8, "===Mode2 LimitTest===") ;
	OLED_Printf(0, 16, OLED_6X8, "Pos1=%.1f/120", Stepper1.Pos_Now) ;
	OLED_Printf(0, 24, OLED_6X8, "Pos2=%.1f/50",  Stepper2.Pos_Now) ;
	OLED_Printf(0, 40, OLED_6X8, "K1:1+ K2:1- K3:2+") ;
	
	// KEY1: Stepper1 正向（测试 +120° 限位）
	if (Key_Check(KEY_1, KEY_SINGLE)) {
		Stepper_PWM_Speed_Set(&Stepper1, 60.0f, 2);
	}
	// KEY2: Stepper1 反向（测试 -120° 限位）
	else if (Key_Check(KEY_2, KEY_SINGLE)) {
		Stepper_PWM_Speed_Set(&Stepper1, -60.0f, 5);
	}
	// KEY3: Stepper2 正向（测试 +50° 限位）
	else if (Key_Check(KEY_3, KEY_SINGLE)) {
		Stepper_PWM_Speed_Set(&Stepper2, 30.0f, 0);
	}
}

void Mode_2_Tick(void)
{
	Serial_printf(&Serial1, "%.2f,%.2f\n",Stepper1.Pos_Now , Stepper1.Speed_Now) ; 
}

void Mode_2_Exit(void)
{
	Stepper_PWM_Stop(&Stepper1);
	Stepper_PWM_Stop(&Stepper2);
}
