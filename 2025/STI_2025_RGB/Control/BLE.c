#include "BLE.h"

// 蓝牙目标接收参数
int RGB_Mode   = 1 ;	// RGB模式状态 ,初始默认为自动
int Servo_Mode = 1 ;	// 舵机模式状态,初始默认为自动

extern int Menu_Confirm_Index ;	// 菜单确定序列,蓝牙也能调节

// 蓝牙控制模块初始化
void BLE_Init(void)
{
	Serial_Init(&Serial_huart) ;
}

// 蓝牙接收数据更新函数
void BLE_Data_Update(void)
{
	// 接收来自手机端的信息
	// 目标协议是手机发送的第一个数据为RGB模式状态(0/1) , 第二个数据为舵机模式状态(0/1)
	// 期待接收的数据包:FF AA 04 00 0x 00 0x 55 FE	, x = 0 / 1 / 2 ,其实2代表本次赋值与该变量无关
	// 手动模式改变的参数:三种颜色 + 两种舵机状态
	if (Serial_GetNewPackageFlag_HEX() == 1)
	{
		// RGB和Servo的手动模式和自动模式蓝牙转换
		/*
		指令集:
			RGB自动档		:FFAA040001000255FE
			RGB手动档		:FFAA040000000255FE
			Servo自动档	:FFAA040002000155FE
			Servo手动档	:FFAA040002000055FE
		*/
		if (Serial_Hex_Data.Serial_New_Package[0] == 2)	// 2条指令:手动or自动
		{
			if (Serial_Hex_Data.Serial_New_Package[1] != 2)
			{
				RGB_Mode   = Serial_Hex_Data.Serial_New_Package[1] ;
			}
			if (Serial_Hex_Data.Serial_New_Package[2] != 2)
			{
				Servo_Mode = Serial_Hex_Data.Serial_New_Package[2] ;
			}
		}
		
		// 手动档模式下进行手动控制
		if (Serial_Hex_Data.Serial_New_Package[0] == 1)	// 1条指令:1-3:RGB调控 , 4-5:舵机调控
		{
			/*
				指令集:
					RGB调节档位:
					红灯: FFAA02000155FE
					黄灯:	FFAA02000255FE
					绿灯:	FFAA02000355FE
				舵机调节档位:
						 L:	FFAA02000455FE
						 R: FFAA02000555FE
				*/
			if (RGB_Mode == 0 && Serial_Hex_Data.Serial_New_Package[1] <= 3)	// RGB手动档控制
			{
				Menu_Confirm_Index = Serial_Hex_Data.Serial_New_Package[1] ;
			}
			else if (Servo_Mode == 0 && Serial_Hex_Data.Serial_New_Package[1] > 3)
			{
				Menu_Confirm_Index = Serial_Hex_Data.Serial_New_Package[1] ;
			}
		}
		
	}
}
