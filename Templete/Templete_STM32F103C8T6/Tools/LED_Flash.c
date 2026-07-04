#include "LED_Flash.h"

// LED闪烁结构体
typedef struct
{
	GPIO_TypeDef * LED_Port ;								// 引脚x
	uint16_t LED_Pin ;											// 引脚号
	GPIO_PinState LED_ON_State ;						// LED亮
	GPIO_PinState LED_OFF_State ;						// LED灭

	int Flash_Cnt ;													// 计数器
	int Flash_Cnt_Max ;											// 一次模式的周期
	
	LED_Flash_Mode_Typedef Flash_Mode ;			// 闪烁模式,默认为慢闪
}LED_Flash_Typedef ;

// LED 参数
LED_Flash_Typedef LED_Flash_0 ;

// LED初始化函数
void LED_Flash_Init(LED_Flash_Typedef *LED , GPIO_TypeDef * LED_Port , uint16_t LED_Pin , GPIO_PinState LED_ON , GPIO_PinState LED_OFF)
{
	LED->LED_Port = LED_Port ;
	LED->LED_Pin = LED_Pin ;

	LED->LED_ON_State  = LED_ON ;
	LED->LED_OFF_State = LED_OFF ;

	LED->Flash_Cnt = 0 ;
	
	LED->Flash_Cnt_Max = 0 ;	
	LED->Flash_Mode = LED_Flash_ON ;
}

// LED状态检测更新函数
void LED_Flash_Tick(LED_Flash_Typedef *LED)
{
	// 慢闪模式
	if (LED->Flash_Mode == LED_Flash_Slow)
	{
		if (LED->Flash_Cnt < 500)
		{
			HAL_GPIO_WritePin(LED->LED_Port , LED->LED_Pin , LED->LED_ON_State) ;
		}
		else
		{
			HAL_GPIO_WritePin(LED->LED_Port , LED->LED_Pin , LED->LED_OFF_State) ;
		}
	}
	// 快闪模式
	else if (LED->Flash_Mode == LED_Flash_Fast)
	{
		if (LED->Flash_Cnt < 100)
		{
			HAL_GPIO_WritePin(LED->LED_Port , LED->LED_Pin , LED->LED_ON_State) ;
		}
		else
		{
			HAL_GPIO_WritePin(LED->LED_Port , LED->LED_Pin , LED->LED_OFF_State) ;
		}
	}	
	// 瞬闪模式
	else if (LED->Flash_Mode == LED_Flash_Topp)
	{
		if (LED->Flash_Cnt < 900)
		{
			HAL_GPIO_WritePin(LED->LED_Port , LED->LED_Pin , LED->LED_OFF_State) ;
		}
		else
		{
			HAL_GPIO_WritePin(LED->LED_Port , LED->LED_Pin , LED->LED_ON_State) ;
		}
	}	
	// 常亮模式
	else if (LED->Flash_Mode == LED_Flash_ON)
	{
		HAL_GPIO_WritePin(LED->LED_Port , LED->LED_Pin , LED->LED_ON_State) ;
	}
	// 常灭模式
	else if (LED->Flash_Mode == LED_Flash_OFF)
	{
		HAL_GPIO_WritePin(LED->LED_Port , LED->LED_Pin , LED->LED_OFF_State) ;
	}
	
	// LED 计数自增
	LED->Flash_Cnt ++ ;
	if (LED->Flash_Cnt >= LED->Flash_Cnt_Max)
	{
		LED->Flash_Cnt = 0 ;
	}
}

// LED设置模式函数
void LED_Flash_Set_Mode(LED_Flash_Typedef *LED , LED_Flash_Mode_Typedef Mode)
{
	LED->Flash_Mode = Mode ;
	
	if (LED->Flash_Mode == LED_Flash_Slow)
	{
		LED->Flash_Cnt_Max = 1000 ;
	}
	else if (LED->Flash_Mode == LED_Flash_Fast)
	{
		LED->Flash_Cnt_Max = 200 ;
	}
	else if (LED->Flash_Mode == LED_Flash_Topp)
	{
		LED->Flash_Cnt_Max = 1000 ;
	}
	else if (LED->Flash_Mode == LED_Flash_ON)
	{
		LED->Flash_Cnt_Max = 0 ;
	}
	else if (LED->Flash_Mode == LED_Flash_OFF)
	{
		LED->Flash_Cnt_Max = 0 ;
	}
}

// ============== 特定LED,作为外部引出 ==============

void LED_Flash_Mode_Init(void)
{
	LED_Flash_Init(&LED_Flash_0 , LED0_GPIO_Port , LED0_Pin , GPIO_PIN_RESET , GPIO_PIN_SET) ;
}

void LED_Flash_Mode_Tick(void)
{
	LED_Flash_Tick(&LED_Flash_0) ;
}

void LED_Flash_Mode_Set_Mode(LED_Flash_Mode_Typedef Mode)
{
	LED_Flash_Set_Mode(&LED_Flash_0 , Mode) ;
}
