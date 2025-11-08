#include "Y8_Track.h"

// 寻迹的IIC模式
#define Y8_IIC_Soft

#define LINE_I2C_ADDR   (0x12 << 1)   // 注意：HAL库需要左移1位

uint8_t Y8_Line_Value   			;
uint8_t Y8_Line_Array[9] = {0};

// IIC软件模拟读取数据
uint8_t MyI2C_ReadReg(uint8_t devAddr, uint8_t regAddr)
{
    uint8_t data;

    MyI2C_Start();
    MyI2C_SendByte(devAddr | 0);       // 写模式
    if (MyI2C_ReceiveAck()) return 0xFF;

    MyI2C_SendByte(regAddr);           // 寄存器地址
    if (MyI2C_ReceiveAck()) return 0xFF;

    MyI2C_Start();                     // 重启信号
    MyI2C_SendByte(devAddr | 1);       // 读模式
    if (MyI2C_ReceiveAck()) return 0xFF;

    data = MyI2C_ReceiveByte();        // 读取数据
    MyI2C_SendAck(1);                  // 发送 NACK
    MyI2C_Stop();

    return data;
}

// 读取8路数据,并转化为数组,为了方便起见,数组有9位,1-8为有效数据
void Y8_LineSensor_Update(void)
{
	// 状态寄存器
	uint8_t reg = 0x30;  

	// 从寄存器 0x30 读取 1 字节数据,分两种方法,选其中一种:
	#ifdef Y8_IIC_Soft
	Y8_Line_Value = MyI2C_ReadReg(LINE_I2C_ADDR, reg);
	#else
	HAL_I2C_Mem_Read(&hi2c1, LINE_I2C_ADDR, reg, I2C_MEMADD_SIZE_8BIT, &Y8_Line_Value, 1, 100);
	#endif
	// 转化数据
	for (int i = 1; i < 9; i++)
	{
			Y8_Line_Array[i] = 1 - ( (Y8_Line_Value >> (8 - i)) & 0x01 );   // 从高位到低位依次提取
	}
}

// 巡线算法编写



