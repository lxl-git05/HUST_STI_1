#ifndef __RGB_H
#define __RGB_H
// 引脚:
//R:A0 ,TIM2_CH1  
//G:A1 ,TIM2_CH2  
//B:A2 ,TIM2_CH3

#include "PWM.h"
#include "stdbool.h"	// bool
#include "Task.h"			// 任务调度
#include "Key.h"			// 按键控制RGB

// RGB相关参数
#define RGB_htim htim2
#define RGB_R_Channel TIM_CHANNEL_1
#define RGB_G_Channel TIM_CHANNEL_2
#define RGB_B_Channel TIM_CHANNEL_3

// RGB颜色码
typedef enum
{
	RGB_DOWN = 0x00U,			// 闭灯
	RGB_R		 = 0x01U,			// 红色
	RGB_Y 	 = 0x02U,			// 黄色
	RGB_G 	 = 0x03U,			// 绿色
	RGB_B 	 = 0x04U, 		// 蓝色
	
}RGB_Color_Num ;

#include "PWM.h"

// RGB初始化
void RGB_Init(void) ;

// RGB调色
void RGB_Set_Color(int R_Color , int G_Color , int B_Color ) ;

// RGB闪烁任务,Mode为1代表自动挡,Mode为0代表手动挡
void RGB_Control(bool Mode) ;

// RGB自动档执行任务
void RGB_Auto_Task__Possess(void) ;

#endif
