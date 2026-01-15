#include "Initial.h"

// Mymain初始化集合,使主函数更简洁
void Initial_ALL(void)
{
	// System
	HAL_SYSTICK_Config(SystemCoreClock / 1000);		// Systick初始化
	
	// Hardware
	OLED_Init() ;																	// OLED初始化
	
	// Software
	Serial_Init() ;																// 串口初始化
	
	// Tools
	LED_Flash_Mode_Init() ;												// LED闪烁工具初始化
	Timer_Counter_Init() ;												// 时间戳测定初始化

	// Top
	Menu_Init() ;
}
