#ifndef __PWM_H
#define __PWM_H

#include "main.h"
#include "tim.h"

// PWM��ʼ��
void PWM_Init(TIM_HandleTypeDef htimx , uint32_t Channel) ;

// ����PWMֵ
void PWM_SetCompare1(TIM_HandleTypeDef htimx , uint32_t Channel , uint16_t Compare) ;

#endif
