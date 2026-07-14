#include "tim.h"

// 定时器初始化
void Timer_Initial(void)
{
	HAL_TIM_Base_Start_IT(&htim6);
	HAL_TIM_Base_Start_IT(&htim7);
}

/*弱函数声明,主函数调用*/
__attribute__((weak)) void Timer_1ms_Callback(void)
{
    // 默认空实现
}

/*弱函数声明,主函数调用*/
__attribute__((weak)) void Timer_20ms_Callback(void)
{
    // 默认空实现
}

__attribute__((weak)) void Timer_Stepper1_Pulse_Callback(void)
{
    // 默认空实现，由Mode_G层重写，调用Stepper_PWM_Pulse_Count
}

__attribute__((weak)) void Timer_Stepper2_Pulse_Callback(void)
{
    // 默认空实现，由Mode_G层重写，调用Stepper_PWM_Pulse_Count
}

// 定时器回调函数
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  //判断是否是TIM6中断
  if (htim->Instance == TIM6)
  {
    Timer_1ms_Callback() ;	// 1ms中断
  }
	// 判断是否是TIM7中断
	if (htim->Instance == TIM7)
	{
		Timer_20ms_Callback() ;	// 20ms中断
	}
	// 步进电机脉冲计数（TIM9=Stepper1, TIM12=Stepper2）
	if (htim->Instance == TIM9)
	{
	    Timer_Stepper1_Pulse_Callback();
	}
	if (htim->Instance == TIM12)
	{
	    Timer_Stepper2_Pulse_Callback();
	}
}
