#include "Mymain.h"

// 要求(更多相关信息见README):
/*
	1. RGB灯:(可开关)
	1.1 自动档(1): 每个周期3s, 红灯3s - 黄灯3s - 绿灯3s
	1.2 手动挡(0): 可选择RGB颜色

	2. 指示牌
	2.1 自动档(1): 每个周期5s, R 5s - L 5s
	2.2 手动档(0): 可任意选择指示牌的方向: R / L

	3. 蓝牙
	蓝牙一旦进行相关参数控制就将相关外设变为手动挡
*/
void Mymain(void)
{
	// ******************* setup *******************
	// 初始化
	{
		HAL_SYSTICK_Config(SystemCoreClock / 1000);
		OLED_Init() ;
		Servo_Init() ;
		RGB_Init() ;
		BLE_Init() ;
		Menu_Init() ;
		// 全部初始化完毕后再开启Systick中断
		__enable_irq();	
	}

	while(1)
	{
		// ******************* while *******************
		BLE_Data_Update() ;					// 蓝牙接收模式信息
		RGB_Control(RGB_Mode) 	  ;	// RGB模式
		Servo_Control(Servo_Mode) ;	// 舵机模式
		Menu_Display() ;						// OLED菜单显示
		Menu_RGB_Servo_Manu_Update(RGB_Mode , Servo_Mode , &RGB_Manu_Num , &Servo_Manu_Num ) ;	// 菜单反馈,实现手动调控参数
//		Menu_Func() ;
		OLED_Update() ;	// 必须存在:OLED更新
	}
}

// Systick定时中断,1ms周期
void HAL_SYSTICK_Callback(void)
{
	// 功能1: 按键
	Key_Tick() ;
	// 功能2:RGB自动档函数
	RGB_Auto_Task__Possess()   ;
	// 功能3:Servo自动档函数
	Servo_Auto_Task__Possess() ;
}
