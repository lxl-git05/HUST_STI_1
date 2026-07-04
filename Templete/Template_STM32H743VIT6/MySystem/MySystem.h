#ifndef __MYSYSTEM_H
#define __MYSYSTEM_H

/* 系统相关头文件,后续移植到别的芯片只需要修改本部分 */

// 1. 芯片底层驱动
#include "main.h"   // 主函数
#include "stdint.h" // 整形数据使用
#include "gpio.h"   // GPIO
#include "tim.h"    // 定时器
#include "usart.h"  // 串口
// #include "i2c.h"    // I2C
// #include "adc.h"    // ADC
// #include "dma.h"    // dma

// 2. 本地底层通用
#include "MyGPIO.h"
#include "MyPWM.h"
#include "MyEncoder.h"
#include "MyTimer.h"

// 3. 芯片参数
#define MySystem_Fre 240000000

#endif // !__MYSYSTEM_H
