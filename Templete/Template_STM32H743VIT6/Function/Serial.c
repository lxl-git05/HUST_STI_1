#include "Serial.h"
#include "string.h"
#include <stdarg.h>
#include <stdio.h>
#include "Serial_base.h"

#ifdef Serial1_Enable
Serial_Typedef 	Serial1 ; // 串口1
#endif
#ifdef Serial2_Enable
Serial_Typedef 	Serial2 ; // 串口2
#endif
#ifdef Serial3_Enable
Serial_Typedef 	Serial3 ; // 串口3
#endif

#ifdef Serial_Debug
int Serial_check[40] ;
int Serial_Count = 0 ;
#endif

// ============== 全局变量 ==============

// 串口接收变量
#define RX_USART1_LEN 50
uint8_t RX_USART1[RX_USART1_LEN] ;	// 接收数组

// 串口初始化:深层
void Serial_Initial(Serial_Typedef *pSerial , USART_TypeDef* USART , UART_HandleTypeDef* huart)
{
	// 串口的USART初始化
	pSerial->huart = huart ;
	pSerial->USART = USART ;
	// 串口的数据链初始化
	pSerial->rxCnt = 0 ;
	pSerial->rx_temp = 0 ;
	memset(pSerial->rxBuf, 0, RX_Serial_LEN);	// 数据缓存区清零
	
	// 打开空闲中断函数,不再是原本的激发中断
	HAL_UARTEx_ReceiveToIdle_IT(huart, pSerial->rxBuf, RX_Serial_LEN);
  
	// 初始化串口协议
	Serial_Agreement_HEX_Init(&Serial_Agreement_HEX) ;
	Serial_Agreement_ABC_Init(&Serial_Agreement_ABC) ;
	// HEX和文本变量都是0,不需要初始化
}

// 串口初始化:外部调用
void Serial_Init(void)
{
	#ifdef Serial1_Enable
	Serial_Initial(&Serial1 , USART1 , &huart1 ) ;	// 串口协议初始化
	#endif
	#ifdef Serial2_Enable
	Serial_Initial(&Serial2 , USART2 , &huart2 ) ;	// 串口协议初始化
	#endif
	#ifdef Serial3_Enable
	Serial_Initial(&Serial3 , USART3 , &huart3 ) ;	// 串口协议初始化
	#endif
}

// 从高8位和低8位合成一个数据
uint16_t Merge_2Bytes(uint8_t high, uint8_t low)
{
    return ((uint16_t)high << 8) | low;
}

// ====================HEX:初步处理数据包(仅合并数据)====================
void Serial_Data_Deal_HEX(Serial_Typedef* pSerial)
{
	// 1. 第2个数据为数据长度(第0,1个为帧头),由于是高低位,所以除以2才是真正的数据长度
	pSerial->Hex_Data.Serial_New_Package[0] = pSerial->rxBuf[2] / 2;
	// 2. 存入数据
	for (int i = 3 , j = 1 ; i < 3 + pSerial->rxBuf[2] ; i += 2 , j ++)
	{
		pSerial->Hex_Data.Serial_New_Package[j] = Merge_2Bytes(pSerial->rxBuf[i] , pSerial->rxBuf[i + 1] ) ;
	}
}	

// HEX:数据检测+存储处理函数
void Serial_Data_Check_HEX(Serial_Typedef* pSerial)
{
	// 1. 检测帧头合规性
	if (pSerial->rxBuf[0] != Serial_Agreement_HEX.head1 || pSerial->rxBuf[1] != Serial_Agreement_HEX.head2)
	{
		// 帧头不合规
		pSerial->Hex_Data.error_Serial = Serial_Error_Head;
		// 操作:
		// 清空正式数组,后续调用会显示空数据
		memset(pSerial->Hex_Data.Serial_New_Package, 0, sizeof(pSerial->Hex_Data.Serial_New_Package));
		// 存储标志位置0
		pSerial->Hex_Data.Serial_New_Package_Flag = 0 ;
	}
	// 2. 借助数据长度检测帧尾合规性
	else if (pSerial->rxBuf[pSerial->rxBuf[2] + 3] != Serial_Agreement_HEX.end1 || pSerial->rxBuf[pSerial->rxBuf[2] + 4] != Serial_Agreement_HEX.end2)
	{
		// 帧尾不合规
		pSerial->Hex_Data.error_Serial = Serial_Error_Tail ;
		// 操作:
		// 清空正式数组,后续调用会显示空数据
		memset(pSerial->Hex_Data.Serial_New_Package, 0, sizeof(pSerial->Hex_Data.Serial_New_Package));
		// 存储标志位置0
		pSerial->Hex_Data.Serial_New_Package_Flag = 0 ;
	}
	else
	{
		// 初步处理数据包(仅合并数据)
		Serial_Data_Deal_HEX(pSerial) ;
		// 无错误
		pSerial->Hex_Data.error_Serial = Serial_Error_None ;
		// 存储标志位置1
		pSerial->Hex_Data.Serial_New_Package_Flag = 1 ;
	}
}
// HEX:得到串口接收标志位
uint8_t Serial_GetNewPackageFlag_HEX(Serial_Typedef *pSerial)
{
	if (pSerial->Hex_Data.Serial_New_Package_Flag == 1)			//如果标志位为1
	{
		pSerial->Hex_Data.Serial_New_Package_Flag = 0;
		return 1;					//则返回1，并自动清零标志位
	}
	return 0;						//如果标志位为0，则返回0
}

