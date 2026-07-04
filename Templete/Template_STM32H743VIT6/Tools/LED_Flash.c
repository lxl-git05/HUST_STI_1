#include "LED_Flash.h"

// LED闪烁结构体
typedef struct
{
	MyGPIO_Typedef GPIO_LEDx ;			// LED引脚

	bool LED_ON_State ;					// LED亮
	bool LED_OFF_State ;				// 

	int Flash_Cnt ;						// 计数器
	int Flash_Cnt_Max ;					// 一次模式	

	Flash_Mode_Typedef Flash_Mode ;		// 闪烁模式,默认为慢闪
}Flash_Typedef ;

// LED初始化函数
void Flash_Init(Flash_Typedef *LED , MyGPIO_Typedef *GPIO_LEDx, bool LED_ON , bool LED_OFF)
{
	LED->GPIO_LEDx = *GPIO_LEDx ;

	LED->LED_ON_State = LED_ON ;
	LED->LED_OFF_State = LED_OFF ;


	LED->Flash_Cnt = 0 ;
	
	LED->Flash_Cnt_Max = 0 ;	
	LED->Flash_Mode = Flash_Mode_ON ;
}

static void Flash_Write( MyGPIO_Typedef* MyGPIO,bool isON)
{
	if (isON)
	{
		MyGPIO_WritePin(MyGPIO , 1) ;
	}
	else 
	{
		MyGPIO_WritePin(MyGPIO , 0) ;
	}
}

// LED状态检测更新函数
void Flash_Tick(Flash_Typedef *LED)
{
	// 慢闪模式
	if (LED->Flash_Mode == Flash_Mode_Slow)
	{
		if (LED->Flash_Cnt < 500)
		{
			Flash_Write(&LED->GPIO_LEDx , LED->LED_ON_State);
		}
		else
		{
			Flash_Write(&LED->GPIO_LEDx , LED->LED_OFF_State);
		}
	}
	// 快闪模式
	else if (LED->Flash_Mode == Flash_Mode_Fast)
	{
		if (LED->Flash_Cnt < 100)
		{
			Flash_Write(&LED->GPIO_LEDx , LED->LED_ON_State);
		}
		else
		{
			Flash_Write(&LED->GPIO_LEDx , LED->LED_OFF_State);
		}
	}	
	// 瞬闪模式
	else if (LED->Flash_Mode == Flash_Mode_Topp)
	{
		if (LED->Flash_Cnt < 900)
		{
			Flash_Write(&LED->GPIO_LEDx , LED->LED_OFF_State);
		}
		else
		{
			Flash_Write(&LED->GPIO_LEDx , LED->LED_ON_State);
		}
	}	
	// 常亮模式
	else if (LED->Flash_Mode == Flash_Mode_ON)
	{
		Flash_Write(&LED->GPIO_LEDx , LED->LED_ON_State);
	}
	// 常灭模式
	else if (LED->Flash_Mode == Flash_Mode_OFF)
	{
		Flash_Write(&LED->GPIO_LEDx , LED->LED_OFF_State);
	}
	
	// LED 计数自增
	LED->Flash_Cnt ++ ;
	if (LED->Flash_Cnt >= LED->Flash_Cnt_Max)
	{
		LED->Flash_Cnt = 0 ;
	}
}

// LED设置模式函数
void Flash_Set_Mode(Flash_Typedef *LED , Flash_Mode_Typedef Mode)
{
	LED->Flash_Mode = Mode ;
	
	if (LED->Flash_Mode == Flash_Mode_Slow)
	{
		LED->Flash_Cnt_Max = 1000 ;
	}
	else if (LED->Flash_Mode == Flash_Mode_Fast)
	{
		LED->Flash_Cnt_Max = 200 ;
	}
	else if (LED->Flash_Mode == Flash_Mode_Topp)
	{
		LED->Flash_Cnt_Max = 1000 ;
	}
	else if (LED->Flash_Mode == Flash_Mode_ON)
	{
		LED->Flash_Cnt_Max = 0 ;
	}
	else if (LED->Flash_Mode == Flash_Mode_OFF)
	{
		LED->Flash_Cnt_Max = 0 ;
	}
}

// ============== 特定LED,作为外部引出 ==============

// LED 参数
Flash_Typedef Flash_0 ;

void Flash_Mode_Init(void)
{
	Flash_Init(&Flash_0 , &MyGPIO_LED0, 1 , 0) ;
}

void Flash_Mode_Tick(void)
{
	Flash_Tick(&Flash_0) ;
}

void Flash_Mode_Set(Flash_Mode_Typedef Mode)
{
	Flash_Set_Mode(&Flash_0 , Mode) ;
}
