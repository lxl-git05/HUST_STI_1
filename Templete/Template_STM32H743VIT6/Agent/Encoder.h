#ifndef __ENCODER_H
#define __ENCODER_H

#include "ti_msp_dl_config.h"

// ==================== 初始化 ====================
void Encoder_Init(IRQn_Type IRQN);

// ==================== 更新编码器(直接放在中断处理函数子位置) ====================
void Encoder_Counter_Tick(GPIO_Regs* Encoder_GPIO_Port , uint32_t Encoder_Pin_1 , uint32_t Encoder_Pin_2 , int32_t* gEncoderCount) ;

// ==================== 获取计数值并清零 ====================
int32_t Encoder_Get_CNT(int32_t* gEncoderCount);

#endif
