#include "Serial.h"
#include "usart.h"

#include "string.h"
#include <stdarg.h>
#include <stdio.h>

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
Serial_Agreement_HEX_TypeDef 	Serial_Agreement_HEX ;		// 串口数据通信协议:HEX
Serial_Agreement_ABC_TypeDef 	Serial_Agreement_ABC ;		// 串口数据通信协议:ABC

// ============== 函数:初始化 ==============
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
	
	// 打开DMA接收函数
	HAL_UART_Receive_DMA(huart, &(pSerial->rx_temp) , 1);  
  
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

// 串口接收数据函数---将数据收集后触发Serial_Rx_Flag的OK
Serial_RX_FLAG_Typedef Serial_Rx_State_Check(Serial_Typedef* pSerial)
{
	// 将暂存数据计入缓冲区,防止丢失
	int rxData = pSerial->rx_temp ;

	// 状态机
	// 状态1:空闲状态,等待帧头
	if (pSerial->Status == 0)
	{
		// 操作:数据记录回到原点
		pSerial->rxCnt = 0 ;
		
		// 任务:等待帧头-HEX模式
		if ( rxData == Serial_Agreement_HEX.head1 )
		{
			pSerial->rxBuf[pSerial->rxCnt++] = rxData ;
			pSerial->Status = 1 ;	// 判断HEX帧尾
			return RX_BUSY	;			// 开始处理数据
		}
		else if ( rxData == Serial_Agreement_ABC.head )
		{
			pSerial->rxBuf[pSerial->rxCnt++] = rxData ;
			pSerial->Status = 2 ;	// 判断ABC帧尾
			return RX_BUSY	;			// 开始处理数据
		}
		else
		{
			return RX_WAIT ;	// 继续等待
		}
	}
	// 开始接收HEX原始数据包
	else if (pSerial->Status == 1)
	{
		// 操作:暂存数据转移到缓冲区
		pSerial->rxBuf[pSerial->rxCnt++] = rxData ;
		
		// 任务:检测帧尾
		// 检测到帧尾,接收完毕
		if (rxData == Serial_Agreement_HEX.end2)
		{
			pSerial->Status = 0 ;	// 状态转移
			return RX_OK_HEX ;
		}
		// 没能检测到帧尾,数据溢出
		else if (pSerial->rxCnt >= Serial_Wait_Tail_MAX)
		{
			pSerial->Status = 0 ;	// 状态转移
			memset(pSerial->rxBuf, 0, RX_Serial_LEN);	// 清空
			pSerial->Hex_Data.error_Serial = Serial_Error_Tail ;
			return RX_Error_Tail_HEX ;
		}
	}
	// 开始接收ABC原始数据包
	else if (pSerial->Status == 2)
	{
		// 操作:暂存数据转移到缓冲区
		pSerial->rxBuf[pSerial->rxCnt++] = rxData ;
		
		// 任务:检测帧尾
		// 检测到帧尾,接收完毕
		if (rxData == Serial_Agreement_ABC.end2)
		{
			pSerial->Status = 0 ;	// 状态转移
			return RX_OK_ABC ;
		}
		// 没能检测到帧尾,数据溢出
		else if (pSerial->rxCnt >= Serial_Wait_Tail_MAX)
		{
			pSerial->Status = 0 ;	// 状态转移
			memset(pSerial->rxBuf, 0, RX_Serial_LEN);	// 清空
			pSerial->ABC_Data.error_Serial = Serial_Error_Tail ;
			return RX_Error_Tail_ABC ;
		}
	}
	return RX_BUSY  ;
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
// *HEX:得到串口接收标志位*
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

// *文本:得到串口接收标志位*
uint8_t Serial_GetNewPackageFlag_ABC(Serial_Typedef *pSerial)
{
	if (pSerial->ABC_Data.Serial_New_Package_Flag == 1)			//如果标志位为1
	{
		pSerial->ABC_Data.Serial_New_Package_Flag = 0;
		return 1;					//则返回1，并自动清零标志位
	}
	return 0;						//如果标志位为0，则返回0
}

// *文本:得到错误原因*
int Serial_GetError_ABC(Serial_Typedef *pSerial)
{
	return pSerial->ABC_Data.error_Serial ;
}

// *文本:1. 封装一个函数,实现简易浮点数变量调试*
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

// *文本:2. 封装一个函数,实现简易整数变量调试*
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



// ============== 串口空闲中断回调函数 ==============
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	#ifdef Serial1_Enable
	if(huart->Instance == Serial1.USART)
	{
		#ifdef Serial_Debug
		Serial_check[Serial_Count++] = Serial1.rx_temp ;	// 得到所有接收到的数据
		#endif 
		
		static Serial_RX_FLAG_Typedef Serial1_Rx_State;		// 数据接收情况标志位-枚举
		
		// 获得串口数据传输状态(更新)
		Serial1_Rx_State = Serial_Rx_State_Check(&Serial1);
		
		// HEX数据包
		if (Serial1_Rx_State == RX_OK_HEX)
		{
			// 开始处理原始数据包:HEX
			Serial_Data_Check_HEX(&Serial1) ;
		}
		// ABC数据包
		else if (Serial1_Rx_State == RX_OK_ABC)
		{
			// 开始处理原始数据包:ABC
			Serial_Data_Check_ABC(&Serial1) ;
		} 
		// 重新打开串口DMA接收，DMA配置为不连续模式
		HAL_UART_Receive_DMA(Serial1.huart, &(Serial1.rx_temp) , 1);    		
	}
	#endif
	#ifdef Serial2_Enable
	if(huart->Instance == Serial2.USART)
	{
		#ifdef Serial_Debug
		Serial_check[Serial_Count++] = Serial2.rx_temp ;	// 得到所有接收到的数据
		#endif 
		
		static Serial_RX_FLAG_Typedef Serial2_Rx_State;		// 数据接收情况标志位-枚举
		
		// 获得串口数据传输状态(更新)
		Serial2_Rx_State = Serial_Rx_State_Check(&Serial2);
		
		// HEX数据包
		if (Serial2_Rx_State == RX_OK_HEX)
		{
			// 开始处理原始数据包:HEX
			Serial_Data_Check_HEX(&Serial2) ;
		}
		// ABC数据包
		else if (Serial2_Rx_State == RX_OK_ABC)
		{
			// 开始处理原始数据包:ABC
			Serial_Data_Check_ABC(&Serial2) ;
		} 
		// 重新打开串口DMA接收，DMA配置为不连续模式
		HAL_UART_Receive_DMA(Serial2.huart, &(Serial2.rx_temp) , 1);    		
	}
	#endif
	#ifdef Serial3_Enable
		#ifdef Serial_Debug
		Serial_check[Serial_Count++] = Serial3.rx_temp ;	// 得到所有接收到的数据
		#endif 
		
		static Serial_RX_FLAG_Typedef Serial3_Rx_State;		// 数据接收情况标志位-枚举
		
		// 获得串口数据传输状态(更新)
		Serial3_Rx_State = Serial_Rx_State_Check(&Serial3);
		
		// HEX数据包
		if (Serial3_Rx_State == RX_OK_HEX)
		{
			// 开始处理原始数据包:HEX
			Serial_Data_Check_HEX(&Serial3) ;
		}
		// ABC数据包
		else if (Serial3_Rx_State == RX_OK_ABC)
		{
			// 开始处理原始数据包:ABC
			Serial_Data_Check_ABC(&Serial3) ;
		} 
		// 重新打开串口DMA接收，DMA配置为不连续模式
		HAL_UART_Receive_DMA(Serial3.huart, &(Serial3.rx_temp) , 1);    		
	}
	#endif
}


