#ifndef __MPU6050_BASE_H
#define __MPU6050_BASE_H

#include "AllHeader.h"

// 内部参数定义
typedef struct
{
	float AX ;	// 加速度x
	float AY ;	// 加速度y
	float	AZ ;	// 加速度z
	float	GX ;	// 角速度x 
	float	GY ;	// 角速度y
	float GZ ;	// 角速度z
}MPU6050_Raw_Data ;

extern MPU6050_Raw_Data  MPU_Raw_Data ;		// 最初的角度

// ************函数声明************

// 初始化MPU6050相关配置
void MPU6050_Init(void);
// MPU6050写入数据
void MPU6050_WriteReg(uint8_t RegAddress , uint8_t Data);
// MPU6050读取数据
uint8_t MPU6050_ReadReg(uint8_t RegAddress) ;
// MPU6050参数读取
void MPU6050_GetData(int16_t *AccX, int16_t *AccY, int16_t *AccZ,int16_t *GyroX, int16_t *GyroY, int16_t *GyroZ) ;
// MPU6050得到器件ID
uint8_t MPU6050_GetID(void) ;
// 原始数据更新到MPU6050_Raw_Data
void MPU6050_Update_Data(void) ;

#endif
