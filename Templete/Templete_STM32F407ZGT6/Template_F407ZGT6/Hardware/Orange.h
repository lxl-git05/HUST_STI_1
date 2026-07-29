#ifndef __ORANGE_H
#define __ORANGE_H

#include "MySystem.h"
#include "Serial_porting.h"

extern int Oran_Param[6] ;

extern int Oran_Goal ;	// STM32发送给Orange的参数
extern int Oran_real 	;	// 1. 真实值(偏移)
extern int Oran_Speed ;	// 2. 速度

void Oran_Update(void) ;
// 调阈值处理
void Oran_Send_Data(int *Data) ;
// 坐标映射
void Oran_XY_Change(void) ;

#endif
