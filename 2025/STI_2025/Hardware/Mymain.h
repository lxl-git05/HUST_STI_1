#ifndef __MYMAIN_H
#define __MYMAIN_H

// 系统库
#include <stdlib.h>
#include "string.h"
#include <stdio.h>
#include <math.h>
#include "main.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "stdbool.h"

// 自设库
#include "OLED.h"
#include "Key.h"
#include "Serial.h"
#include "Serial3.h"
#include "Task.h"

#include "Encoder_Motor.h"
#include "Encoder.h"
#include "Motor.h"
#include "RasPi.h"
#include "Timer_Counter.h"
#include "Y8_Track.h"
#include "Con_Motor.h"
#include "Con_Car.h"
#include "Menu_Key.h"
#include "Key_Check.h"

// 截胡主函数
void Mymain(void) ;

#endif
