#include "Serial.h"
#include "usart.h"

#include "string.h"
#include <stdarg.h>
#include <stdio.h>

// ********** ���� **********
Serial_RX_FLAG_Typedef 		Serial_Rx_State;							// ���ݽ��������־λ-ö��
Serial_RX_Data_TypeDef 		Serial_Rx_Data ;							// ���ݽ��ջ�����

Serial_Agreement_HEX_TypeDef 	Serial_Agreement_HEX ;		// ��������ͨ��Э��:HEX
Serial_Agreement_ABC_TypeDef 	Serial_Agreement_ABC ;		// ��������ͨ��Э��:ABC

Serial_HEX_Data_Typedef   Serial_Hex_Data ;							// *�����õ�HEX���ݰ�*
Serial_ABC_Data_Typedef   Serial_ABC_Data ;							// *�����õ�ABC���ݰ�*

#ifdef Serial_Debug
int Serial_check[200] ;
int Serial_Count = 0 ;
#endif

// ********** ���� **********
// ���ڽ��ջ�������ʼ��
void Serial_Rx_Data_Init(Serial_RX_Data_TypeDef *pSerial_Rx_Data)
{
	pSerial_Rx_Data->rxCnt = 0 ;											// ����������
	pSerial_Rx_Data->rx_temp = 0 ;										// �ݴ���������
	memset(pSerial_Rx_Data->rxBuf, 0, RX_Serial_LEN);	// ���ݻ���������
}

// ����Э���ʼ��:HEX
void Serial_Agreement_HEX_Init(Serial_Agreement_HEX_TypeDef *pSerial_Agreement)
{
	pSerial_Agreement->head1 = 0xFF ;
	
	pSerial_Agreement->head2 = 0xAA ;
	pSerial_Agreement->end1	 = 0x55 ;
	
	pSerial_Agreement->end2  = 0xFE	;
}

// ����Э���ʼ��:ABC
void Serial_Agreement_ABC_Init(Serial_Agreement_ABC_TypeDef *pSerial_Agreement)
{
	pSerial_Agreement->head  =  '@' ;
	pSerial_Agreement->end1	 =  '$' ;
	pSerial_Agreement->end2  =  '#' ;
}

// *���ڳ�ʼ��*
void Serial_Init(UART_HandleTypeDef *huart)
{
	// ���ջ�������ʼ��
	Serial_Rx_Data_Init(&Serial_Rx_Data) ;
	// ��DMA���պ���
	HAL_UART_Receive_DMA(huart, &Serial_Rx_Data.rx_temp , 1);   
	// ��ʼ������Э��
	Serial_Agreement_HEX_Init(&Serial_Agreement_HEX) ;
	Serial_Agreement_ABC_Init(&Serial_Agreement_ABC) ;
	// HEX���ı���������0,����Ҫ��ʼ��
}

// *���ڷ�������*
void Serial_SendData_DMA(uint8_t *pData, uint16_t Size)
{
	// ȷ��DMA���У���ֹ����ͬʱ����
	if (Serial_huart.gState == HAL_UART_STATE_READY)
	{
			HAL_UART_Transmit_DMA(&Serial_huart, pData, Size);
	}
}

// �Ӹ�8λ�͵�8λ�ϳ�һ������
uint16_t Merge_2Bytes(uint8_t high, uint8_t low)
{
    return ((uint16_t)high << 8) | low;
}

