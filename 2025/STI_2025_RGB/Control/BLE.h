#ifndef __BLE_H
#define __BLE_H
// 蓝牙模块控制库 
// PA2  ------> USART2_TX  ------>   BLE_RX
// PA3  ------> USART2_RX	 ------>	 BLE_TX
#include "Serial.h"
#include "usart.h"

// 导出两个状态变量
extern int RGB_Mode   ;	// RGB模式状态 ,初始默认为自动
extern int Servo_Mode ;	// 舵机模式状态,初始默认为自动

// 蓝牙控制模块初始化
void BLE_Init(void) ;

// 蓝牙接收数据更新函数
void BLE_Data_Update(void) ;

#endif
