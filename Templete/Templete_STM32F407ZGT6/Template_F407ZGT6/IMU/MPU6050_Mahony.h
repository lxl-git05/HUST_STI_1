#ifndef __MPU6050_MAHONY_H
#define __MPU6050_MAHONY_H

#include "MPU6050_base.h"
#include "IMU.h"

// ==================== Mahony AHRS 参数 ====================
// 适配 MPU6050 (20ms Tick, 噪声 ~400μg/√Hz)
#define MPU_MAHONY_KP       5.12f    // PI 比例增益
#define MPU_MAHONY_KI       0.001f   // PI 积分增益
#define MPU_MAHONY_HALF_T   0.010f   // 半采样周期 (20ms / 2)，单位秒

// ==================== 标定参数 ====================
#define MPU_MAHONY_CALIB_SAMPLES        1000     // 标定采样数

// ==================== 全局输出 ====================
extern ImuReal_Typedef MPU_Mahony_Real;

// ==================== 【陀螺零偏（extern，可供 AT24C02 读写）】 ====================
extern float MPU_Mahony_GyroBiasX;	// 陀螺X零偏 (°/s)
extern float MPU_Mahony_GyroBiasY;	// 陀螺Y零偏 (°/s)
extern float MPU_Mahony_GyroBiasZ;	// 陀螺Z零偏 (°/s)

// ==================== API ====================
// doCalib=1: 自动标定零偏（原地采样 MPU_MAHONY_CALIB_SAMPLES 次）
// doCalib=0: 跳过标定，使用 MPU_Mahony_GyroBiasX/Y/Z 当前值（直接填的 0.0f 或 AT24C02 恢复值）
void MPU6050_Mahony_Init(uint8_t doCalib);

// 20ms Tick 入口
void MPU6050_Mahony_Update_Tick(void);

// 运行时手动重新标定零偏（不重新初始化硬件）
void MPU6050_Mahony_Calibrate(int samples);

// 绝对累计偏航角：顺时针持续增大，无 ±180° 跳变，可超过 360°
float MPU_Yaw_Abs_Get(void);
void  MPU_Yaw_Abs_Reset(void);     // 归零 yaw_abs（不影响 yaw 解算）

#endif
