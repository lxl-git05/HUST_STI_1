#ifndef __ENCODER_H
#define __ENCODER_H

#include "main.h"
#include "tim.h"

// *****************����*****************

// ��������ʼ��
void Encoder_Init(TIM_HandleTypeDef *htimx) ;

// �õ���������������
int Encoder_Get_CNT(TIM_HandleTypeDef *htimx) ;
	
#endif
