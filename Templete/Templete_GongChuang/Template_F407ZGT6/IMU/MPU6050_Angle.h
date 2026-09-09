#ifndef __MPU6050_ANGLE_H
#define __MPU6050_ANGLE_H

#include "IMU.h"
#include "MPU6050_base.h"

// 预先给定的MPU6050零漂值
#define MPU6050_AX_Offset	0.132624149f
#define MPU6050_AY_Offset	-0.00553222653f
#define MPU6050_AZ_Offset	-0.0402218103f

#define MPU6050_GX_Offset -4.41270494f
#define MPU6050_GY_Offset -1.50023127f
#define MPU6050_GZ_Offset -0.650924802f

// 零漂自校准
#define STILL_ACCEL_THRES_BASE_SQ   (0.05f * 0.05f)   // 基础阈值 0.05g
#define STILL_GYRO_THRES_BASE_SQ    (3.0f * 3.0f)     // 基础阈值 3°/s
#define STILL_REQUIRED_CNT          100               // 连续100次满足才确认静止（可调 50~150）
#define OFFSET_LEARNING_RATE 				0.005f	  // 零漂自校准趋近率

// 引出参数 (ImuOffset_Typedef / ImuCali_Typedef / ImuReal_Typedef 定义在 IMU.h)
extern ImuOffset_Typedef  MPU_Offset;			// 误差纠正参数
extern ImuCali_Typedef	  MPU_Cali	 ;			// 纠正后的数据
extern ImuReal_Typedef 	  MPU_Real  ;			// 最终的确定角度
// extern int isMPU_Still_Flag ;

// ************函数************
// 初始化
void MPU6050_Angle_Init(void) ;
// 自动纠正误差(可配合按键使用)
void MPU6050_Data_Error_Check(int Sanple_Cnt) ;
// 得到陀螺仪的六个较精确角度
void MPU6050_Angle_Update_Tick(void) ;	// 耗时1.45ms
#endif
