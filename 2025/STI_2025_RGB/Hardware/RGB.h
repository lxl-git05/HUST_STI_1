#ifndef __RGB_H
#define __RGB_H
// 引脚:
//R:A0 ,TIM2_CH1  
//G:A1 ,TIM2_CH2  
//B:A2 ,TIM2_CH3

#include "PWM.h"

// RGB相关参数
#define RGB_htim htim2
#define RGB_R_Channel TIM_CHANNEL_1
#define RGB_G_Channel TIM_CHANNEL_2
#define RGB_B_Channel TIM_CHANNEL_3

#include "PWM.h"

// RGB初始化
void RGB_Init(void) ;

// RGB调色
void RGB_Set_Color(int R_Color , int G_Color , int B_Color ) ;

#endif
