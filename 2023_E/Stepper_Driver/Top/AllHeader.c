#include "AllHeader.h"

// Mymain初始化集合,使主函数更简洁
void Initial_ALL(void)
{
	// Hardware
	OLED_Init() ;																	// OLED初始化
	
	// Software
	
	
	// Control
	Con_Motor_Init() ;														// 电机初始化
	
	// Tools
	Flash_Mode_Init() ;														// LED闪烁工具初始化
	Timer_Counter_Init() ;												// 时间戳测定初始化
}

// 定时器初始化
void Initial_Timer(void)
{
	Timer_Initial() ;
}

