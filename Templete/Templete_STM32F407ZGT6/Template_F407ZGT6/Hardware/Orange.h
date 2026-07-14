#ifndef __ORANGE_H
#define __ORANGE_H

#include "MySystem.h"
#include "Serial_porting.h"

extern float x_real ;
extern float y_real ;
extern float x_tar  ;
extern float y_tar  ;

extern int angle_shift ;
extern int offset      ;
extern int black_h     ;
extern int black_s     ;
extern int black_v     ;

void Oran_Update(void) ;
// 调阈值处理
void Oran_Send_Data(int *Data) ;

#endif
