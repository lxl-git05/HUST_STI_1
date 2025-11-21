#include "Encoder.h"

// 编码器初始化
void Encoder_Init(TIM_HandleTypeDef *htimx)
{
	HAL_TIM_Encoder_Start(htimx, TIM_CHANNEL_ALL);
}

// 得到编码器的脉冲数
int Encoder_Get_CNT(TIM_HandleTypeDef *htimx)
{
	// 得到一次采样时间的脉冲数
	int cnt = __HAL_TIM_GET_COUNTER(htimx);

	// 得到脉冲数,>0为正,<0为负
	if(cnt > 0x7fff)
	{
		cnt = cnt - 0xffff;	// 反转,否则就是正转,没变化
	}

	// 清零
	__HAL_TIM_SET_COUNTER(htimx, 0);
	
	// 返回本次周期的计数值
	return cnt;
}

