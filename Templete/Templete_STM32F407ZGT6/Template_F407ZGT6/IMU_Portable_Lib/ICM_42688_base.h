#ifndef __ICM_42688_BASE_H
#define __ICM_42688_BASE_H

#include "MySystem.h"

// 内部参数定义（与 MPU6050_Raw_Data 结构一致，方便上层复用）
typedef struct
{
	float AX ;	// 加速度x (单位: g)
	float AY ;	// 加速度y (单位: g)
	float AZ ;	// 加速度z (单位: g)
	float GX ;	// 角速度x (单位: °/s)
	float GY ;	// 角速度y (单位: °/s)
	float GZ ;	// 角速度z (单位: °/s)
}ICM42688_Raw_Data ;

extern ICM42688_Raw_Data  ICM_Raw_Data ;		// 原始数据

// ************函数声明************

// 初始化ICM42688相关配置
void ICM42688_Init(void);
// ICM42688写入数据
void ICM42688_WriteReg(uint8_t RegAddress , uint8_t Data);
// ICM42688读取数据
uint8_t ICM42688_ReadReg(uint8_t RegAddress) ;
// ICM42688参数读取（连续读取6轴原始ADC值）
void ICM42688_GetData(int16_t *AccX, int16_t *AccY, int16_t *AccZ,
                      int16_t *GyroX, int16_t *GyroY, int16_t *GyroZ) ;
// ICM42688得到器件ID
uint8_t ICM42688_GetID(void) ;
// 原始数据更新到ICM42688_Raw_Data（含灵敏度转换）
void ICM42688_Update_Data(void) ;

#endif