// ���ڽ������ݺ���---�������ռ��󴥷�Serial_Rx_Flag��OK
Serial_RX_FLAG_Typedef Serial_Rx_State_Check(void)
{
	// ���ݴ����ݼ��뻺����,��ֹ��ʧ
	int rxData = Serial_Rx_Data.rx_temp ;

	// ״̬��
	static int Serial_RxState = 0 ;
	// ����״̬,�ȴ�֡ͷ
	if (Serial_RxState == 0)
	{
		// ����:���ݼ�¼�ص�ԭ��
		Serial_Rx_Data.rxCnt = 0 ;
		
		// ����:�ȴ�֡ͷ-HEXģʽ
		if ( rxData == Serial_Agreement_HEX.head1 )
		{
			Serial_Rx_Data.rxBuf[Serial_Rx_Data.rxCnt++] = rxData ;
			Serial_RxState = 1 ;	// �ж�HEX֡β
			return RX_BUSY	;			// ��ʼ��������
		}
		else if ( rxData == Serial_Agreement_ABC.head )
		{
			Serial_Rx_Data.rxBuf[Serial_Rx_Data.rxCnt++] = rxData ;
			Serial_RxState = 2 ;	// �ж�ABC֡β
			return RX_BUSY	;			// ��ʼ��������
		}
		else
		{
			return RX_WAIT ;	// �����ȴ�
		}
	}
	// ��ʼ����HEXԭʼ���ݰ�
	else if (Serial_RxState == 1)
	{
		// ����:�ݴ�����ת�Ƶ�������
		Serial_Rx_Data.rxBuf[Serial_Rx_Data.rxCnt++] = rxData ;
		
		// ����:���֡β
		// ��⵽֡β,�������
		if (rxData == Serial_Agreement_HEX.end2)
		{
			Serial_RxState = 0 ;	// ״̬ת��
			return RX_OK_HEX ;
		}
		// û�ܼ�⵽֡β,�������
		else if (Serial_Rx_Data.rxCnt >= Serial_Wait_Tail_MAX)
		{
			Serial_RxState = 0 ;	// ״̬ת��
			memset(Serial_Rx_Data.rxBuf, 0, RX_Serial_LEN);	// ���
			Serial_Hex_Data.error_Serial = Serial_Error_Tail ;
			return RX_Error_Tail_HEX ;
		}
	}
	// ��ʼ����ABCԭʼ���ݰ�
	else if (Serial_RxState == 2)
	{
		// ����:�ݴ�����ת�Ƶ�������
		Serial_Rx_Data.rxBuf[Serial_Rx_Data.rxCnt++] = rxData ;
		
		// ����:���֡β
		// ��⵽֡β,�������
		if (rxData == Serial_Agreement_ABC.end2)
		{
			Serial_RxState = 0 ;	// ״̬ת��
			return RX_OK_ABC ;
		}
		// û�ܼ�⵽֡β,�������
		else if (Serial_Rx_Data.rxCnt >= Serial_Wait_Tail_MAX)
		{
			Serial_RxState = 0 ;	// ״̬ת��
			memset(Serial_Rx_Data.rxBuf, 0, RX_Serial_LEN);	// ���
			Serial_ABC_Data.error_Serial = Serial_Error_Tail ;
			return RX_Error_Tail_ABC ;
		}
	}
	
	return RX_BUSY  ;
}
// HEX:�����������ݰ�(���ϲ�����)
void Serial_Data_Deal_HEX(void)
{
	// 1. ��2������Ϊ���ݳ���(��0,1��Ϊ֡ͷ),�����Ǹߵ�λ,���Գ���2�������������ݳ���
	Serial_Hex_Data.Serial_New_Package[0] = Serial_Rx_Data.rxBuf[2] / 2;
	// 2. ��������
	for (int i = 3 , j = 1 ; i < 3 + Serial_Rx_Data.rxBuf[2] ; i += 2 , j ++)
	{
		Serial_Hex_Data.Serial_New_Package[j] = Merge_2Bytes(Serial_Rx_Data.rxBuf[i] , Serial_Rx_Data.rxBuf[i + 1] ) ;
	}
}	

