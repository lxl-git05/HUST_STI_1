#ifndef __Y8_TRACK_H
#define __Y8_TRACK_H

#include "main.h"

extern uint8_t Y8_Line_Array[9] ;	// 8路传感器数据包

// 读取8路数据,并转化为数组,为了方便起见,数组有9位,1-8为有效数据
void Y8_LineSensor_Update(void) ;

#endif
