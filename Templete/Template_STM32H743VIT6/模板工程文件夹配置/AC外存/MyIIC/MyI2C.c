#include "stm32f1xx.h"
#include "MyI2C.h"
#include "stdint.h"

void MyI2C_Init(void)
{
	// 不需要该函数,因为Cube已经配置好了
	/****** Cube配置:
	1. MPU_SCL --- 	PA4 , 开漏 , 初始为高电平 , 无上下拉
	2. MPU_SDA --- 	PA5 , 开漏 , 初始为高电平 , 无上下拉
	******/
}

void Delay_us_diy_MPU(uint32_t us)
{
	us *= 10; // 校准因子，需实测调整
	while (us--)
	{
		__NOP(); __NOP(); __NOP(); __NOP(); // 4个NOP，防止被优化
	}
}

void MyI2C_W_SCL(uint8_t Bitvalue)
{
	HAL_GPIO_WritePin(Y8_SCL_GPIO_Port, Y8_SCL_Pin, (GPIO_PinState)Bitvalue);
	Delay_us_diy_MPU(10);
}

void MyI2C_W_SDA(uint8_t Bitvalue)
{
	HAL_GPIO_WritePin(Y8_SDA_GPIO_Port, Y8_SDA_Pin, (GPIO_PinState)Bitvalue);
	Delay_us_diy_MPU(10);
}

uint8_t MyI2C_R_SDA(void)
{
	uint8_t Bitvalue = HAL_GPIO_ReadPin(Y8_SDA_GPIO_Port , Y8_SDA_Pin);
	Delay_us_diy_MPU(10);
	return Bitvalue;
}

void MyI2C_Start(void)
{
	MyI2C_W_SDA(1);	// 这里的SDA放在前面是因为如果SDA先拉低，那么SCL拉低时，SDA会因为上拉电阻而拉高
	MyI2C_W_SCL(1);
	MyI2C_W_SDA(0);
	MyI2C_W_SCL(0);
}	

void MyI2C_Stop(void)
{
	MyI2C_W_SDA(0);
	MyI2C_W_SCL(1);
	MyI2C_W_SDA(1);
}	

void MyI2C_SendByte(uint8_t Byte)
{
	uint8_t i;
	for (i = 0; i < 8; i++)
	{
		MyI2C_W_SDA(Byte & (0x80 >> i));
		MyI2C_W_SCL(1);
		MyI2C_W_SCL(0);
	}	
}

// 源代码会导致数据偏移一位,增加延时后改为下方代码
//uint8_t MyI2C_ReceiveByte(void)
//{
//	uint8_t i , Byte = 0x00 ;
//	MyI2C_W_SDA(1);
//	for (i = 0; i < 8; i++)
//	{
//		MyI2C_W_SCL(1);
//		if (MyI2C_R_SDA() == 1)
//		{
//			Byte |= (0x80 >> i);
//		}
//		MyI2C_W_SCL(0);
//	}
//	return Byte ;
//}

// 延时版接收代码
uint8_t MyI2C_ReceiveByte(void)
{
    uint8_t i, Byte = 0x00;
    MyI2C_W_SDA(1);  // 释放SDA线，准备接收

    for (i = 0; i < 8; i++)
    {
        MyI2C_W_SCL(1);          // 产生时钟高电平
        Delay_us_diy_MPU(5);     // 增加采样稳定性
        if (MyI2C_R_SDA())       // 读取SDA电平
        {
            Byte |= (1 << (7 - i)); // 从高位到低位填充
        }
        MyI2C_W_SCL(0);          // 产生时钟低电平
        Delay_us_diy_MPU(5);
    }
    return Byte;
}


void MyI2C_SendAck(uint8_t Ackbit)
{
	MyI2C_W_SDA(Ackbit);
	MyI2C_W_SCL(1);
	MyI2C_W_SCL(0);
}

uint8_t MyI2C_ReceiveAck(void)
{
	uint8_t AckBit ;
	MyI2C_W_SDA(1);
	MyI2C_W_SCL(1);
	AckBit = MyI2C_R_SDA();
	MyI2C_W_SCL(0);
	return AckBit ;
}
