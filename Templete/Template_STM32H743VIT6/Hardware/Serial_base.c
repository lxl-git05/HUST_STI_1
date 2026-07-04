#include "Serial_base.h"

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


