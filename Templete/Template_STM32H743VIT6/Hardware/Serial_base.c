#include "Serial_base.h"

// ============== 全局变量（协议常量）==============
Serial_Agreement_ABC_TypeDef   Serial_Agreement_ABC;   // ABC协议帧
Serial_Agreement_HEX_TypeDef   Serial_Agreement_HEX;   // HEX协议帧

// ============== 协议初始化 ==============
// ABC协议初始化
void Serial_Agreement_ABC_Init(void)
{
    Serial_Agreement_ABC.head = '@';
    Serial_Agreement_ABC.end1 = '$';
    Serial_Agreement_ABC.end2 = '#';
}

// HEX协议初始化
void Serial_Agreement_HEX_Init(void)
{
    Serial_Agreement_HEX.head1 = 0xFF;
    Serial_Agreement_HEX.head2 = 0xAA;
    Serial_Agreement_HEX.end1  = 0x55;
    Serial_Agreement_HEX.end2  = 0xFE;
}
