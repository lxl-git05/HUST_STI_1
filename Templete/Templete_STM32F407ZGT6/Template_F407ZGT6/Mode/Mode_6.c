#include "AllHeader.h"

//#define IS_ICM 1

// ========================== ICM42688 互补滤波角度测试 ==========================

void Mode_6_Setup(void)
{
	// 初始化
#ifdef IS_ICM
	ICM42688_Angle_Init();
//	ICM42688_Data_Error_Check(1000) ;
#else
	MPU6050_Angle_Init() ;
//	MPU6050_Data_Error_Check(1000) ;
#endif
}

void Mode_6_Loop(void)
{
#ifdef IS_ICM
	OLED_Printf(0, 12, OLED_6X8, "R:%.1f", ICM_Real.roll);
	OLED_Printf(0, 24, OLED_6X8, "P:%.1f", ICM_Real.pitch);
	OLED_Printf(0, 36, OLED_6X8, "Y:%.1f", ICM_Real.yaw);
#else
	OLED_Printf(0, 12, OLED_6X8, "R:%.1f", MPU_Real.roll);
	OLED_Printf(0, 24, OLED_6X8, "P:%.1f", MPU_Real.pitch);
	OLED_Printf(0, 36, OLED_6X8, "Y:%.1f", MPU_Real.yaw);
#endif
}

void Mode_6_Tick(void)
{
#ifdef IS_ICM
	ICM42688_Angle_Update_Tick();
	Serial_printf(&Serial1, "%.2f,%.2f,%.2f\r\n", ICM_Real.roll, ICM_Real.pitch, ICM_Real.yaw);
#else
	MPU6050_Angle_Update_Tick() ;
	Serial_printf(&Serial1, "%.2f,%.2f,%.2f\r\n", MPU_Real.roll, MPU_Real.pitch, MPU_Real.yaw);
#endif
}

void Mode_6_Exit(void)
{

}