// *HEX:得到错误原因*
int Serial_GetError_HEX(Serial_Typedef *pSerial)
{
	return pSerial->Hex_Data.error_Serial	 ;
}



// ====================文本:数据检测+存储+数据处理(仅保留文本)处理函数====================
void Serial_Data_Check_ABC(Serial_Typedef *pSerial)
{
	// 开始检测文本
	// 1. 检测数据包帧头是否错误
	if (pSerial->rxBuf[0] != Serial_Agreement_ABC.head)
	{
		pSerial->ABC_Data.error_Serial = Serial_Error_Head ; // *错误*:帧头不合规
	}
	// 2. 开始一边处理数据一边检测帧尾,从1开始(跳过帧头)
	else
	{
		int i = 0 ;
		for (i = 1 ; pSerial->rxBuf[i+1] != Serial_Agreement_ABC.end1 ; i++)
		{
			pSerial->ABC_Data.Serial_New_Package_ABC[i-1] = pSerial->rxBuf[i] ;
			// 检测是否溢出
			if (i > Serial_Wait_Tail_MAX)
				break ;
		}
		// 补1位!!!,毕竟无论如何都是被迫离开for,少了1位
		pSerial->ABC_Data.Serial_New_Package_ABC[i-1] = pSerial->rxBuf[i] ;	
		// 情况1:退出for是因为检测到了帧尾
		if (pSerial->rxBuf[i+1] == Serial_Agreement_ABC.end1)
		{
			// 3. 开始检测第2个帧尾
			if (pSerial->rxBuf[i+2] != Serial_Agreement_ABC.end2)
			{
				pSerial->ABC_Data.error_Serial = Serial_Error_Tail ; // 错误3:第2个帧尾不合规
				memset(pSerial->ABC_Data.Serial_New_Package_ABC, 0, sizeof(pSerial->ABC_Data.Serial_New_Package_ABC));	// 清空记录数据
				return ;
			}
			else
			{
				// 初步处理数据包(仅合并数据)
				pSerial->ABC_Data.Serial_New_Package_ABC[i] = '\0' ;	// 加个结尾符号
				// 无错误
				pSerial->ABC_Data.error_Serial = 0 ; 
				// 存储标志位置1
				pSerial->ABC_Data.Serial_New_Package_Flag = 1 ;
			}
		}
		// 情况2:退出for是因为溢出了,说明第1个帧尾没有检测到
		else
		{
			pSerial->ABC_Data.error_Serial = Serial_Error_Tail ; // 错误2:第1个帧尾不合规
			memset(pSerial->ABC_Data.Serial_New_Package_ABC, 0, sizeof(pSerial->ABC_Data.Serial_New_Package_ABC));	// 清空记录数据
			return ;
		}
	}
}

// 文本:得到串口接收标志位
uint8_t Serial_GetNewPackageFlag_ABC(Serial_Typedef *pSerial)
{
	if (pSerial->ABC_Data.Serial_New_Package_Flag == 1)			//如果标志位为1
	{
		pSerial->ABC_Data.Serial_New_Package_Flag = 0;
		return 1;					//则返回1，并自动清零标志位
	}
	return 0;						//如果标志位为0，则返回0
}

// 文本:得到错误原因
int Serial_GetError_ABC(Serial_Typedef *pSerial)
{
	return pSerial->ABC_Data.error_Serial ;
}

// 文本:1. 封装一个函数,实现简易浮点数变量调试
bool Serial_SetFloatData( Serial_Typedef *pSerial , char *KeyWord , char *cmd , float *Data)
{
	// KeyWord为关键词,有别与别的指令 cmd为整句话,包含%f等,VOFA怎么写这里也怎么写 Data为接收改变量的变量
	// 浮点数加上%f就行,位数不用管,空格也需要注意,具体协议还得看VOFA怎么输出的
	// 例:	建议VOFA发送:	@Kp=%.2f$#	串口接收:	Serial_SetFloatData("Kp" , "Kp=%f" , &test1) ;
	if ( strstr(pSerial->ABC_Data.Serial_New_Package_ABC , KeyWord) != NULL )
	{
		sscanf(pSerial->ABC_Data.Serial_New_Package_ABC, cmd , Data);
		return true ;
	}
	else
	{
		return false ;
	}
}

