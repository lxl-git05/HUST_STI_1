#include "AllHeader.h"

// Mymain初始化集合,使主函数更简洁
void Initial_ALL(void)
{
	// Hardware
	OLED_Init() ;																	// OLED初始化
	IMU_Mahony_Init(0) ;														// 陀螺仪初始化
	Con_Servo_Init() ;															// 舵机初始化(6路归中90°)

	// Software
	Serial_Init();																// 串口初始化
	Encoder_Init() ;														// EC11编码器EXTI初始化

	// Control
	// ★ 注意顺序: Con_Motor_Init 必须先于 Param_AT24C02_Init，
	//   否则 EEPROM 恢复的电机 PID 参数会被 PID_Init 默认值覆盖
	Con_Motor_Init() ;														// 电机初始化
	Param_AT24C02_Init() ;													// AT24C02初始化+从EEPROM恢复参数

	// Tools
	Flash_Mode_Init() ;														// LED闪烁工具初始化
	Timer_Counter_Init() ;												// 时间戳测定初始化
}

// 定时器初始化
void Initial_Timer(void)
{
	Timer_Initial() ;
}
