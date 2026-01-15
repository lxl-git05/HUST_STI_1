#ifndef __SERIAL_H
#define __SERIAL_H

#include "main.h"
#include <stdbool.h>

// =============== define声明 ===============

//#define Serial_Debug					// Debug模式

#define Serial1_Enable						// USART1串口DMA模式开启
//#define Serial2_Enable						// USART2串口DMA模式开启
//#define Serial3_Enable						// USART3串口DMA模式开启

#define RX_Serial_LEN 40				// DMA接收数组长度,一次接受的数据不能大于这个长度
#define Serial_Wait_Tail_MAX 25	// DMA等待帧尾判断溢出阈值

// =============== 结构体初始化 ===============
// 数据接收过程标志位
typedef enum
{
	// 数据初步存入数据缓冲区阶段
	RX_OK_HEX 		= 0x00U,	// HEX数据包接收完成
	RX_OK_ABC 		= 0x01U,	// ABC数据包接收完成
	
	RX_BUSY		= 0x02U,	// 数据包正在接收存储中,跳过此次解析过程
	RX_WAIT		= 0x03U,	// 等待数据传入(如果头帧不通过就一直卡在这里)
	
	RX_Error_Tail_HEX = 0x6U,		// 数据尾帧出错,导致数据溢出
	RX_Error_Tail_ABC = 0x7U,		// 数据尾帧出错,导致数据溢出
}Serial_RX_FLAG_Typedef;
 
// 数据包检测错误处理
typedef enum
{
	Serial_Error_None = 0x00U,		// 数据无误
	Serial_Error_Head = 0x01U,		// 数据头帧出错
	Serial_Error_Tail = 0x02U,		// 数据尾帧出错
	Serial_Error_Data = 0x03U,		
	Serial_Error_Data_Len = 0x04U,
}Serial_Data_Error_Typedef;
// 串口协议:HEX
typedef struct
{
	uint8_t head1;	// 头帧1
	uint8_t head2;	// 头帧2
	uint8_t end1;		// 尾帧1
	uint8_t end2;		// 尾帧2
}Serial_Agreement_HEX_TypeDef;

// 串口协议:ABC
typedef struct
{
	uint8_t head;	  // 头帧
	uint8_t end1;		// 尾帧1
	uint8_t end2;		// 尾帧2
}Serial_Agreement_ABC_TypeDef;

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
// DMA串口接收初始化
void Serial_Init(void) ;

// 串口发送数组
void Serial_SendData_DMA(Serial_Typedef *pSerial , uint8_t *pData, uint16_t Size) ;

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

// 打印数据,记得加减乘除都要在后方进行而不是""里面进行
void Serial_printf(Serial_Typedef *pSerial, const char *fmt, ...) ;

#endif
