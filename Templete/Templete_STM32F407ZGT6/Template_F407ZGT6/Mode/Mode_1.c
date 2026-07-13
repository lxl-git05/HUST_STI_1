// ========================== 脱机调参模式 ==========================
// 用途：专门用于脱机调整参数并保存到存储（如 AT24C02）
// =================================================================
#include "AllHeader.h"

void Mode_1_Setup(void)
{

}

void Mode_1_Loop(void)
{
	OLED_Printf(0, 0, OLED_6X8, "=====Mode_1=====") ;
}

// 打印电机A参数
void Mode_1_Tick(void)
{

}

void Mode_1_Exit(void)
{
	
}
