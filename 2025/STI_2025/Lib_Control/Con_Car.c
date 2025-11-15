#include "Con_Car.h"

// 小车控制核心库

// **********************变量**********************
Car_Position_Typedef Car_Y8_Pos ;	// Y8巡线得到的方位变量


// **********************Y8算法逻辑**********************
// Y8巡线对照函数
bool Y8_Line_Contrast(int EX1 , int EX2 , int EX3 , int EX4 , int EX5 , int EX6 , int EX7 , int EX8 )
{
	return Y8_Line_Array[1] == EX1 && Y8_Line_Array[2] == EX2 && Y8_Line_Array[3] == EX3 && Y8_Line_Array[4] == EX4 &&
		Y8_Line_Array[5] == EX5 && Y8_Line_Array[6] == EX6 && Y8_Line_Array[7] == EX7 && Y8_Line_Array[8] == EX8 ;
}

// Y8巡线方位判断:*总函数*:状态分析:分析小车现在是出于什么状态 , 巡线识别逻辑分析 
void Y8_Position_Update(void)
{
	// 状态一:小车在第一个弯道
	if (Car_State == Turn_1)
	{
		if (Y8_Line_Contrast(1 , 0 , 0 , 0 , 0 , 0 , 0 , 1) || Y8_Line_Contrast(1 , 0 , 0 , 0 , 0 , 0 , 1 , 0) || 
				Y8_Line_Contrast(0 , 1 , 0 , 0 , 0 , 0 , 1 , 0) || Y8_Line_Contrast(0 , 1 , 0 , 0 , 0 , 0 , 0 , 1))
		{
			// 先确定小车上一个状态
			if ()
		}
	}

}

