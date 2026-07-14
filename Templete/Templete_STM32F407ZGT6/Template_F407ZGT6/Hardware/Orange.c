#include "Orange.h"

// 1. x_real 2. y_real 3. x_tar 4. y_tar
float x_real = 640.0f ;
float y_real = 640.0f ;
float x_tar  = 500.0f ;
float y_tar  = 360.0f ;

int angle_shift = 50  ;
int offset      = 20  ;
int black_h     = 20  ;
int black_s     = 255 ;
int black_v     = 100 ;

uint8_t Oran_cmd = 0 ;	// 0 -> 正常数据 1-> 调试模式数据

// 香橙派数据更新,在Mode_G实现20ms固定更新
void Oran_Update(void)
{
	// 读取Serial2的消息
	if (Serial_GetNewPackageFlag_HEX(&Serial2))
	{
		Oran_cmd = Serial_GetHexData(&Serial2 , 0) ;
		if (Oran_cmd == 0)
		{
			x_real = (float)(Serial_GetHexData(&Serial2 , 1) == 0 ?  x_real :  Serial_GetHexData(&Serial2 , 1)) ;
			y_real = (float)(Serial_GetHexData(&Serial2 , 2) == 0 ?  y_real :  Serial_GetHexData(&Serial2 , 2)) ;
			x_tar  = (float)(Serial_GetHexData(&Serial2 , 3) == 0 ?  x_tar  :  Serial_GetHexData(&Serial2 , 3)) ;
			y_tar  = (float)(Serial_GetHexData(&Serial2 , 4) == 0 ?  y_tar  :  Serial_GetHexData(&Serial2 , 4)) ;
		}
		else if (Oran_cmd == 1)
		{
			angle_shift = Serial_GetHexData(&Serial2 , 1) ;
			offset      = Serial_GetHexData(&Serial2 , 2) ;
			black_h     = Serial_GetHexData(&Serial2 , 3) ;
			black_s     = Serial_GetHexData(&Serial2 , 4) ;
			black_v     = Serial_GetHexData(&Serial2 , 5) ;
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






