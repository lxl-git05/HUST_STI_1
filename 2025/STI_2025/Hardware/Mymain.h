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

// 自设库
#include "OLED.h"
#include "Key.h"
#include "Serial.h"
#include "Serial3.h"

#include "Encoder_Motor.h"
#include "Encoder.h"
#include "Motor.h"

// 任务管理结构体
typedef struct 
{
	uint8_t Flag;
	uint32_t cnt; 
	uint32_t cycle;
	uint8_t Enable;
	void (*callback)(void);   // 新增：回调函数指针
}mytask;

// 截胡主函数
void Mymain(void) ;
// 任务初始化(setup)
void taskInit(mytask* task,uint32_t cnt_init,uint32_t cycle_init ,void (*callback_func)(void) );
// 任务周期函数(放在定时器)
void task_possess(mytask* task);

#endif
