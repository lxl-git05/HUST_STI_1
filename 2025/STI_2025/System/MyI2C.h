#ifndef __MYI2C_H
#define __MYI2C_H

#include "main.h"

// void MyI2C_Init(void) ; // HAL库情形下不加入Init函数
void MyI2C_Start(void);
void MyI2C_Stop(void);
void MyI2C_SendByte(uint8_t Byte);
uint8_t MyI2C_ReceiveByte(void);
void MyI2C_SendAck(uint8_t Ackbit);
uint8_t MyI2C_ReceiveAck(void);

#endif 
