#ifndef __ORANGE_H
#define __ORANGE_H

#include "MySystem.h"
#include "Serial_porting.h"

extern int Oran_Param[6] ;

void Oran_Update(void) ;
// 调阈值处理
void Oran_Send_Data(int *Data) ;
// 坐标映射
void Oran_XY_Change(void) ;

#endif
