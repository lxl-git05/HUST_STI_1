#ifndef __MYSERIAL_H
#define __MYSERIAL_H

// 定义串口数据包帧格式
typedef enum
{
	HEX_Head1 = 0xFF ,
	HEX_Head2 = 0xAA ,
	HEX_Tail1 = 0x55 ,
	HEX_Tail2 = 0xFE
}Serial_HEX_Frame_Typedef ;

// 定义串口数据状态机
typedef enum
{
	Serial_S_Init,		// 初始化
	Serial_S_Head1 ,
	Serial_S_Head2 ,
	Serial_S_Num ,
	Serial_S_Data ,
	Serial_S_Tail2 ,
	Serial_S_OK ,
	Serial_Error			// 错误处理
}Serial_HEX_Status_Typedef ;


// 定义串口数据包
typedef struct
{
	Serial_HEX_Status_Typedef Status ;	// 状态机
	
	
}Serial_HEX_Data_Typedef ;


#endif


















































