// 文本:2. 封装一个函数,实现简易整数变量调试
bool Serial_SetIntData( Serial_Typedef *pSerial , char *KeyWord , char *cmd , int *Data)
{
	// KeyWord为关键词,有别与别的指令 cmd为整句话,包含%d等,VOFA没有%d,所以VOFA写%.0f即可代表%d Data为接收改变量的变量
	// 整数加上%d即可,空格也需要注意,具体协议还得看VOFA怎么输出的
	// 例:	建议VOFA发送:	@test=%.0f$#	串口接收:	Serial_SetIntData("test" , "test=%d" , &check1) ;
	if ( strstr(pSerial->ABC_Data.Serial_New_Package_ABC , KeyWord) != NULL )
	{
		sscanf(pSerial->ABC_Data.Serial_New_Package_ABC, cmd , Data);
		return true ;
	}
	else
	{
		return false ;
	}
}

// 文本:3. 判断指令
bool Serial_Check_Str(Serial_Typedef *pSerial , char *KeyWord)
{
    // 只检测是否包含指定关键词，不做数据解析
    // KeyWord：要匹配的指令关键字
    // 返回 true 表示检测到该指令

    if (strstr(pSerial->ABC_Data.Serial_New_Package_ABC, KeyWord) != NULL)
    {
        return true;
    }
    else
    {
        return false;
    }
}

// ============== 串口空闲中断回调函数 ==============
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
#ifdef Serial1_Enable
    if (huart->Instance == Serial1.USART)
    {
        // Size 就是本次实际接收到的字节数（Idle 触发或缓冲区满）
        if (Size > 0 && Size <= RX_Serial_LEN)
        {
            Serial1.rxBuf[Size] = '\0';                    // 加字符串结束符（对 ABC 协议有用）

            // === 数据处理（你的原有逻辑基本不动）===
            if (Serial1.rxBuf[0] == 0xFF && Serial1.rxBuf[1] == 0xAA)
            {
                Serial_Data_Check_HEX(&Serial1);
            }
            else if (Serial1.rxBuf[0] == '@')
            {
                Serial_Data_Check_ABC(&Serial1);
            }

            // 回显
//            HAL_UART_Transmit(&huart1, (uint8_t *)Serial1.rxBuf, Size, 100);     // 阻塞发送测试
        }

        // === 必须重新启动接收 ===
        HAL_UARTEx_ReceiveToIdle_IT(Serial1.huart, Serial1.rxBuf, RX_Serial_LEN);
    }
#endif
#ifdef Serial2_Enable
    if (huart->Instance == Serial2.USART)
    {
        // Size 就是本次实际接收到的字节数（Idle 触发或缓冲区满）
        if (Size > 0 && Size <= RX_Serial_LEN)
        {
            Serial2.rxBuf[Size] = '\0';                    // 加字符串结束符（对 ABC 协议有用）

            // === 数据处理（原有逻辑基本不动）===
            if (Serial2.rxBuf[0] == 0xFF && Serial2.rxBuf[1] == 0xAA)
            {
                Serial_Data_Check_HEX(&Serial2);
            }
            else if (Serial2.rxBuf[0] == '@')
            {
                Serial_Data_Check_ABC(&Serial2);
            }

            // 回显
//            HAL_UART_Transmit(&huart1, (uint8_t *)Serial2.rxBuf, Size, 100);     // 阻塞发送测试
        }

        // === 必须重新启动接收 ===
        HAL_UARTEx_ReceiveToIdle_IT(Serial2.huart, Serial2.rxBuf, RX_Serial_LEN);
    }
#endif
#ifdef Serial3_Enable
    if (huart->Instance == Serial3.USART)
    {
        // Size 就是本次实际接收到的字节数（Idle 触发或缓冲区满）
        if (Size > 0 && Size <= RX_Serial_LEN)
        {
            Serial3.rxBuf[Size] = '\0';                    // 加字符串结束符（对 ABC 协议有用）

            // === 数据处理（你的原有逻辑基本不动）===
            if (Serial3.rxBuf[0] == 0xFF && Serial3.rxBuf[1] == 0xAA)
            {
                Serial_Data_Check_HEX(&Serial3);
            }
            else if (Serial3.rxBuf[0] == '@')
            {
                Serial_Data_Check_ABC(&Serial3);
            }

            // 回显
//            HAL_UART_Transmit(&huart1, (uint8_t *)Serial3.rxBuf, Size, 100);     // 阻塞发送测试
        }

        // === 必须重新启动接收 ===
        HAL_UARTEx_ReceiveToIdle_IT(Serial3.huart, Serial3.rxBuf, RX_Serial_LEN);
    }
#endif
}

// ======================= Serial_printf 使用阻塞发送（最简单稳定） =====================
void Serial_printf(Serial_Typedef *pSerial, const char *fmt, ...)
{
    char buffer[256];
    va_list args;
    int len;

    va_start(args, fmt);
    len = vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    if (len > 0)
    {
        if (len >= (int)sizeof(buffer))
            len = sizeof(buffer) - 1;

        HAL_UART_Transmit(pSerial->huart, (uint8_t*)buffer, len, 100);  // 100ms 超时
    }
}
