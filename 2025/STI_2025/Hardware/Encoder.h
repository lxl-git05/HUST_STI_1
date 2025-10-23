#ifndef __ENCODER_H
#define __ENCODER_H

#include "main.h"
#include "tim.h"

// *****************函数*****************

// 编码器初始化
void Encoder_Init(TIM_HandleTypeDef *htimx) ;

// 得到编码器的脉冲数
int Encoder_Get_CNT(TIM_HandleTypeDef *htimx) ;
	
#endif
