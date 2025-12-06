#include "Mymain.h"
#include "Initial.h"
// =================== 全局变量 ===================

// =================== 实验区域 ===================
// Debug调试参数
//int check1 ;
//int check2 ;
//int check[50] ;

void Mymain(void)
{
	Initial_ALL() ;	
	__enable_irq(); // 与Systick有关的在Systick初始化后初始化
	// ===================实验区域===================

	while(1)
	{
		// =================== while ===================
		LED_Flash_Mode_Set_Mode(LED_Flash_Slow) ;
		
		// =================== 实验区域 ===================
		
		// 必须存在:OLED更新
		OLED_Update() ;
	}
}

// Systick定时中断,1ms周期
void HAL_SYSTICK_Callback(void)
{
	// 功能1: 按键,有Key1和Key2
	Key_Tick() ;
	// 功能2: LED闪烁指示灯
	LED_Flash_Mode_Tick() ;
}
