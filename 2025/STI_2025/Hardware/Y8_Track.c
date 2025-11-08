#include "Y8_Track.h"
#include "i2c.h"
#include "stdio.h"

#define LINE_I2C_ADDR   (0x12 << 1)   // 注意：HAL库需要左移1位

uint8_t Y8_Line_Value   			;
uint8_t Y8_Line_Array[9] = {0};

// 读取8路数据,并转化为数组,为了方便起见,数组有9位,1-8为有效数据
void Y8_LineSensor_Update(void)
{
	// 状态寄存器
	uint8_t reg = 0x30;  

	// 从寄存器 0x30 读取 1 字节数据
	HAL_I2C_Mem_Read(&hi2c1, LINE_I2C_ADDR, reg, I2C_MEMADD_SIZE_8BIT, &Y8_Line_Value, 1, 100);
	
	// 转化数据
	for (int i = 1; i < 9; i++)
	{
			Y8_Line_Array[i] = 1 - ( (Y8_Line_Value >> (8 - i)) & 0x01 );   // 从高位到低位依次提取
	}
}

// 巡线算法编写












