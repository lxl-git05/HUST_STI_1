#include "Mymain.h"
#include "Initial.h"
// =================== 全局变量 ===================

// =================== 实验区域 ===================

void Mymain(void)
{
	Initial_ALL() ;	
	__enable_irq(); // 与Systick有关的在Systick初始化后初始化

	while(1)
	{
//		Menu_Func() ;
		
		// 计时器
		Timer_Counter_Begin() ;
		
		Timer_Counter_End() ;
		
		
		OLED_ShowNum(0 , 0 , time_us , 8 , OLED_6X8) ;
		OLED_Update() ;
	}
}

// Systick定时中断,1ms周期
void HAL_SYSTICK_Callback(void)
{
	// 功能1: 按键
	Key_Tick() ;
	// 功能2: LED闪烁指示灯
	LED_Flash_Mode_Tick() ;
	// 功能3: 单次任务处理序列
	task_Once_Cnt_Tick() ;
}
