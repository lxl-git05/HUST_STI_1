#include "Mymain.h"

// 要求:
/*
1. RGB灯:(可开关)
1.1 自动档: 每个周期3s, 红灯3s - 黄灯3s - 绿灯3s
1.2 手动挡: 可选择RGB颜色

2. 指示牌
2.1 自动档: 每个周期5s, R 5s - L 5s
2.2 手动档: 可任意选择指示牌的方向: R / L

3. 蓝牙
实现控制切换这两个模式各自的手动挡和自动档
*/

// *******************全局变量*******************


// *******************实验区域变量*******************
// Debug调试参数
int check1 = 1;
int check2 ;
int check[50] = {0};

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
		// 全部初始化完毕后再开启Systick中断
		__enable_irq();	
	}
	// *******************实验区域*******************
	
	while(1)
	{
		// ******************* while *******************
		BLE_Data_Update() ;					// 蓝牙接收模式信息
		RGB_Control(RGB_Mode) 	  ;	// RGB模式
		Servo_Control(Servo_Mode) ;	// 舵机模式
		// ******************* 实验区域 *******************
		
//		// 必须存在:OLED更新
//		OLED_Update() ;
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
	// 功能2:RGB自动档函数
	RGB_Auto_Task__Possess()   ;
	// 功能3:Servo自动档函数
	Servo_Auto_Task__Possess() ;
}
