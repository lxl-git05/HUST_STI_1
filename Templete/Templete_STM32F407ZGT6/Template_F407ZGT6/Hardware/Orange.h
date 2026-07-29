#ifndef __ORANGE_H
#define __ORANGE_H

#include "MySystem.h"
#include "Serial_porting.h"

extern float x_real ;
extern float y_real ;
extern float x_tar  ;
extern float y_tar  ;
extern float x_change ;	// 坐标映射点
extern float y_change	;	// 坐标映射点
extern int Oran_Check_XY[6] ;	// 3个像素点
extern float Oran_X_A ;	// 线性标定x, y的系数
extern float Oran_X_B ;
extern float Oran_Y_A ;
extern float Oran_Y_B ;

extern int Oran_Param[6] ;	// 香橙派通信参数（Menu_Param 调参用）
extern int angle_shift ;
extern int offset      ;
extern int black_h     ;
extern int black_s     ;
extern int black_v     ;



void Oran_Update(void) ;
// 调阈值处理
void Oran_Send_Data(int *Data) ;
// 坐标映射
void Oran_XY_Change(void) ;

#endif
