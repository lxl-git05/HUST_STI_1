#include "Orange.h"

// 0: cmd 1. x_tar 2. y_tar 
float x_tar  		= 0 ;	// 1. x目标值
float y_tar  		= 0 ;	// 2. y目标值
float x_real  	= 0 ;	// 3. x真实值(在本题用不上)
float y_real  	= 0 ;	// 4. y真实值(在本题用不上)
float x_change	= 0 ;	// 坐标映射点
float y_change	= 0 ;	// 坐标映射点

int angle_shift = 50  ;
int offset      = 20  ;
int black_h     = 20  ;
int black_s     = 255 ;
int black_v     = 100 ;

int Oran_Check_XY[6] = {0} ;
int Oran_Param[6] ;	// 香橙派通信参数（Menu_Param 调参用）
float Oran_X_A = -3;	
float Oran_X_B = 2850;	
float Oran_Y_A = 2.889;	
float Oran_Y_B = 160;	

uint8_t Oran_cmd = 0 ;	// 0 -> 正常数据 1-> 调试模式数据 2->坐标标定数据(3组点)

// 坐标映射
void Oran_XY_Change(void)
{
	x_change = x_tar * Oran_X_A + Oran_X_B ;
	y_change = y_tar * Oran_Y_A + Oran_Y_B ;
}

// 香橙派数据更新,在Mode_G实现20ms固定更新
void Oran_Update(void)
{
	// 读取Serial2的消息
	if (Serial_GetNewPackageFlag_HEX(&Serial2))
	{
		Oran_cmd = Serial_GetHexData(&Serial2 , 0) ;
		if (Oran_cmd == 0)
		{
			x_tar  = (float)(Serial_GetHexData(&Serial2 , 1) == 0 ?  x_tar  :  Serial_GetHexData(&Serial2 , 1)) ;
			y_tar  = (float)(Serial_GetHexData(&Serial2 , 2) == 0 ?  y_tar  :  Serial_GetHexData(&Serial2 , 2)) ;
		}
		else if (Oran_cmd == 1)
		{
			angle_shift = Serial_GetHexData(&Serial2 , 1) ;
			offset      = Serial_GetHexData(&Serial2 , 2) ;
			black_h     = Serial_GetHexData(&Serial2 , 3) ;
			black_s     = Serial_GetHexData(&Serial2 , 4) ;
			black_v     = Serial_GetHexData(&Serial2 , 5) ;
		}
		else if (Oran_cmd == 2)
		{
			Oran_Check_XY[0] = Serial_GetHexData(&Serial2 , 1) ;
			Oran_Check_XY[1] = Serial_GetHexData(&Serial2 , 2) ;
			Oran_Check_XY[2] = Serial_GetHexData(&Serial2 , 3) ;
			Oran_Check_XY[3] = Serial_GetHexData(&Serial2 , 4) ;
			Oran_Check_XY[4] = Serial_GetHexData(&Serial2 , 5) ;
			Oran_Check_XY[5] = Serial_GetHexData(&Serial2 , 6) ;
		}
	}
}

// 调阈值处理
void Oran_Send_Data(int *Data)
{
	if (Data == &angle_shift) {Serial_printf(&Serial2 , "@angle_shift:%d$#",angle_shift);}
	if (Data == &offset)  {Serial_printf(&Serial2 , "@offset:%d$#",offset) ;}
	if (Data == &black_h) {Serial_printf(&Serial2 , "@black_h:%d$#",black_h);}
	if (Data == &black_s) {Serial_printf(&Serial2 , "@black_s:%d$#",black_s);}
	if (Data == &black_v) {Serial_printf(&Serial2 , "@black_v:%d$#",black_v);}
}
