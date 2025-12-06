#include "Func_Out.h"
#include "PWM.h"

// 函数发生器的TIM配置
#define Func_Out_TIM  TIM1
#define Func_Out_htim htim1
#define Func_Out_Channel	TIM_CHANNEL_1

// 函数发生器初始化
void Func_Out_Init(void)
{
	PWM_Init(Func_Out_htim , Func_Out_Channel) ;	// PWM输出
}

// 函数发生器输出函数
void Func_Out_Square(void)
{
	
}

