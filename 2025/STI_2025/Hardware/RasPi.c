#include "RasPi.h"
// 全局参数
int Pi_xLine_goal = 160;	
// 数据包发送内容
int Pi_xLine_real = 160;		// x 的真实值,数据量 x_real + 100
int Pi_task1	;							// 运动: 0 , 停止: 1 ,等停5秒: 2
int Pi_angle 	;							// angle + 100:偏转角度
bool Pi_is_Left ;						// 小车遇到岔路口默认为右转
int RGB_Status = 0 ;				// 红绿灯识别状态,1:红灯停 2:绿灯行

extern bool isBreak ;
extern int Pi_Wait_Flag ;



// 数据更新函数
void RasPi_Data_Update(void)
{
	// 数据更新
	if (Serial3_GetNewPackageFlag_HEX() == 1)
	{
		// Serial3_New_Package:// 5个 : x_real , task1 , angle , 
		Pi_xLine_real = Serial3_Hex_Data.Serial3_New_Package[1] - 100 ;	
		Pi_task1 = Serial3_Hex_Data.Serial3_New_Package[2] ;	
		Pi_angle = Serial3_Hex_Data.Serial3_New_Package[3] - 100 ;
		
	}
	
//	// 巡线逻辑判断:偏差较小就亮灯,偏差大就灭灯
//	if (Pi_xLine_real - Pi_xLine_goal < 20 && Pi_xLine_real - Pi_xLine_goal > -20)
//	{
//		HAL_GPIO_WritePin(LED0_GPIO_Port , LED0_Pin , GPIO_PIN_RESET);
//	}
//	else
//	{
//		HAL_GPIO_WritePin(LED0_GPIO_Port , LED0_Pin , GPIO_PIN_SET);
//	}
}

// 树莓派指令代码实现
void RasPi_Func(void)
{
	// 数据包2:task1:终点停止 / 等停功能实现
	if (Pi_task1 == 1)
	{
		isBreak = true ;
	}
	// 等停标示,等待5s
	else if (Pi_task1 == 2 && Pi_Wait_Flag == 0)
	{
		isBreak = true ;
		Pi_Wait_Flag = 1 ;	// 标志位置1,开始倒计时
	}
	
	// 数据包4:红绿灯状态
	if (RGB_Status == 0)			// 初始状态
	{
		;
	}
	else if (RGB_Status == 2)	// 绿灯行
	{
		isBreak = false ;
	}
	else if (RGB_Status == 1)	// 红灯停
	{
		isBreak = true ;
	}
}
