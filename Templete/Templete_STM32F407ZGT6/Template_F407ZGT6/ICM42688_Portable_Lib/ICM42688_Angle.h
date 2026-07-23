#ifndef __ICM42688_ANGLE_H
#define __ICM42688_ANGLE_H

#include "ICM_42688_base.h"
#include "Imu_Types.h"

// ==================== 互补滤波系数 ====================
#define ICM_COMP_GYRO   0.98f   // 陀螺仪权重
#define ICM_COMP_ACC    0.02f   // 加速度计权重

// ==================== 全局变量 ====================
extern ImuOffset_Typedef  ICM_Offset;
extern ImuCali_Typedef    ICM_Cali;
extern ImuReal_Typedef    ICM_Real;

// ==================== 函数声明 ====================
void ICM42688_Angle_Init(void);
void ICM42688_Data_Error_Check(int Sample_Cnt);
void ICM42688_Angle_Update_Tick(void);

#endif
