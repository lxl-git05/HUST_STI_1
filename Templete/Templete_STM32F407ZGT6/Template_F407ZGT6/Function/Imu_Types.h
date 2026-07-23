#ifndef __IMU_TYPES_H
#define __IMU_TYPES_H

// ==================== IMU 通用数据类型 ====================
// 适用于 MPU6050 和 ICM42688，实现统一的角度解算接口

// 零偏校准值（加速度误差 + 陀螺仪零漂）
typedef struct
{
	float AccErrorX ;	// 加速度X零偏 (g)
	float AccErrorY ;	// 加速度Y零偏 (g)
	float AccErrorZ ;	// 加速度Z零偏 (g) — 静止时应为 1g 的偏移量

	float GyroErrorX ;	// 陀螺仪X零漂 (°/s)
	float GyroErrorY ;	// 陀螺仪Y零漂 (°/s)
	float GyroErrorZ ;	// 陀螺仪Z零漂 (°/s)
} ImuOffset_Typedef ;

// 零偏纠正后的物理量（直接参与姿态解算）
typedef struct
{
	float AX ;	// 加速度X (g)
	float AY ;	// 加速度Y (g)
	float AZ ;	// 加速度Z (g) — 静止时接近 1.0

	float GX ;	// 角速度X (°/s)
	float GY ;	// 角速度Y (°/s)
	float GZ ;	// 角速度Z (°/s)
} ImuCali_Typedef ;

// 最终输出：归一化加速度 + 欧拉角
typedef struct
{
	float AccX ;	// 归一化加速度X (方向余弦)
	float AccY ;	// 归一化加速度Y
	float AccZ ;	// 归一化加速度Z

	float roll ;	// 横滚角 (°)
	float pitch ;	// 俯仰角 (°)
	float yaw ;		// 偏航角 (°) — 无磁力计时会漂移
} ImuReal_Typedef ;

#endif