// HEX:���ݼ��+�洢������
void Serial_Data_Check_HEX(void)
{
	// 1. ���֡ͷ�Ϲ���
	if (Serial_Rx_Data.rxBuf[0] != Serial_Agreement_HEX.head1 || Serial_Rx_Data.rxBuf[1] != Serial_Agreement_HEX.head2)
	{
		// ֡ͷ���Ϲ�
		Serial_Hex_Data.error_Serial = Serial_Error_Head;
		// ����:
		// �����ʽ����,�������û���ʾ������
		memset(Serial_Hex_Data.Serial_New_Package, 0, sizeof(Serial_Hex_Data.Serial_New_Package));
		// �洢��־λ��0
		Serial_Hex_Data.Serial_New_Package_Flag = 0 ;
	}
	// 2. �������ݳ��ȼ��֡β�Ϲ���
	else if (Serial_Rx_Data.rxBuf[Serial_Rx_Data.rxBuf[2] + 3] != Serial_Agreement_HEX.end1 || Serial_Rx_Data.rxBuf[Serial_Rx_Data.rxBuf[2] + 4] != Serial_Agreement_HEX.end2)
	{
		// ֡β���Ϲ�
		Serial_Hex_Data.error_Serial = Serial_Error_Tail ;
		// ����:
		// �����ʽ����,�������û���ʾ������
		memset(Serial_Hex_Data.Serial_New_Package, 0, sizeof(Serial_Hex_Data.Serial_New_Package));
		// �洢��־λ��0
		Serial_Hex_Data.Serial_New_Package_Flag = 0 ;
	}
	else
	{
		// �����������ݰ�(���ϲ�����)
		Serial_Data_Deal_HEX() ;
		// �޴���
		Serial_Hex_Data.error_Serial = Serial_Error_None ;
		// �洢��־λ��1
		Serial_Hex_Data.Serial_New_Package_Flag = 1 ;
	}
}
// *HEX:�õ����ڽ��ձ�־λ*
uint8_t Serial_GetNewPackageFlag_HEX(void)
{
	if (Serial_Hex_Data.Serial_New_Package_Flag == 1)			//�����־λΪ1
	{
		Serial_Hex_Data.Serial_New_Package_Flag = 0;
		return 1;					//�򷵻�1�����Զ������־λ
	}
	return 0;						//�����־λΪ0���򷵻�0
}

// *HEX:�õ�����ԭ��*
int Serial_GetError_HEX(void)
{
	return Serial_Hex_Data.error_Serial	 ;
}

// �ı�:���ݼ��+�洢+���ݴ���(�������ı�)������
void Serial_Data_Check_ABC(void)
{
	// ��ʼ����ı�
	// 1. ������ݰ�֡ͷ�Ƿ����
	if (Serial_Rx_Data.rxBuf[0] != Serial_Agreement_ABC.head)
	{
		Serial_ABC_Data.error_Serial = Serial_Error_Head ; // *����*:֡ͷ���Ϲ�
	}
	// 2. ��ʼһ�ߴ�������һ�߼��֡β,��1��ʼ(����֡ͷ)
	else
	{
		int i = 0 ;
		for (i = 1 ; Serial_Rx_Data.rxBuf[i+1] != Serial_Agreement_ABC.end1 ; i++)
		{
			Serial_ABC_Data.Serial_New_Package_ABC[i-1] = Serial_Rx_Data.rxBuf[i] ;
			// ����Ƿ����
			if (i > Serial_Wait_Tail_MAX)
				break ;
		}
		// ��1λ!!!,�Ͼ�������ζ��Ǳ����뿪for,����1λ
		Serial_ABC_Data.Serial_New_Package_ABC[i-1] = Serial_Rx_Data.rxBuf[i] ;	
		// ���1:�˳�for����Ϊ��⵽��֡β
		if (Serial_Rx_Data.rxBuf[i+1] == Serial_Agreement_ABC.end1)
		{
			// 3. ��ʼ����2��֡β
			if (Serial_Rx_Data.rxBuf[i+2] != Serial_Agreement_ABC.end2)
			{
				Serial_ABC_Data.error_Serial = Serial_Error_Tail ; // ����3:��2��֡β���Ϲ�
				memset(Serial_ABC_Data.Serial_New_Package_ABC, 0, sizeof(Serial_ABC_Data.Serial_New_Package_ABC));	// ��ռ�¼����
				return ;
			}
			else
			{
				// �����������ݰ�(���ϲ�����)
				Serial_ABC_Data.Serial_New_Package_ABC[i] = '\0' ;	// �Ӹ���β����
				// �޴���
				Serial_ABC_Data.error_Serial = 0 ; 
				// �洢��־λ��1
				Serial_ABC_Data.Serial_New_Package_Flag = 1 ;
			}
		}
		// ���2:�˳�for����Ϊ�����,˵����1��֡βû�м�⵽
		else
		{
			Serial_ABC_Data.error_Serial = Serial_Error_Tail ; // ����2:��1��֡β���Ϲ�
			memset(Serial_ABC_Data.Serial_New_Package_ABC, 0, sizeof(Serial_ABC_Data.Serial_New_Package_ABC));	// ��ռ�¼����
			return ;
		}
	}
}

