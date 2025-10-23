#include "Encoder_Motor.h"

//int Motor_Cnt ;

//// 电机初始化
//void Motor_Init(void)
//{
//	// 启动编码器接口
//	HAL_TIM_Encoder_Start(&Motor_htim, TIM_CHANNEL_ALL);
//}

//// 初始调试编码器测速
//void Motor_Encoder_Tick(void)
//{
//	static int Motor_Encoder_count = 0 ;
//	Motor_Encoder_count ++ ;
//	if ( Motor_Encoder_count == 1000)
//	{
//		Motor_Cnt = __HAL_TIM_GET_COUNTER(&Motor_htim) ;	// 得到一个采样周期间CNT的个数(不要溢出了否则CNT刷新,误差非常大)
//		
//		__HAL_TIM_SET_COUNTER(&Motor_htim , 0) ;	// 清零CNT,开启新的采样周期
//		
//		Motor_Encoder_count = 1 ;
//		
//	}
//	
//}
