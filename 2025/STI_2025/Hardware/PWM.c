#include "PWM.h"

// PWM��ʼ��
void PWM_Init(TIM_HandleTypeDef htimx , uint32_t Channel)
{
	HAL_TIM_PWM_Start(&htimx , Channel) ;
}

// ����PWMֵ
void PWM_SetCompare1(TIM_HandleTypeDef htimx , uint32_t Channel , uint16_t Compare)
{
	// ����LED��ռ�ձ�,LED_PWM��Ҫ��0-ARR֮��
	__HAL_TIM_SET_COMPARE(&htimx , Channel , Compare ) ;
}
