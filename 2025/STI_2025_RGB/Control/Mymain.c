#include "Mymain.h"

void Mymain(void)
{
	// ******************* setup *******************
	// 初始化
	{
		HAL_SYSTICK_Config(SystemCoreClock / 1000);
		OLED_Init() ;
		Serial_Init(&Serial_huart) ;
		// ***全部初始化完毕后再开启Systick中断***
		__enable_irq();	
	}
	// *******************实验区域*******************
//	// Debug调试参数
//	int check1 ;
//	int check2 ;
//	int check[50] ;
	while(1)
	{
		// ******************* while *******************
		// 测试按键功能
		if (Key_Check(KEY_1 , KEY_SINGLE))
		{
			HAL_GPIO_TogglePin(LED0_GPIO_Port , LED0_Pin) ;
		}
		// ******************* 实验区域 *******************
		
		// 必须存在:OLED更新
		OLED_Update() ;
	}
}

// Systick定时中断,1ms周期
void HAL_SYSTICK_Callback(void)
{
	// 计时
	static int count_sys = 0 ;
	count_sys ++ ;
	// 功能1: 按键
	Key_Tick() ;
	// 功能2:
	if (count_sys % 1000 == 0)
	{
//		HAL_GPIO_TogglePin(LED0_GPIO_Port , LED0_Pin ) ;
	}
}
