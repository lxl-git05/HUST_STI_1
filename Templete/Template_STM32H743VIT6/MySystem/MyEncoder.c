#include "MyEncoder.h"

// 0. 编码器定义
MyEncoder_Typedef Motor_A_Encoder = {&htim2 , 4, 0};
MyEncoder_Typedef Motor_B_Encoder = {&htim3 , 4, 0};

// 1. 编码器初始化
void MyEncoder_Init(MyEncoder_Typedef* MyEncoder)
{
	HAL_TIM_Encoder_Start(MyEncoder->htimx, TIM_CHANNEL_ALL);
}

// 2. 得到编码器的脉冲数
int MyEncoder_Get_CNT(MyEncoder_Typedef* MyEncoder)
{
	// 得到一次采样时间的脉冲数
	int cnt = __HAL_TIM_GET_COUNTER(MyEncoder->htimx);

	// 得到脉冲数,>0为正,<0为负
	if(cnt > MyEncoder->htimx->Instance->ARR/2)
	{
		cnt = cnt - MyEncoder->htimx->Instance->ARR;	// 反转,否则就是正转,没变化
	}
	
	// 脉冲累加
	MyEncoder->total_cnt += cnt ;

	// 清零
	__HAL_TIM_SET_COUNTER(MyEncoder->htimx, 0);
	
	// 返回本次周期的计数值
	return cnt;
}

// 3. 得到累计脉冲数
int MyEncoder_Get_Total_CNT(MyEncoder_Typedef* MyEncoder)
{
	return MyEncoder->total_cnt ;
}
