#ifndef __SERIAL_H
#define __SERIAL_H

#include "main.h"
#include <stdbool.h>

// һЩ����ģʽ
//#define Serial_Debug
//#define Serial_VOFA


// ����ͨ��ͨ��(����Ǩ��ʱ�޸����Ｔ��)
#define Serial_huart huart1
#define Serial_USART USART1

// DMA�������鳤��,һ�ν��ܵ����ݲ��ܴ����������
#define RX_Serial_LEN 50
// DMA�ȴ�֡β�ж������ֵ
#define Serial_Wait_Tail_MAX 25

// ͨ����������ݰ��ĳ���(�϶���DMA��С,�����������)


// ********** �ṹ���ʼ�� **********
// ���ݽ��չ��̱�־λ
typedef enum
{
	// ���ݳ����������ݻ������׶�
	RX_OK_HEX 		= 0x00U,	// HEX���ݰ��������
	RX_OK_ABC 		= 0x01U,	// ABC���ݰ��������
	
	RX_BUSY		= 0x02U,	// ���ݰ����ڽ��մ洢��,�����˴ν�������
	RX_WAIT		= 0x03U,	// �ȴ����ݴ���(���ͷ֡��ͨ����һֱ��������)
	
	RX_Error_Tail_HEX = 0x6U,		// ����β֡����,�����������
	RX_Error_Tail_ABC = 0x7U,		// ����β֡����,�����������

}Serial_RX_FLAG_Typedef;
 
// ���ݰ���������
typedef enum
{
	Serial_Error_None = 0x00U,		// ��������
	Serial_Error_Head = 0x01U,		// ����ͷ֡����
	Serial_Error_Tail = 0x02U,		// ����β֡����
	Serial_Error_Data = 0x03U,		
	Serial_Error_Data_Len = 0x04U,

}Serial_Data_Error_Typedef;

// �������ݽ��ջ�����(��ǰ��ʹ������,����ʹ�ýṹ����н���)
typedef struct
{
	uint8_t rx_temp;							// DMA�����temp�ݴ�,���Һܿ콫��������rxBuf��
	uint8_t rxCnt;								// Cnt��¼DMA�����˶���λ����
	uint8_t rxBuf[RX_Serial_LEN];	// ���ջ�����,����temp����
}Serial_RX_Data_TypeDef;
 
// ����Э��:HEX
typedef struct
{
	uint8_t head1;	// ͷ֡1
	uint8_t head2;	// ͷ֡2
	uint8_t end1;		// β֡1
	uint8_t end2;		// β֡2
}Serial_Agreement_HEX_TypeDef;

// ����Э��:ABC
typedef struct
{
	uint8_t head;	  // ͷ֡
	uint8_t end1;		// β֡1
	uint8_t end2;		// β֡2
}Serial_Agreement_ABC_TypeDef;

// HEX�������ݰ�
typedef struct
{
	int Serial_New_Package[RX_Serial_LEN] ; 		// ��ȷ��Ϣ�洢����,���ȹܹ�,�Ժ��ٸ�
	bool Serial_New_Package_Flag ;							// ���ݰ��������flag
	int error_Serial	;								  				// �����ѯ����
}Serial_HEX_Data_Typedef;

// �ı��������ݰ�
typedef struct
{
	char Serial_New_Package_ABC[RX_Serial_LEN] ; // ��ȷ��Ϣ�洢����,���ȹܹ�,�Ժ��ٸ�
	bool Serial_New_Package_Flag ;							 // ���ݰ��������flag
	int error_Serial	;								  				 // �����ѯ����
}Serial_ABC_Data_Typedef;



// ********** �������� **********
// DMA���ڽ��ճ�ʼ��
void Serial_Init(UART_HandleTypeDef *huart) ;

// ���ڷ�������
void Serial_SendData_DMA(uint8_t *pData, uint16_t Size) ;

// HEX:�õ����ڽ��ձ�־λ
uint8_t Serial_GetNewPackageFlag_HEX(void) ;

// HEX:�õ�����ԭ��
int Serial_GetError_HEX(void) ;


// �ı�:�õ����ڽ��ձ�־λ
uint8_t Serial_GetNewPackageFlag_ABC(void) ;

// �ı�:�õ�����ԭ��
int Serial_GetError_ABC(void) ;

// �ı�:1. ��װһ������,ʵ�ּ��׸�������������
bool Serial_SetFloatData( char *KeyWord , char *cmd , float *Data) ;

// �ı�:2. ��װһ������,ʵ�ּ���������������
bool Serial_SetIntData( char *KeyWord , char *cmd , int *Data) ;

#endif

