#include "Con_Stepper.h"

void Stepper_Init(void)
{
	// 假设 1.8° 步进角，16 细分 → 0.1125°/脉冲，正方向暂用STEPPER_DIR_P
	Stepper_PWM_Init(&Stepper1, &MyPWM_Stepper1, &MyGPIO_Stepper_Dir, 0.1125f, STEPPER_DIR_P);
	Stepper_PWM_Init(&Stepper2, &MyPWM_Stepper2, &MyGPIO_Stepper2_Dir, 0.1125f, STEPPER_DIR_P);
}

