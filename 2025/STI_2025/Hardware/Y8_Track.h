#ifndef __Y8_TRACK_H
#define __Y8_TRACK_H

#include "main.h"
#include "MyI2C.h"
#include "i2c.h"
#include "stdbool.h"

// ************外部变量声明************
extern uint8_t Y8_Line_Array[9] ;				// 8路传感器数据包
extern bool Y8_Update_Flag ;						// 8路传感器更新标志位

// ************函数声明************

// 读取8路数据,并转化为数组,为了方便起见,数组有9位,1-8为有效数据
void Y8_LineSensor_Update(void) ;

#endif
