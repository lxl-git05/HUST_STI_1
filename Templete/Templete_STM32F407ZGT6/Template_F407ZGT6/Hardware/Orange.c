#include "Orange.h"

// 1. x_real 2. y_real 3. x_tar 4. y_tar
float x_real = 640.0f ;
float y_real = 640.0f ;
float x_tar  = 500.0f ;
float y_tar  = 360.0f ;

// 香橙派数据更新,在Mode_G实现20ms固定更新
void Oran_Update(void)
{
	// 读取Serial2的消息
	if (Serial_GetNewPackageFlag_HEX(&Serial2))
	{
		x_real = (float)(Serial_GetHexData(&Serial2 , 0) == 0 ?  x_real :  Serial_GetHexData(&Serial2 , 0)) ;
		y_real = (float)(Serial_GetHexData(&Serial2 , 1) == 0 ?  y_real :  Serial_GetHexData(&Serial2 , 1)) ;
		x_tar  = (float)(Serial_GetHexData(&Serial2 , 2) == 0 ?  x_tar  :  Serial_GetHexData(&Serial2 , 2)) ;
		y_tar  = (float)(Serial_GetHexData(&Serial2 , 3) == 0 ?  y_tar  :  Serial_GetHexData(&Serial2 , 3)) ;
	}
}








