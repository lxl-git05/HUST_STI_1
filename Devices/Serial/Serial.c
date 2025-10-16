#include "Serial.h"
#include "usart.h"

#include "string.h"
#include <stdarg.h>
#include <stdio.h>

// ********** 变量 **********
Serial_RX_FLAG_Typedef 		Serial_Rx_State;							// 数据接收情况标志位-枚举
Serial_RX_Data_TypeDef 		Serial_Rx_Data ;							// 数据接收缓存区

Serial_Agreement_HEX_TypeDef 	Serial_Agreement_HEX ;		// 串口数据通信协议:HEX
Serial_Agreement_ABC_TypeDef 	Serial_Agreement_ABC ;		// 串口数据通信协议:ABC

Serial_HEX_Data_Typedef   Serial_Hex_Data ;							// *解析好的HEX数据包*
Serial_ABC_Data_Typedef   Serial_ABC_Data ;							// *解析好的ABC数据包*

#ifdef Serial_Debug
int Serial_check[200] ;
int Serial_Count = 0 ;
#endif

// ********** 函数 **********
// 串口接收缓存区初始化
void Serial_Rx_Data_Init(Serial_RX_Data_TypeDef *pSerial_Rx_Data)
{
	pSerial_Rx_Data->rxCnt = 0 ;											// 计数器清零
	pSerial_Rx_Data->rx_temp = 0 ;										// 暂存数据清零
	memset(pSerial_Rx_Data->rxBuf, 0, RX_Serial_LEN);	// 数据缓存区清零
}

// 串口协议初始化:HEX
void Serial_Agreement_HEX_Init(Serial_Agreement_HEX_TypeDef *pSerial_Agreement)
{
	pSerial_Agreement->head1 = 0xFF ;
	
	pSerial_Agreement->head2 = 0xAA ;
	pSerial_Agreement->end1	 = 0x55 ;
	
	pSerial_Agreement->end2  = 0xFE	;
}

// 串口协议初始化:ABC
void Serial_Agreement_ABC_Init(Serial_Agreement_ABC_TypeDef *pSerial_Agreement)
{
	pSerial_Agreement->head  =  '@' ;
	pSerial_Agreement->end1	 =  '$' ;
	pSerial_Agreement->end2  =  '#' ;
}

// *串口初始化*
void Serial_Init(UART_HandleTypeDef *huart)
{
	// 接收缓存区初始化
	Serial_Rx_Data_Init(&Serial_Rx_Data) ;
	// 打开DMA接收函数
	HAL_UART_Receive_DMA(huart, &Serial_Rx_Data.rx_temp , 1);   
	// 初始化串口协议
	Serial_Agreement_HEX_Init(&Serial_Agreement_HEX) ;
	Serial_Agreement_ABC_Init(&Serial_Agreement_ABC) ;
	// HEX和文本变量都是0,不需要初始化
}

// *串口发送数组*
void Serial_SendData_DMA(uint8_t *pData, uint16_t Size)
{
	// 确保DMA空闲（防止两次同时发）
	if (Serial_huart.gState == HAL_UART_STATE_READY)
	{
			HAL_UART_Transmit_DMA(&Serial_huart, pData, Size);
	}
}

// 从高8位和低8位合成一个数据
uint16_t Merge_2Bytes(uint8_t high, uint8_t low)
{
    return ((uint16_t)high << 8) | low;
}

