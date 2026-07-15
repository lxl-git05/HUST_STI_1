#ifndef __SERIAL_PORTING_H
#define __SERIAL_PORTING_H

#include "Serial_base.h"

// ============== 串口实例结构体 ==============
typedef struct {
    USART_TypeDef *Instance;              // USART外设基址
    UART_HandleTypeDef *huart;          // HAL句柄

    uint8_t rxBuf[Serial_RX_BUF_SIZE];    // 接收缓冲区
    uint16_t rxLen;                       // 本次接收数据长度

    Serial_ABC_Data_Typedef ABC_Data;     // ABC协议数据
    Serial_HEX_Data_Typedef HEX_Data;     // HEX协议数据
} Serial_Typedef;

// ============== 外部实例声明 ==============
#define Serial2_Enable 1
#define Serial4_Enable 1

extern Serial_Typedef Serial1;
#ifdef Serial2_Enable
extern Serial_Typedef Serial2;
#endif
#ifdef Serial4_Enable
extern Serial_Typedef Serial4;
#endif

// ============== 初始化/发送 ==============
void Serial_Init(void);
void Serial_printf(Serial_Typedef *pSerial, const char *fmt, ...);

// ============== HEX协议 ==============
int16_t Serial_GetHexData(Serial_Typedef *pSerial, uint8_t index);           // 获取第index个字
uint8_t Serial_GetHexLen(Serial_Typedef *pSerial);                          // 获取实际字数
uint8_t Serial_GetNewPackageFlag_HEX(Serial_Typedef *pSerial);              // 获取新包标志
int Serial_GetError_HEX(Serial_Typedef *pSerial);                           // 获取HEX错误码

// ============== ABC协议 ==============
uint8_t Serial_GetNewPackageFlag_ABC(Serial_Typedef *pSerial);
int Serial_GetError_ABC(Serial_Typedef *pSerial);                           // 获取ABC错误码
bool Serial_SetFloatData(Serial_Typedef *pSerial, char *KeyWord, char *cmd, float *Data);
bool Serial_SetIntData(Serial_Typedef *pSerial, char *KeyWord, char *cmd, int *Data);
bool Serial_Check_Str(Serial_Typedef *pSerial, char *KeyWord);

#endif // !__SERIAL_PORTING_H