// *�ı�:�õ����ڽ��ձ�־λ*
uint8_t Serial_GetNewPackageFlag_ABC(void)
{
	if (Serial_ABC_Data.Serial_New_Package_Flag == 1)			//�����־λΪ1
	{
		Serial_ABC_Data.Serial_New_Package_Flag = 0;
		return 1;					//�򷵻�1�����Զ������־λ
	}
	return 0;						//�����־λΪ0���򷵻�0
}

// *�ı�:�õ�����ԭ��*
int Serial_GetError_ABC(void)
{
	return Serial_ABC_Data.error_Serial ;
}

// *�ı�:1. ��װһ������,ʵ�ּ��׸�������������*
bool Serial_SetFloatData( char *KeyWord , char *cmd , float *Data)
{
	// KeyWordΪ�ؼ���,�б�����ָ�� cmdΪ���仰,����%f��,VOFA��ôд����Ҳ��ôд DataΪ���ոı����ı���
	// ����������%f����,λ�����ù�,�ո�Ҳ��Ҫע��,����Э�黹�ÿ�VOFA��ô�����
	// ��:	����VOFA����:	@Kp=%.2f$#	���ڽ���:	Serial_SetFloatData("Kp" , "Kp=%f" , &test1) ;
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

// *�ı�:2. ��װһ������,ʵ�ּ���������������*
bool Serial_SetIntData( char *KeyWord , char *cmd , int *Data)
{
	// KeyWordΪ�ؼ���,�б�����ָ�� cmdΪ���仰,����%d��,VOFAû��%d,����VOFAд%.0f���ɴ���%d DataΪ���ոı����ı���
	// ��������%d����,�ո�Ҳ��Ҫע��,����Э�黹�ÿ�VOFA��ô�����
	// ��:	����VOFA����:	@test=%.0f$#	���ڽ���:	Serial_SetIntData("test" , "test=%d" , &check1) ;
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

// ���ڿ����жϻص�����
/*���ڽ����жϻص�*/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == Serial_USART)
	{
		#ifdef Serial_Debug
		Serial_check[Serial_Count++] = Serial_Rx_Data.rx_temp ;	// �õ����н��յ�������
		#endif 
		
		// ��ô������ݴ���״̬(����)
		Serial_Rx_State = Serial_Rx_State_Check();
		
		// HEX���ݰ�
		if (Serial_Rx_State == RX_OK_HEX)
		{
			// ��ʼ����ԭʼ���ݰ�:HEX
			Serial_Data_Check_HEX() ;
		}
		// ABC���ݰ�
		else if (Serial_Rx_State == RX_OK_ABC)
		{
			// ��ʼ����ԭʼ���ݰ�:ABC
			Serial_Data_Check_ABC() ;
		}
		
		// ���´򿪴���DMA���գ�DMA����Ϊ������ģʽ
		HAL_UART_Receive_DMA(huart, &Serial_Rx_Data.rx_temp , 1);   
	}
}

