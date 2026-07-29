#ifndef __Y8_USART_H
#define __Y8_USART_H

#include "MySystem.h"

// ============== 宏定义 ==============
#define Y8U_RX_BUF_SIZE  128    // DMA 接收缓冲区大小（足够容纳多帧）

// ============== 全局数据 ==============
extern uint16_t Y8U_ADC[8];        // 8路模拟型 ADC 数据（0~4095）
extern uint8_t  Y8U_Digital[8];    // 8路数字型数据（0/1）

// ============== API ==============
void Y8U_Init(void);               // 初始化：启动 DMA+Idle 接收，发送 $0,1,0#
void Y8U_SendCmd(uint8_t calib, uint8_t analog, uint8_t digital);  // 发送命令
uint8_t Y8U_NewData(void);         // 检查新帧标志（读后自动清零）
void Y8U_DMA_RxCallback(UART_HandleTypeDef *huart, uint16_t Size);  // DMA 空闲中断回调

#endif
