#include "RasPi.h"
// 全局参数
int Pi_xLine_goal = 160;	

// ============================================================
/* 数据包发送内容:
	RGB -> 0初始化 , 1红灯 , 2绿灯 , 3黄灯
	LR  -> 0初始化 , 1->L  , 2->R
	wait & stop -> 0无 , 1停止 , 2等停
	x_line_real -> *x传输过来+100,处理时需要-100* , 
*/
int Pi_RGB_Status  = 0 ;	// RGB -> 0初始化 , 1红灯 , 2绿灯 , 3黄灯
int Pi_LR_Status	 = 0 ;	// LR  -> 0初始化 , 1->L  , 2->R
int Pi_Stop_Status = 0 ;	// wait & stop -> 0无 , 1停止 , 2等停
int Pi_x_Line_real = 0 ;	// 小车的巡线x值 , 目标值在下面

// ============================================================
extern bool isBreak ;
extern int Pi_Wait_Flag ;

// 数据更新函数
void RasPi_Data_Update(void)
{
	// 数据更新
	if (Serial3_GetNewPackageFlag_HEX() == 1)
	{
		// Serial3_New_Package:
		Pi_RGB_Status  = Serial3_Hex_Data.Serial3_New_Package[1] 			 ;	
		Pi_LR_Status   = Serial3_Hex_Data.Serial3_New_Package[2] 			 ;	
		Pi_Stop_Status = Serial3_Hex_Data.Serial3_New_Package[3] 			 ;
		Pi_x_Line_real = Serial3_Hex_Data.Serial3_New_Package[4] - 100 ;
	}
}

