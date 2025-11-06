#include "RasPi.h"
// 全局参数
int Pi_xLine_goal = 160;	
// 数据包发送内容
int Pi_xLine_real = 160;		// x 的真实值,数据量 x_real + 100
int Pi_task1	;							// 运动: 0 , 停止: 1 ,等停5秒: 2
int Pi_angle 	;							// angle + 100:偏转角度


// 数据更新函数
void RasPi_Data_Update(void)
{
	// 数据更新
	if (Serial3_GetNewPackageFlag_HEX() == 1)
	{
		// Serial3_New_Package:// 3个 : x_real , task1 , angle
		Pi_xLine_real = Serial3_Hex_Data.Serial3_New_Package[1] - 100 ;	
		Pi_task1 = Serial3_Hex_Data.Serial3_New_Package[2] ;	
		Pi_angle = Serial3_Hex_Data.Serial3_New_Package[3] - 100 ;
	}
	
	// 巡线逻辑判断:偏差较小就亮灯,偏差大就灭灯
	if (Pi_xLine_real - Pi_xLine_goal < 20 && Pi_xLine_real - Pi_xLine_goal > -20)
	{
		HAL_GPIO_WritePin(LED0_GPIO_Port , LED0_Pin , GPIO_PIN_RESET);
	}
	else
	{
		HAL_GPIO_WritePin(LED0_GPIO_Port , LED0_Pin , GPIO_PIN_SET);
	}
}