// ======================= UART_DMA_Send:替代printf方案,实现更快的速度和更小的消耗(7个%,100us) =====================
// 整数转字符串
static void int_to_str(int x, char *buf)
{
    char tmp[12];
    int i = 0, j = 0;

    if (x < 0) { buf[j++] = '-'; x = -x; }

    do {
        tmp[i++] = x % 10 + '0';
        x /= 10;
    } while (x);

    while (i--) buf[j++] = tmp[i];

    buf[j] = 0;
}

// 浮点数转字符串
static void float_to_str(float f, char *buf)
{
    int int_part = (int)f;
    float frac_part = f - int_part;
    if (frac_part < 0) frac_part = -frac_part;

    int_to_str(int_part, buf);
    int len = strlen(buf);
    buf[len++] = '.';

    int frac = (int)(frac_part * 100); // 两位小数
    if (frac < 10) buf[len++] = '0';   // 补0
    int_to_str(frac, &buf[len]);
}

// DMA发送信息
void UART_DMA_Send(Serial_Typedef *pSerial, char *str)
{
    HAL_UART_Transmit_DMA(pSerial->huart, (uint8_t*)str, strlen(str));
}

// Serial发送消息
void Serial_printf(Serial_Typedef *pSerial, const char *fmt, ...)
{
    char buffer[256];  // DMA发送缓冲区
    int buf_index = 0;

    va_list args;
    va_start(args, fmt);

    for (int i = 0; fmt[i] != '\0'; i++)
    {
        if (fmt[i] == '%' && fmt[i+1] != '\0')
        {
            char tmp[32];
            tmp[0] = '\0';

            switch (fmt[i+1])
            {
                case 'd':
                {
                    int val = va_arg(args, int);
                    int_to_str(val, tmp);
                    break;
                }
                case 'f':
                {
                    double val = va_arg(args, double); // float 被提升为 double
                    float_to_str((float)val, tmp);
                    break;
                }
                case 's':
                {
                    char *val = va_arg(args, char*);
                    strcpy(tmp, val);
                    break;
                }
                default:
                    tmp[0] = fmt[i+1];
                    tmp[1] = '\0';
                    break;
            }

            int len = strlen(tmp);
            if (buf_index + len < sizeof(buffer))
            {
                strcpy(&buffer[buf_index], tmp);
                buf_index += len;
            }

            i++; // 跳过格式字符
        }
        else
        {
            if (buf_index + 1 < sizeof(buffer))
                buffer[buf_index++] = fmt[i];
        }
    }

    buffer[buf_index] = '\0';
    va_end(args);

    UART_DMA_Send(pSerial, buffer);
}
