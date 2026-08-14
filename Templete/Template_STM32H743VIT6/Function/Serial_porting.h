#ifndef __SERIAL_PORTING_H
#define __SERIAL_PORTING_H

#include "Serial_base.h"

// ============== 串口实例结构体 ==============
#define Serial_TX_BUF_SIZE 256                        // DMA 发送缓冲大小（超长自动回退阻塞发送）

typedef struct {
    USART_TypeDef *Instance;              // USART外设基址
    UART_HandleTypeDef *huart;          // HAL句柄

    uint8_t rxBuf[((Serial_RX_BUF_SIZE + 31) & ~31)] __attribute__((aligned(32)));    // 接收缓冲区(32字节对齐,DMA+D-Cache维护要求)
    uint8_t txBuf[Serial_TX_BUF_SIZE] __attribute__((aligned(32)));                   // DMA发送缓冲区(32字节对齐,D-Cache维护要求)
    uint16_t rxLen;                       // 本次接收数据长度

    Serial_ABC_Data_Typedef ABC_Data;     // ABC协议数据
    Serial_HEX_Data_Typedef HEX_Data;     // HEX协议数据
} Serial_Typedef;

// ============== 外部实例声明 ==============
#define Serial2_Enable 1
#define Serial3_Enable 1
#define Serial4_Enable 1

extern Serial_Typedef Serial1;
#ifdef Serial2_Enable
extern Serial_Typedef Serial2;
#endif
#ifdef Serial3_Enable
extern Serial_Typedef Serial3;
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
bool Serial_CheckCmd(Serial_Typedef *pSerial, char *cmd);          // 精确匹配（strcmp）

// ============== 发送 API ==============
void Serial_send_string(Serial_Typedef *pSerial, char *str);           // 轮询发字符串
void Serial_SendBytes(Serial_Typedef *pSerial, uint8_t *buf, uint16_t len);
void Serial_Send_HEX_Package(Serial_Typedef *pSerial, uint16_t *data, uint8_t count);

// ============== 工具 API ==============
void Serial_Clear_ABC(Serial_Typedef *pSerial);                       // 清空 ABC 接收缓冲区
void Serial_PrintDebug(Serial_Typedef *pSerial);                       // 打印调试统计

#endif // !__SERIAL_PORTING_H
