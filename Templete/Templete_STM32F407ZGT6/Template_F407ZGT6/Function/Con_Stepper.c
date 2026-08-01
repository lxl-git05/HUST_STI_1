#include "Con_Stepper.h"
#include "Orange.h"
// 云台运动:电机1水平旋转(顺时针为正)，电机2竖直旋转(顺时针为正)

void Stepper_Init(void)
{
	// 使能步进电机驱动（EN=1 使能，高有效）
	MyGPIO_WritePin(&MyGPIO_Stepper_En, 1);
	MyGPIO_WritePin(&MyGPIO_Stepper2_En, 1);

	// 假设 1.8° 步进角，16 细分 → 0.1125°/脉冲，正方向暂用STEPPER_DIR_P
	Stepper_PWM_Init(&Stepper1, &MyPWM_Stepper1, &MyGPIO_Stepper_Dir, 0.1125f, STEPPER_DIR_P);
	Stepper_PWM_Init(&Stepper2, &MyPWM_Stepper2, &MyGPIO_Stepper2_Dir, 0.1125f, STEPPER_DIR_P);
	
	PID_Init(&Stepper1.PID_Angle , 4.0f , 0.0f , 0.829f , 200.0f , -200.0f , 1000.0f) ;
	PID_Init(&Stepper2.PID_Angle , 4.0f , 0.0f , 0.224f , 200.0f , -200.0f , 1000.0f) ;

	// 软件限位配置
//	Stepper_PWM_Limit_Config(&Stepper1, 120.0f, -120.0f);  // 电机1 水平旋转 ±120°
//	Stepper_PWM_Limit_Config(&Stepper2, 50.0f,  -50.0f);   // 电机2 竖直旋转 ±50°
}
