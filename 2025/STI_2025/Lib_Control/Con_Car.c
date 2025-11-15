#include "Con_Car.h"

// 小车控制核心库

// **********************变量**********************
Car_Position_Typedef Car_Y8_Pos ;			// Y8巡线得到的方位变量
extern int goalPoint_A ;
extern int goalPoint_B ;

// **********************Y8算法逻辑**********************
// Y8巡线对照函数
bool Y8_Line_Contrast(int EX1 , int EX2 , int EX3 , int EX4 , int EX5 , int EX6 , int EX7 , int EX8 )
{
	return Y8_Line_Array[1] == EX1 && Y8_Line_Array[2] == EX2 && Y8_Line_Array[3] == EX3 && Y8_Line_Array[4] == EX4 &&
		Y8_Line_Array[5] == EX5 && Y8_Line_Array[6] == EX6 && Y8_Line_Array[7] == EX7 && Y8_Line_Array[8] == EX8 ;
}

// 弯道进入直道逻辑判断
bool Turn_to_Cross(void)
{
	// 走弯道时,理论上不可能出现内轮大于外轮的情况,所以内<外,而进入直道,如果还是这个状态必然偏向,所以内轮与外轮必然产生交叉,所以逻辑如下
	// 当然,仅仅一个函数肯定不够,但是在状态机判断方位的逻辑下足够有把握
	if (goalPoint_A - goalPoint_B > -5 && goalPoint_A - goalPoint_B < 5)
	{
		return true ;
	}
	return false ;
}

// Y8巡线方位判断:*总函数*:状态分析:分析小车现在是出于什么状态 , 巡线识别逻辑分析 
int Y8_Position_Update(bool is_Turn_Left)
{
	// 状态1:小车在初始化短直道,准备进入分叉路口
	if (Car_Y8_Pos == Car_Cross_Init_way)
	{
		if (Y8_Line_Contrast(1 , 0 , 0 , 0 , 0 , 0 , 0 , 1) || Y8_Line_Contrast(1 , 0 , 0 , 0 , 0 , 0 , 1 , 0) || 
				Y8_Line_Contrast(1 , 0 , 0 , 0 , 0 , 1 , 0 , 0) || Y8_Line_Contrast(1 , 0 , 0 , 0 , 1 , 0 , 0 , 0) || 
		
				Y8_Line_Contrast(1 , 0 , 0 , 0 , 0 , 0 , 0 , 1) || Y8_Line_Contrast(0 , 1 , 0 , 0 , 0 , 0 , 0 , 1) || 
				Y8_Line_Contrast(0 , 0 , 1 , 0 , 0 , 0 , 0 , 1) || Y8_Line_Contrast(0 , 0 , 0 , 1 , 0 , 0 , 0 , 1) || 
		
				Y8_Line_Contrast(0 , 1 , 0 , 0 , 0 , 0 , 1 , 0) || Y8_Line_Contrast(0 , 1 , 0 , 0 , 0 , 1 , 0 , 0) || 
				Y8_Line_Contrast(0 , 1 , 0 , 0 , 0 , 0 , 1 , 0) || Y8_Line_Contrast(0 , 0 , 1 , 0 , 0 , 0 , 1 , 0) 
			 )
		{
			// 进入分叉路口
			if (is_Turn_Left == true)
			{
				Car_Y8_Pos = Car_Turn_L_In_way ;
			}
			else
			{
				Car_Y8_Pos = Car_Turn_R_In_way ;
			}
		}
	}	
	
	// 状态2:小车在分叉路口,准备出分叉路口,速度发生交叉说明进入直道
	else if (Car_Y8_Pos == Car_Turn_L_In_way || Car_Y8_Pos == Car_Turn_R_In_way)
	{
		if (Turn_to_Cross() == true)
		{
			Car_Y8_Pos = Car_Cross_L_In_way ;
			return 1 ;
		}
	}
	return 0 ;
}



//if (Y8_Line_Contrast(1 , 0 , 0 , 0 , 0 , 0 , 0 , 1) || Y8_Line_Contrast(1 , 0 , 0 , 0 , 0 , 0 , 1 , 0) || 
//				Y8_Line_Contrast(1 , 0 , 0 , 0 , 0 , 1 , 0 , 0) || Y8_Line_Contrast(1 , 0 , 0 , 0 , 1 , 0 , 0 , 0) || 
//		
//				Y8_Line_Contrast(0 , 0 , 0 , 0 , 0 , 0 , 0 , 0) || Y8_Line_Contrast(0 , 0 , 0 , 0 , 0 , 0 , 0 , 0) || 
//				Y8_Line_Contrast(0 , 0 , 0 , 0 , 0 , 0 , 0 , 0) || Y8_Line_Contrast(0 , 0 , 0 , 0 , 0 , 0 , 0 , 0) || 
//		
//				Y8_Line_Contrast(0 , 0 , 0 , 0 , 0 , 0 , 0 , 0) || Y8_Line_Contrast(0 , 0 , 0 , 0 , 0 , 0 , 0 , 0) || 
//				Y8_Line_Contrast(0 , 0 , 0 , 0 , 0 , 0 , 0 , 0) || Y8_Line_Contrast(0 , 0 , 0 , 0 , 0 , 0 , 0 , 0)
//			 )

