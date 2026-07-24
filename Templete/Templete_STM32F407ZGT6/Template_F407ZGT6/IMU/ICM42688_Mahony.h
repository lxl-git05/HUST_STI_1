#ifndef __ICM42688_MAHONY_H
#define __ICM42688_MAHONY_H

#include "ICM_42688_base.h"
#include "IMU.h"

// ==================== Mahony AHRS 参数 ====================
// 参考 DAIMXA angle.c，适配 ICM-42688 (20ms Tick, 低噪声 70μg/√Hz)
#define MAHONY_KP       5.12f    // PI 比例增益
#define MAHONY_KI       0.001f   // PI 积分增益
#define MAHONY_HALF_T   0.010f   // 半采样周期 (20ms / 2)，单位秒

// ==================== 标定参数 ====================
#define MAHONY_CALIB_SAMPLES        1000     // 标定采样数

// ==================== 全局输出 ====================
extern ImuReal_Typedef ICM_Mahony_Real;

// ==================== 【陀螺零偏（extern，可供 AT24C02 读写）】 ====================
extern float ICM_Mahony_GyroBiasX;	// 陀螺X零偏 (°/s)
extern float ICM_Mahony_GyroBiasY;	// 陀螺Y零偏 (°/s)
extern float ICM_Mahony_GyroBiasZ;	// 陀螺Z零偏 (°/s)

// ==================== API ====================
// doCalib=1: 自动标定零偏（原地采样 MAHONY_CALIB_SAMPLES 次）
// doCalib=0: 跳过标定，使用 ICM_Mahony_GyroBiasX/Y/Z 当前值（直接填的 0.0f 或 AT24C02 恢复值）
void ICM42688_Mahony_Init(uint8_t doCalib);

// 20ms Tick 入口
void ICM42688_Mahony_Update_Tick(void);	// 1.38ms

// 运行时手动重新标定零偏（不重新初始化硬件）
void ICM42688_Mahony_Calibrate(int samples);

// 绝对累计偏航角：顺时针持续增大，无 ±180° 跳变，可超过 360°
float ICM_Yaw_Abs_Get(void);
void  ICM_Yaw_Abs_Reset(void);     // 归零 yaw_abs（不影响 yaw 解算）

#endif
