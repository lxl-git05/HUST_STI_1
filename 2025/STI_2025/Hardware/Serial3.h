#ifndef __Serial3_H
#define __Serial3_H

#include "main.h"
#include <stdbool.h>

// 一些测试模式
//#define Serial3_Debug
//#define Serial3_VOFA

// 串口通信通道(代码迁移时修改这里即可)
#define Serial3_huart huart3
#define Serial3_USART USART3

// DMA接收数组长度,一次接受的数据不能大于这个长度
#define RX_Serial3_LEN 50
// DMA等待帧尾判断溢出阈值
#define Serial3_Wait_Tail_MAX 25

// 通过检验的数据包的长度(肯定比DMA的小,根据情况处理)


// ********** 结构体初始化 **********
// 数据接收过程标志位
typedef enum
{
	// 数据初步存入数据缓冲区阶段
	RX3_OK_HEX 		= 0x00U,	// HEX数据包接收完成
	RX3_OK_ABC 		= 0x01U,	// ABC数据包接收完成
	
	RX3_BUSY		= 0x02U,	// 数据包正在接收存储中,跳过此次解析过程
	RX3_WAIT		= 0x03U,	// 等待数据传入(如果头帧不通过就一直卡在这里)
	
	RX3_Error_Tail_HEX = 0x6U,		// 数据尾帧出错,导致数据溢出
	RX3_Error_Tail_ABC = 0x7U,		// 数据尾帧出错,导致数据溢出

}Serial3_RX_FLAG_Typedef;
 
// 数据包检测错误处理
typedef enum
{
	Serial3_Error_None = 0x00U,		// 数据无误
	Serial3_Error_Head = 0x01U,		// 数据头帧出错
	Serial3_Error_Tail = 0x02U,		// 数据尾帧出错
	Serial3_Error_Data = 0x03U,		
	Serial3_Error_Data_Len = 0x04U,

}Serial3_Data_Error_Typedef;

// 串口数据接收缓冲区(以前是使用数组,现在使用结构体进行接收)
typedef struct
{
	uint8_t rx_temp;							// DMA传输给temp暂存,并且很快将被保存在rxBuf中
	uint8_t rxCnt;								// Cnt记录DMA传输了多少位数据
	uint8_t rxBuf[RX_Serial3_LEN];	// 接收缓冲区,接收temp数据
}Serial3_RX_Data_TypeDef;
 
// 串口协议:HEX
typedef struct
{
	uint8_t head1;	// 头帧1
	uint8_t head2;	// 头帧2
	uint8_t end1;		// 尾帧1
	uint8_t end2;		// 尾帧2
}Serial3_Agreement_HEX_TypeDef;

// 串口协议:ABC
typedef struct
{
	uint8_t head;	  // 头帧
	uint8_t end1;		// 尾帧1
	uint8_t end2;		// 尾帧2
}Serial3_Agreement_ABC_TypeDef;

// HEX接收数据包
typedef struct
{
	int Serial3_New_Package[RX_Serial3_LEN] ; 		// 正确信息存储数组,长度管够,以后再改
	bool Serial3_New_Package_Flag ;							// 数据包解析完成flag
	int error_Serial3	;								  				// 错误查询参数
}Serial3_HEX_Data_Typedef;

// 文本接收数据包
typedef struct
{
	char Serial3_New_Package_ABC[RX_Serial3_LEN] ; // 正确信息存储数组,长度管够,以后再改
	bool Serial3_New_Package_Flag ;							 // 数据包解析完成flag
	int error_Serial3	;								  				 // 错误查询参数
}Serial3_ABC_Data_Typedef;

// ********** 变量extern **********
extern Serial3_RX_FLAG_Typedef 		Serial3_Rx_State;							// 数据接收情况标志位-枚举
extern Serial3_RX_Data_TypeDef 		Serial3_Rx_Data ;							// 数据接收缓存区

// ********** 函数声明 **********
// DMA串口接收初始化
void Serial3_Init(UART_HandleTypeDef *huart) ;

// 串口发送数组
void Serial3_SendData_DMA(uint8_t *pData, uint16_t Size) ;

// HEX:得到串口接收标志位
uint8_t Serial3_GetNewPackageFlag_HEX(void) ;

// HEX:得到错误原因
int Serial3_GetError_HEX(void) ;


// 文本:得到串口接收标志位
uint8_t Serial3_GetNewPackageFlag_ABC(void) ;

// 文本:得到错误原因
int Serial3_GetError_ABC(void) ;

// 文本:1. 封装一个函数,实现简易浮点数变量调试
bool Serial3_SetFloatData( char *KeyWord , char *cmd , float *Data) ;

// 文本:2. 封装一个函数,实现简易整数变量调试
bool Serial3_SetIntData( char *KeyWord , char *cmd , int *Data) ;

// 临时加
Serial3_RX_FLAG_Typedef Serial3_Rx_State_Check(void) ;

void Serial3_Data_Check_HEX(void) ;

void Serial3_Data_Check_ABC(void) ;

#endif