// 串口接收数据函数---将数据收集后触发Serial_Rx_Flag的OK
Serial_RX_FLAG_Typedef Serial_Rx_State_Check(void)
{
	// 将暂存数据计入缓冲区,防止丢失
	int rxData = Serial_Rx_Data.rx_temp ;

	// 状态机
	static int Serial_RxState = 0 ;
	// 空闲状态,等待帧头
	if (Serial_RxState == 0)
	{
		// 操作:数据记录回到原点
		Serial_Rx_Data.rxCnt = 0 ;
		
		// 任务:等待帧头-HEX模式
		if ( rxData == Serial_Agreement_HEX.head1 )
		{
			Serial_Rx_Data.rxBuf[Serial_Rx_Data.rxCnt++] = rxData ;
			Serial_RxState = 1 ;	// 判断HEX帧尾
			return RX_BUSY	;			// 开始处理数据
		}
		else if ( rxData == Serial_Agreement_ABC.head )
		{
			Serial_Rx_Data.rxBuf[Serial_Rx_Data.rxCnt++] = rxData ;
			Serial_RxState = 2 ;	// 判断ABC帧尾
			return RX_BUSY	;			// 开始处理数据
		}
		else
		{
			return RX_WAIT ;	// 继续等待
		}
	}
	// 开始接收HEX原始数据包
	else if (Serial_RxState == 1)
	{
		// 操作:暂存数据转移到缓冲区
		Serial_Rx_Data.rxBuf[Serial_Rx_Data.rxCnt++] = rxData ;
		
		// 任务:检测帧尾
		// 检测到帧尾,接收完毕
		if (rxData == Serial_Agreement_HEX.end2)
		{
			Serial_RxState = 0 ;	// 状态转移
			return RX_OK_HEX ;
		}
		// 没能检测到帧尾,数据溢出
		else if (Serial_Rx_Data.rxCnt >= Serial_Wait_Tail_MAX)
		{
			Serial_RxState = 0 ;	// 状态转移
			memset(Serial_Rx_Data.rxBuf, 0, RX_Serial_LEN);	// 清空
			Serial_Hex_Data.error_Serial = Serial_Error_Tail ;
			return RX_Error_Tail_HEX ;
		}
	}
	// 开始接收ABC原始数据包
	else if (Serial_RxState == 2)
	{
		// 操作:暂存数据转移到缓冲区
		Serial_Rx_Data.rxBuf[Serial_Rx_Data.rxCnt++] = rxData ;
		
		// 任务:检测帧尾
		// 检测到帧尾,接收完毕
		if (rxData == Serial_Agreement_ABC.end2)
		{
			Serial_RxState = 0 ;	// 状态转移
			return RX_OK_ABC ;
		}
		// 没能检测到帧尾,数据溢出
		else if (Serial_Rx_Data.rxCnt >= Serial_Wait_Tail_MAX)
		{
			Serial_RxState = 0 ;	// 状态转移
			memset(Serial_Rx_Data.rxBuf, 0, RX_Serial_LEN);	// 清空
			Serial_ABC_Data.error_Serial = Serial_Error_Tail ;
			return RX_Error_Tail_ABC ;
		}
	}
	
	return RX_BUSY  ;
}
// HEX:初步处理数据包(仅合并数据)
void Serial_Data_Deal_HEX(void)
{
	// 1. 第2个数据为数据长度(第0,1个为帧头),由于是高低位,所以除以2才是真正的数据长度
	Serial_Hex_Data.Serial_New_Package[0] = Serial_Rx_Data.rxBuf[2] / 2;
	// 2. 存入数据
	for (int i = 3 , j = 1 ; i < 3 + Serial_Rx_Data.rxBuf[2] ; i += 2 , j ++)
	{
		Serial_Hex_Data.Serial_New_Package[j] = Merge_2Bytes(Serial_Rx_Data.rxBuf[i] , Serial_Rx_Data.rxBuf[i + 1] ) ;
	}
}	

