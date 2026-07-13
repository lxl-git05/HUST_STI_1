#include "Mode_2.h"
#include "AllHeader.h"

void Mode_2_Setup(void)
{
	OLED_Clear();

	// 初始化参数编辑器
	Param_Init();

	// 注册 5 个演示参数
	// AT24C02 的值已在 Initial_ALL → Param_AT24C02_Init() 中从 EEPROM 恢复
	// Param_Register 内会自动检测 AT 关联并载入已存值
	Param_Register("Mode",  &g_mode,       1,    PARAM_INT8);
	Param_Register("Speed", &g_motorSpeed, 10,   PARAM_INT32);
	Param_Register("Kp",    &g_pidKp,      0.1f, PARAM_FLOAT);
	Param_Register("Ki",    &g_pidKi,      0.01f,PARAM_FLOAT);
	Param_Register("Kd",    &g_pidKd,      0.01f,PARAM_FLOAT);
}

void Mode_2_Loop(void)
{
	// 非编辑模式时显示静态参数快照
	if (!Param_IsActive())
	{
		
	}

	// Param_Loop 内部会自行处理 OLED 显示 (Param_Show)
	// 编辑模式下 Param_Show 绘制完整参数列表
	Param_Loop();
}

void Mode_2_Tick(void)
{
	
}

void Mode_2_Exit(void)
{
	
}
