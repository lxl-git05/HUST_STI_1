// Orange 模块 — 初始化与通信更新
#include "Orange.h"
#include "AllHeader.h"

// STM32->Orange
int Oran_Goal = 0 ;	// STM32发送给Orange的参数

uint8_t Oran_cmd = 0 ;	// 0. 指令模式
int Oran_real = 0 ;			// 1. 偏差
int Oran_Speed=0	;			// 2. 钢球速度

// Orange通信脱机阈值调节:暂时设置6个
int Oran_Param[6] ;
/*
串口Orange通信解析帧意义:
	Oran_cmd：模式，0为工作模式
	Oran_real:距离目标位置的偏差,需要-1000
	Oran_Speed:钢球移动的速度
*/

// 香橙派数据更新,在Mode_G实现20ms固定更新
void Oran_Update(void)
{
	// 读取Serial2的消息
	if (Serial_GetNewPackageFlag_HEX(&Serial2))
	{
		// 第0位是cmd!!!所以后续都必须从1开始
		Oran_cmd = Serial_GetHexData(&Serial2 , 0) ;
		// 钢球识别模式
		if (Oran_cmd == 0)
		{
			Oran_real = Serial_GetHexData(&Serial2 , 1) ;
			Oran_Speed=	Serial_GetHexData(&Serial2 , 2) ;
		}
		// 
		else if (Oran_cmd == 1)
		{
			Oran_Param[0] = Serial_GetHexData(&Serial2 , 1) ;
			Oran_Param[1] = Serial_GetHexData(&Serial2 , 2) ;
			Oran_Param[2] = Serial_GetHexData(&Serial2 , 3) ;
			Oran_Param[3] = Serial_GetHexData(&Serial2 , 4) ;
			Oran_Param[4] = Serial_GetHexData(&Serial2 , 5) ;
			Oran_Param[5] = Serial_GetHexData(&Serial2 , 6) ;
		}

	}
}

// 香橙派处理
void Oran_Send_Data(int* Data) 
{
	if (Data == &Oran_Param[0]) {Serial_printf(&Serial2 , "@Oran_Param_1:%d$#",Oran_Param[0]);}
	if (Data == &Oran_Param[1]) {Serial_printf(&Serial2 , "@Oran_Param_2:%d$#",Oran_Param[1]);}
	if (Data == &Oran_Param[2]) {Serial_printf(&Serial2 , "@Oran_Param_3:%d$#",Oran_Param[2]);}
	if (Data == &Oran_Param[3]) {Serial_printf(&Serial2 , "@Oran_Param_4:%d$#",Oran_Param[3]);}
	if (Data == &Oran_Param[4]) {Serial_printf(&Serial2 , "@Oran_Param_5:%d$#",Oran_Param[4]);}
	if (Data == &Oran_Param[5]) {Serial_printf(&Serial2 , "@Oran_Param_6:%d$#",Oran_Param[5]);}
}
// ======================= 香橙派寻迹PID =======================
Pid_Typedef PID_Oran ;	// 铁球PID
#define Oran_PID_Dir (1)
void Oran_XY_Init(void)
{
	// PID初始化
	PID_Init(&PID_Oran , 0.0f , 0.0f , 0.0f , 200 , -200 , 1000) ;
}

void Oran_PID_Update(void)
{
	// 1. 香橙派更新数据,得到Real值:这个是全局任务，直接放在Mode_G
	// Oran_Update() ;
	// 2. PID数据更新:real更新 goal为0 set需要求
	PID_Oran.realPoint_Now = Oran_real ;
	// 3. PID计算
	PID_Update(&PID_Oran, PID_Oran.realPoint_Now) ;
	// 4. 内环驱动: 步进电机1即可
	Stepper_PWM_Speed_Set(&Stepper1 , PID_Oran.setPoint * Oran_PID_Dir , 0) ;
}
