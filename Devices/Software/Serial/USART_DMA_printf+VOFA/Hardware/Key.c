#include "stm32f1xx.h"                  // Device header
#include "Key.h"

#define KEY_PRESSED				1
#define KEY_UNPRESSED			0

#define KEY_TIME_DOUBLE			200		// 判断双击阈值,变为0时单击响应速度很快
#define KEY_TIME_LONG				2000	// 判断长按阈值
#define KEY_TIME_REPEAT			100		// 长按下数据变化快慢

uint8_t Key_Flag[KEY_COUNT];			// 各个按键的标志位

// 改写方法:
// 只需要改变Key_GetState的引脚标签即可
uint8_t Key_GetState(uint8_t n)		// 得到按键状态
{
	if (n == KEY_1)
	{
		if (HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin) == 0)
		{
			return KEY_PRESSED;
		}
	}
	else if (n == KEY_2)
	{
		if (HAL_GPIO_ReadPin(KEY2_GPIO_Port, KEY2_Pin) == 0)
		{
			return KEY_PRESSED;
		}
	}
//	else if (n == KEY_3)
//	{
//		if (HAL_GPIO_ReadPin(KEY3_GPIO_Port, KEY3_Pin) == 0)
//		{
//			return KEY_PRESSED;
//		}
//	}
//	else if (n == KEY_4)
//	{
//		if (HAL_GPIO_ReadPin(KEY4_GPIO_Port, KEY4_Pin) == 0)
//		{
//			return KEY_PRESSED;
//		}
//	}
	return KEY_UNPRESSED;
}

// 查看按键是否被按下(检查标志位HOLD)
uint8_t Key_Check(uint8_t n, uint8_t Flag)
{
	if (Key_Flag[n] & Flag)	
	{
		if (Flag != KEY_HOLD)
		{
			Key_Flag[n] &= ~Flag;
		}
		return 1;
	}
	return 0;
}

void Key_Tick(void)
{
	static uint8_t Count, i;
	static uint8_t CurrState[KEY_COUNT], PrevState[KEY_COUNT];
	static uint8_t S[KEY_COUNT];
	static uint16_t Time[KEY_COUNT];
	
	for (i = 0; i < KEY_COUNT; i ++)
	{
		if (Time[i] > 0)
		{
			Time[i] --;
		}
	}
	
	Count ++;
	if (Count >= 20)
	{
		Count = 0;
		
		for (i = 0; i < KEY_COUNT; i ++)
		{
			PrevState[i] = CurrState[i];
			CurrState[i] = Key_GetState(i);
			
			if (CurrState[i] == KEY_PRESSED)
			{
				Key_Flag[i] |= KEY_HOLD;
			}
			else
			{
				Key_Flag[i] &= ~KEY_HOLD;
			}
			
			if (CurrState[i] == KEY_PRESSED && PrevState[i] == KEY_UNPRESSED)
			{
				Key_Flag[i] |= KEY_DOWN;
			}
			
			if (CurrState[i] == KEY_UNPRESSED && PrevState[i] == KEY_PRESSED)
			{
				Key_Flag[i] |= KEY_UP;
			}
			
			if (S[i] == 0)
			{
				if (CurrState[i] == KEY_PRESSED)
				{
					Time[i] = KEY_TIME_LONG;
					S[i] = 1;
				}
			}
			else if (S[i] == 1)
			{
				if (CurrState[i] == KEY_UNPRESSED)
				{
					Time[i] = KEY_TIME_DOUBLE;
					S[i] = 2;
				}
				else if (Time[i] == 0)
				{
					Time[i] = KEY_TIME_REPEAT;
					Key_Flag[i] |= KEY_LONG;
					S[i] = 4;
				}
			}
			else if (S[i] == 2)
			{
				if (CurrState[i] == KEY_PRESSED)
				{
					Key_Flag[i] |= KEY_DOUBLE;
					S[i] = 3;
				}
				else if (Time[i] == 0)
				{
					Key_Flag[i] |= KEY_SINGLE;
					S[i] = 0;
				}
			}
			else if (S[i] == 3)
			{
				if (CurrState[i] == KEY_UNPRESSED)
				{
					S[i] = 0;
				}
			}
			else if (S[i] == 4)
			{
				if (CurrState[i] == KEY_UNPRESSED)
				{
					S[i] = 0;
				}
				else if (Time[i] == 0)
				{
					Time[i] = KEY_TIME_REPEAT;
					Key_Flag[i] |= KEY_REPEAT;
					S[i] = 4;
				}
			}
		}
	}
}
