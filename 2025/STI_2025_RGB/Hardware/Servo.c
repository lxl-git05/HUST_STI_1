#include "Servo.h"

// 舵机初始化
void Servo_Init(void)
{
	PWM_Init(&Servo_htim , Servo_Channel ) ;
}

// 舵机调节角度:0度-180度
void Servo_Set_Angle(int Angle)
{
	// 舵机角度限幅
	if (Angle < 0)
		Angle = 0 ;
	else if (Angle > 180)
		Angle = 180 ;
	// 舵机角度调整
	PWM_SetCompare1( Servo_htim , Servo_Channel , 500 + (Angle * 2000 / 180) ) ;
}
