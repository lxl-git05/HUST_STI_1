#ifndef __MPU6050_ANGLE_H
#define __MPU6050_ANGLE_H

#include "MPU6050_base.h"

// 预先给定的MPU6050零漂值
#define MPU6050_AX_Offset	0.0151069332
#define MPU6050_AY_Offset	-0.0293471683
#define MPU6050_AZ_Offset	0.0195673704

#define MPU6050_GX_Offset -2.13066816
#define MPU6050_GY_Offset 1.19515312
#define MPU6050_GZ_Offset -0.132022813

// 零漂自校准
#define STILL_ACCEL_THRES_BASE_SQ   (0.05f * 0.05f)   // 基础阈值 0.05g
#define STILL_GYRO_THRES_BASE_SQ    (3.0f * 3.0f)     // 基础阈值 3°/s
#define STILL_REQUIRED_CNT          100               // 连续100次满足才确认静止（可调 50~150）
#define OFFSET_LEARNING_RATE 				0.005f	  // 零漂自校准趋近率

// 角度和加速度误差值
typedef struct
{
	float AccErrorX	 ;	// 加速度误差
	float AccErrorY	 ;	// 加速度误差
	float AccErrorZ	 ;	// 加速度累次平均值,水平放置时应该是1g,所以不进行零漂处理!
	
	float GyroErrorX ;	// 角速度误差 
	float GyroErrorY ;	// 角速度误差
	float GyroErrorZ ;	// 角速度误差
}ImuOffset_Typedef;

// 得到的零偏纠正值
typedef struct
{
	float AX ;	// 加速度
	float AY ;	// 加速度
	float AZ ;	// 加速度,由于别的数据处理都是使用去零漂的数据,所以这里不删除了,但是事实上就是raw
	
	float GX ;	// 角速度
	float GY ;	// 角速度
	float GZ ;	// 角速度
}ImuCali_Typedef;

// 得到的真实数据
typedef struct
{
	float	AccX ;	  // 加速度x 
	float	AccY ;	  // 加速度y
	float 	AccZ ;	  // 加速度z
	
	float	roll ;	// 角度x 
	float	pitch ;	// 角度y
	float 	yaw ;	// 角度z
}ImuReal_Typedef ;			// 真实角度值

// 引出参数
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
