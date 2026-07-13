#ifndef __MYENCODER_H
#define __MYENCODER_H

/*这里的编码器是编码器模式,而不是外部触发模式,外部触发需要另外写相应代码*/

#include "MySystem.h"

typedef struct
{
	TIM_HandleTypeDef *htimx;
	uint32_t time_Fre ;		// 倍频数(2倍频,4倍频等等)
	int total_cnt ;
}MyEncoder_Typedef;

extern MyEncoder_Typedef Motor_A_Encoder ;
extern MyEncoder_Typedef Motor_B_Encoder ;

// 1. 编码器初始化
void MyEncoder_Init(MyEncoder_Typedef* MyEncoder) ;

// 2. 得到编码器的脉冲数
int MyEncoder_Get_CNT(MyEncoder_Typedef* MyEncoder) ;

// 3. 得到累计脉冲数
int MyEncoder_Get_Total_CNT(MyEncoder_Typedef* MyEncoder) ;

#endif
