#ifndef __RASPI_H
#define __RASPI_H

#include "MyPID.h"
#include "Serial3.h"

// *************** 变量声明 ***************

// 树莓派视觉传感器
extern int Pi_RGB_Status  ;	// RGB -> 0初始化 , 1红灯 , 2绿灯 , 3黄灯
extern int Pi_LR_Status	  ;	// LR  -> 0初始化 , 1->L  , 2->R
extern int Pi_Stop_Status ;	// wait & stop -> 0无 , 1停止 , 2等停
extern int Pi_x_Line_real ;	// 巡线x的真实值,已处理

// *************** 函数声明 ***************

// 树莓派数据更新
void RasPi_Data_Update(void) ;

#endif