// HEX:数据检测+存储处理函数
void Serial_Data_Check_HEX(void)
{
	// 1. 检测帧头合规性
	if (Serial_Rx_Data.rxBuf[0] != Serial_Agreement_HEX.head1 || Serial_Rx_Data.rxBuf[1] != Serial_Agreement_HEX.head2)
	{
		// 帧头不合规
		Serial_Hex_Data.error_Serial = Serial_Error_Head;
		// 操作:
		// 清空正式数组,后续调用会显示空数据
		memset(Serial_Hex_Data.Serial_New_Package, 0, sizeof(Serial_Hex_Data.Serial_New_Package));
		// 存储标志位置0
		Serial_Hex_Data.Serial_New_Package_Flag = 0 ;
	}
	// 2. 借助数据长度检测帧尾合规性
	else if (Serial_Rx_Data.rxBuf[Serial_Rx_Data.rxBuf[2] + 3] != Serial_Agreement_HEX.end1 || Serial_Rx_Data.rxBuf[Serial_Rx_Data.rxBuf[2] + 4] != Serial_Agreement_HEX.end2)
	{
		// 帧尾不合规
		Serial_Hex_Data.error_Serial = Serial_Error_Tail ;
		// 操作:
		// 清空正式数组,后续调用会显示空数据
		memset(Serial_Hex_Data.Serial_New_Package, 0, sizeof(Serial_Hex_Data.Serial_New_Package));
		// 存储标志位置0
		Serial_Hex_Data.Serial_New_Package_Flag = 0 ;
	}
	else
	{
		// 初步处理数据包(仅合并数据)
		Serial_Data_Deal_HEX() ;
		// 无错误
		Serial_Hex_Data.error_Serial = Serial_Error_None ;
		// 存储标志位置1
		Serial_Hex_Data.Serial_New_Package_Flag = 1 ;
	}
}
// *HEX:得到串口接收标志位*
uint8_t Serial_GetNewPackageFlag_HEX(void)
{
	if (Serial_Hex_Data.Serial_New_Package_Flag == 1)			//如果标志位为1
	{
		Serial_Hex_Data.Serial_New_Package_Flag = 0;
		return 1;					//则返回1，并自动清零标志位
	}
	return 0;						//如果标志位为0，则返回0
}

// *HEX:得到错误原因*
int Serial_GetError_HEX(void)
{
	return Serial_Hex_Data.error_Serial	 ;
}

// 文本:数据检测+存储+数据处理(仅保留文本)处理函数
void Serial_Data_Check_ABC(void)
{
	// 开始检测文本
	// 1. 检测数据包帧头是否错误
	if (Serial_Rx_Data.rxBuf[0] != Serial_Agreement_ABC.head)
	{
		Serial_ABC_Data.error_Serial = Serial_Error_Head ; // *错误*:帧头不合规
	}
	// 2. 开始一边处理数据一边检测帧尾,从1开始(跳过帧头)
	else
	{
		int i = 0 ;
		for (i = 1 ; Serial_Rx_Data.rxBuf[i+1] != Serial_Agreement_ABC.end1 ; i++)
		{
			Serial_ABC_Data.Serial_New_Package_ABC[i-1] = Serial_Rx_Data.rxBuf[i] ;
			// 检测是否溢出
			if (i > Serial_Wait_Tail_MAX)
				break ;
		}
		// 补1位!!!,毕竟无论如何都是被迫离开for,少了1位
		Serial_ABC_Data.Serial_New_Package_ABC[i-1] = Serial_Rx_Data.rxBuf[i] ;	
		// 情况1:退出for是因为检测到了帧尾
		if (Serial_Rx_Data.rxBuf[i+1] == Serial_Agreement_ABC.end1)
		{
			// 3. 开始检测第2个帧尾
			if (Serial_Rx_Data.rxBuf[i+2] != Serial_Agreement_ABC.end2)
			{
				Serial_ABC_Data.error_Serial = Serial_Error_Tail ; // 错误3:第2个帧尾不合规
				memset(Serial_ABC_Data.Serial_New_Package_ABC, 0, sizeof(Serial_ABC_Data.Serial_New_Package_ABC));	// 清空记录数据
				return ;
			}
			else
			{
				// 初步处理数据包(仅合并数据)
				Serial_ABC_Data.Serial_New_Package_ABC[i] = '\0' ;	// 加个结尾符号
				// 无错误
				Serial_ABC_Data.error_Serial = 0 ; 
				// 存储标志位置1
				Serial_ABC_Data.Serial_New_Package_Flag = 1 ;
			}
		}
		// 情况2:退出for是因为溢出了,说明第1个帧尾没有检测到
		else
		{
			Serial_ABC_Data.error_Serial = Serial_Error_Tail ; // 错误2:第1个帧尾不合规
			memset(Serial_ABC_Data.Serial_New_Package_ABC, 0, sizeof(Serial_ABC_Data.Serial_New_Package_ABC));	// 清空记录数据
			return ;
		}
	}
}

