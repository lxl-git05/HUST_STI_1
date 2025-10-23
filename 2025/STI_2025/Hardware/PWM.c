#include "PWM.h"

// PWM初始化
void PWM_Init(TIM_HandleTypeDef htimx , uint32_t Channel)
{
	HAL_TIM_PWM_Start(&htimx , Channel) ;
}

// 设置PWM值
void PWM_SetCompare1(TIM_HandleTypeDef htimx , uint32_t Channel , uint16_t Compare)
{
	// 设置LED的占空比,LED_PWM需要在0-ARR之间
	__HAL_TIM_SET_COMPARE(&htimx , Channel , Compare ) ;
}
