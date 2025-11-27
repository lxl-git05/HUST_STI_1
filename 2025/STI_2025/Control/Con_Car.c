#include "Con_Car.h"
extern int Wait_Pos ;
// 小车控制核心库

// **********************变量**********************
// Task2变量
extern int Car_Wait_Flag  ;	// 小车等停标志位
extern int Car_Wait_cnt ;		// 小车等停计时
// ALL Task 变量
extern int Turn_Num_MPU ;
extern bool is_Car_Turn_Left ;

// ====================== 电工基地题目处理 ======================
void LED_Flash(void)
{
	HAL_Delay(1000) ;
	HAL_GPIO_TogglePin(LED0_GPIO_Port , LED0_Pin ) ;
	HAL_Delay(1000) ;
	HAL_GPIO_TogglePin(LED0_GPIO_Port , LED0_Pin ) ;
	HAL_Delay(1000) ;
	HAL_GPIO_TogglePin(LED0_GPIO_Port , LED0_Pin ) ;
	HAL_Delay(1000) ;
	HAL_GPIO_TogglePin(LED0_GPIO_Port , LED0_Pin ) ;
	HAL_Delay(1000) ;
	HAL_GPIO_TogglePin(LED0_GPIO_Port , LED0_Pin ) ;
	HAL_Delay(1000) ;
	HAL_GPIO_TogglePin(LED0_GPIO_Port , LED0_Pin ) ;
	HAL_Delay(1000) ;
}

// 电工基地第1题
void Y8_Task1(void)
{
	// 识别停止
	if (Y8_is_Init() && Turn_Num_MPU >= 4)
	{
		isBreak = 1 ;
		Motor_Stop_Force() ;
	}
	if (Pi_Stop_Status == 1  && Turn_Num_MPU >= 4 )
	{
		isBreak = 1 ;
//		LED_Flash() ;
	}
}

// 电工基地第2题
void Y8_Task2(void)
{
	// 记录等停时圈数,与接下来停止圈数进行对比
	static int Wait_Position ;
	static int Wait_Pos_Num ;	// 识别等停的区间
	if (Wait_Pos == 1)
	{
		Wait_Pos_Num = 1 ;
	}
	else
	{
		Wait_Pos_Num = 3 ;
	}
	// 等停处理 
	if ((Pi_Stop_Status == 2 && Car_Wait_Flag == 0)|| (Y8_is_Wait() && Car_Wait_Flag == 0 && 2 == Turn_Num_MPU ) )
	{
		Car_Wait_Flag = 1 ;
		Car_Wait_cnt  = 5000 ;
		isBreak = 1 ;
		Wait_Position = Turn_Num_MPU ;	// 记录等停识别的圈数
	}
	// 识别停止
	if (Y8_is_Init() && Turn_Num_MPU >= 4)
	{
		isBreak = 1 ;
		Motor_Stop_Force() ;
	}
	if (Pi_Stop_Status == 1 && Turn_Num_MPU >= 4 && Wait_Position != Turn_Num_MPU)	// 视觉识别停止和等停如果在同一圈就视为误识别,Y8全权操作
	{
		isBreak = 1 ;
//		LED_Flash() ;
	}
}

// 电工基地第3题
void Y8_Task3(void)
{
	// 识别停止
	if (Y8_is_Init() && Turn_Num_MPU >= 4)
	{
		isBreak = 1 ;
	}
	if (Pi_Stop_Status == 1  && Turn_Num_MPU >= 4 )
	{
		isBreak = 1 ;
	}
}

// 电工基地第4题
void Y8_Task4(void)
{
	// 识别停止,跑12个1/4圈
	if (Y8_is_Init() && Turn_Num_MPU >= 12)
	{
		isBreak = 1 ;
	}
	if (Turn_Num_MPU % 4 == 1)
	{
		is_Car_Turn_Left = 0 ;	// 默认右转
	}
	if (Pi_Stop_Status == 1  && Turn_Num_MPU >= 12 )
	{
		isBreak = 1 ;
//		LED_Flash() ;
	}
	// LR转换 is_Car_Turn_Left Pi_LR_Status
	if (Pi_LR_Status != 0)
	{
		is_Car_Turn_Left = Pi_LR_Status - 1 ;
	}
}

// 电工基地第5题
void Y8_Task5(void)
{
	static bool is_RGB_Open = true ;
	// 识别停止,跑16个1/4圈
	if (Y8_is_Init() && Turn_Num_MPU >= 16)
	{
		isBreak = 1 ;
		is_RGB_Open = false ;
	}
	if (Pi_Stop_Status == 1  && Turn_Num_MPU >= 16 )
	{
		isBreak = 1 ;
//		LED_Flash() ;
	}
	// RGB检测命令执行 RGB -> 0初始化 , 1红灯 , 2绿灯 , 3黄灯
	if (Pi_RGB_Status != 0 && is_RGB_Open == true)
	{
		if (Pi_RGB_Status == 1)
		{
			isBreak = 1 ;
		}
		else if (Pi_RGB_Status == 2)
		{
			isBreak = 0 ;
			Y8_Lose_Line_isOK = true ;
		}
		else if (Pi_RGB_Status == 3 && Turn_Num_MPU % 4 == 0)
		{
			// 树莓派识别到停止位置,说明小车没有超过停止线,那么就停车,否则继续跑
			if (Pi_Stop_Status != 0 || Y8_Pos != Y8_Init_Pos)
			{
				isBreak = 1 ;
			}
		}
	}
}

// 电工基地试题
void Car_Task(int Car_Task_Seq)
{
	if (Car_Task_Seq == 1)
	{
		Y8_Task1() ;
	}
	else if (Car_Task_Seq == 2)
	{
		Y8_Task2() ;
	}
	else if (Car_Task_Seq == 3)
	{
		Y8_Task3() ;
	}
	else if (Car_Task_Seq == 4)
	{
		Y8_Task4() ;
	}
	else if (Car_Task_Seq == 5)
	{
		Y8_Task5() ;
	}
}
