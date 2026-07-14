#ifndef __MYSYSTEM_H
#define __MYSYSTEM_H

/* ====================================================================
 * MySystem.h — 芯片底层抽象层
 *
 * 【跨芯片移植指南】只需修改以下两处：
 * 1. 芯片底层头文件（main.h / gpio.h / tim.h / usart.h等）
 *    替换为目标芯片对应的头文件
 * 2. MySystem_Fre 改为目标芯片主频（单位：Hz）
 *
 * 其他文件（Hardware / Function / Mode）无需修改
 * ==================================================================== */

// -------------------- 芯片主频（Hz）【移植时修改这里】--------------------
#define MySystem_Fre 240000000

// -------------------- 芯片底层头文件（移植时修改这里）--------------------
#include "main.h"   // 系统总头文件
#include "gpio.h"   // GPIO 外设
#include "tim.h"    // 定时器/PWM/Encoder
#include "usart.h"  // 串口通信
// #include "i2c.h"
// #include "adc.h"
 #include "dma.h"

// -------------------- C标准库 --------------------
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

// -------------------- 抽象层头文件 --------------------
#include "MyGPIO.h"
#include "MyPWM.h"
#include "MyEncoder.h"
#include "MyTimer.h"

#endif // !__MYSYSTEM_H
