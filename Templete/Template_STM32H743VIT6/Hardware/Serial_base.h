#ifndef __SERIAL_BASE_H
#define __SERIAL_BASE_H

#include "MySystem.h"

// 串口协议类型
// 1. 数据接收过程标志位
typedef enum
{
	// 数据初步存入数据缓冲区阶段
	RX_OK_HEX 		= 0x00U,	// HEX数据包接收完成
	RX_OK_ABC 		= 0x01U,	// ABC数据包接收完成
	
	RX_BUSY		= 0x02U,	// 数据包正在接收存储中,跳过此次解析过程
	RX_WAIT		= 0x03U,	// 等待数据传入(如果头帧不通过就一直卡在这里)
	
	RX_Error_Tail_HEX = 0x6U,		// 数据尾帧出错,导致数据溢出
	RX_Error_Tail_ABC = 0x7U,		// 数据尾帧出错,导致数据溢出

	RX_Error,
}Serial_RX_FLAG_Typedef;
// 2. 数据包检测错误处理
typedef enum
{
	Serial_Error_None = 0x00U,		// 数据无误
	Serial_Error_Head = 0x01U,		// 数据头帧出错
	Serial_Error_Tail = 0x02U,		// 数据尾帧出错
	Serial_Error_Data = 0x03U,		
	Serial_Error_Data_Len = 0x04U,
}Serial_Data_Error_Typedef;
// 3. 串口协议:HEX
typedef struct
{
	uint8_t head1;	// 头帧1
	uint8_t head2;	// 头帧2
	uint8_t end1;		// 尾帧1
	uint8_t end2;		// 尾帧2
}Serial_Agreement_HEX_TypeDef;
// 4. 串口协议:ABC
typedef struct
{
	uint8_t head;	  // 头帧
	uint8_t end1;		// 尾帧1
	uint8_t end2;		// 尾帧2
}Serial_Agreement_ABC_TypeDef;

// 外部声明
extern Serial_Agreement_HEX_TypeDef 	Serial_Agreement_HEX ;		// 串口数据通信协议:HEX
extern Serial_Agreement_ABC_TypeDef 	Serial_Agreement_ABC ;		// 串口数据通信协议:ABC

// 函数
// 串口协议初始化:HEX
void Serial_Agreement_HEX_Init(Serial_Agreement_HEX_TypeDef *pSerial_Agreement) ;
// 串口协议初始化:ABC
void Serial_Agreement_ABC_Init(Serial_Agreement_ABC_TypeDef *pSerial_Agreement) ;

#endif // !__SERIAL_BASE_H
