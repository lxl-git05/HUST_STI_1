#ifndef __SERIAL_H
#define __SERIAL_H

#include "Mysystem.h"
#include <stdbool.h>

// =============== define声明 ===============

//#define Serial_Debug						// Debug模式

#define Serial1_Enable						// USART1串口DMA模式开启
#define Serial2_Enable					// USART2串口DMA模式开启
//#define Serial3_Enable					// USART3串口DMA模式开启

#define RX_Serial_LEN 40					// DMA接收数组长度,一次接受的数据不能大于这个长度
#define Serial_Wait_Tail_MAX 25				// DMA等待帧尾判断溢出阈值

// =============== 结构体初始化 ===============
// HEX接收数据包
typedef struct
{
	int Serial_New_Package[RX_Serial_LEN] ; 		// 正确信息存储数组,长度管够,以后再改
	bool Serial_New_Package_Flag ;							// 数据包解析完成flag
	int error_Serial	;								  				// 错误查询参数
}Serial_HEX_Data_Typedef;

// 文本接收数据包
typedef struct
{
	char Serial_New_Package_ABC[RX_Serial_LEN] ; // 正确信息存储数组,长度管够,以后再改
	bool Serial_New_Package_Flag ;							 // 数据包解析完成flag
	int error_Serial	;								  				 // 错误查询参数
}Serial_ABC_Data_Typedef;

// 串口数据处理定义
typedef struct
{
	USART_TypeDef * USART ;				
	UART_HandleTypeDef* huart ;		// 外设必须使用指针,尤其是huart类似的结构体,否则地址会改变

	uint8_t rx_temp;							// DMA传输给temp暂存,并且很快将被保存在rxBuf中
	uint8_t rxCnt;								// Cnt记录DMA传输了多少位数据
	uint8_t rxBuf[RX_Serial_LEN];	// 接收缓冲区,接收temp数据
	
	uint8_t Status ;							// 串口数据接收状态机
	
	Serial_HEX_Data_Typedef Hex_Data ;	// 16进制数据包
	Serial_ABC_Data_Typedef ABC_Data ;	// 字符串数据包
}Serial_Typedef ;

// =============== 外部变量声明 ===============
#ifdef Serial1_Enable
extern Serial_Typedef 		 Serial1 ; 		// 串口1
#endif
#ifdef Serial2_Enable
extern Serial_Typedef 		 Serial2 ; 		// 串口2
#endif
#ifdef Serial3_Enable
extern Serial_Typedef 		 Serial3 ; 		// 串口3
#endif


// =============== 函数声明 ===============
// 串口接收初始化
void Serial_Init(void) ;

// HEX:得到串口接收标志位
uint8_t Serial_GetNewPackageFlag_HEX(Serial_Typedef *pSerial) ;

// HEX:得到错误原因
int Serial_GetError_HEX(Serial_Typedef *pSerial) ;


// 文本:得到串口接收标志位
uint8_t Serial_GetNewPackageFlag_ABC(Serial_Typedef *pSerial) ;

// 文本:得到错误原因
int Serial_GetError_ABC(Serial_Typedef *pSerial) ;

// 文本:1. 封装一个函数,实现简易浮点数变量调试
bool Serial_SetFloatData( Serial_Typedef *pSerial , char *KeyWord , char *cmd , float *Data) ;

// 文本:2. 封装一个函数,实现简易整数变量调试
bool Serial_SetIntData( Serial_Typedef *pSerial , char *KeyWord , char *cmd , int *Data) ;

// 文本:3. 判断指令
bool Serial_Check_Str(Serial_Typedef *pSerial , char *KeyWord) ;

// 打印数据,记得加减乘除都要在后方进行而不是""里面进行
void Serial_printf(Serial_Typedef *pSerial, const char *fmt, ...) ;	// 972us -> 约等于1ms

#endif