// *文本:得到串口接收标志位*
uint8_t Serial_GetNewPackageFlag_ABC(void)
{
	if (Serial_ABC_Data.Serial_New_Package_Flag == 1)			//如果标志位为1
	{
		Serial_ABC_Data.Serial_New_Package_Flag = 0;
		return 1;					//则返回1，并自动清零标志位
	}
	return 0;						//如果标志位为0，则返回0
}

// *文本:得到错误原因*
int Serial_GetError_ABC(void)
{
	return Serial_ABC_Data.error_Serial ;
}

// *文本:1. 封装一个函数,实现简易浮点数变量调试*
bool Serial_SetFloatData( char *KeyWord , char *cmd , float *Data)
{
	// KeyWord为关键词,有别与别的指令 cmd为整句话,包含%f等,VOFA怎么写这里也怎么写 Data为接收改变量的变量
	// 浮点数加上%f就行,位数不用管,空格也需要注意,具体协议还得看VOFA怎么输出的
	// 例:	建议VOFA发送:	@Kp=%.2f$#	串口接收:	Serial_SetFloatData("Kp" , "Kp=%f" , &test1) ;
	if ( strstr(Serial_ABC_Data.Serial_New_Package_ABC , KeyWord) != NULL )
	{
		sscanf(Serial_ABC_Data.Serial_New_Package_ABC, cmd , Data);
		return true ;
	}
	else
	{
		return false ;
	}
}

// *文本:2. 封装一个函数,实现简易整数变量调试*
bool Serial_SetIntData( char *KeyWord , char *cmd , int *Data)
{
	// KeyWord为关键词,有别与别的指令 cmd为整句话,包含%d等,VOFA没有%d,所以VOFA写%.0f即可代表%d Data为接收改变量的变量
	// 整数加上%d即可,空格也需要注意,具体协议还得看VOFA怎么输出的
	// 例:	建议VOFA发送:	@test=%.0f$#	串口接收:	Serial_SetIntData("test" , "test=%d" , &check1) ;
	if ( strstr(Serial_ABC_Data.Serial_New_Package_ABC , KeyWord) != NULL )
	{
		sscanf(Serial_ABC_Data.Serial_New_Package_ABC, cmd , Data);
		return true ;
	}
	else
	{
		return false ;
	}
}

// 串口空闲中断回调函数
/*串口接收中断回调*/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == Serial_USART)
	{
		#ifdef Serial_Debug
		Serial_check[Serial_Count++] = Serial_Rx_Data.rx_temp ;	// 得到所有接收到的数据
		#endif 
		
		// 获得串口数据传输状态(更新)
		Serial_Rx_State = Serial_Rx_State_Check();
		
		// HEX数据包
		if (Serial_Rx_State == RX_OK_HEX)
		{
			// 开始处理原始数据包:HEX
			Serial_Data_Check_HEX() ;
		}
		// ABC数据包
		else if (Serial_Rx_State == RX_OK_ABC)
		{
			// 开始处理原始数据包:ABC
			Serial_Data_Check_ABC() ;
		}
		
		// 重新打开串口DMA接收，DMA配置为不连续模式
		HAL_UART_Receive_DMA(huart, &Serial_Rx_Data.rx_temp , 1);   
	}
}

