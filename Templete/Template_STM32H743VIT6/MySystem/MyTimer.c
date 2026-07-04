#include "tim.h"

// 定时器初始化
void Timer_Initial(void)
{
	HAL_TIM_Base_Start_IT(&htim16);
	HAL_TIM_Base_Start_IT(&htim17);
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

// 定时器回调函数
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  //判断是否是TIM17中断
  if (htim->Instance == TIM17)
  {
    Timer_1ms_Callback() ;	// 1ms中断
  }
	// 判断是否是TIM16中断
	else if (htim->Instance == TIM16)
	{
		Timer_20ms_Callback() ;	// 20ms中断
	}
}
