#ifndef MPU6050_H
#define MPU6050_H

#include "stm32f1xx_hal.h"
#include "main.h"
#include "i2c.h"

// 全局声明
extern int MPU_flag;
// 处理后的传感器数据（物理量）
extern float Ax;
extern float Ay;
extern float Az;
extern float Gx;
extern float Gy;
extern float Gz;
extern float Temperature;

// 您的寄存器定义和函数声明
#define MPU6050_ADDR    (0x68 << 1)
// ... 其他定义

HAL_StatusTypeDef MPU6050_Init(void);
void MPU6050_Read_Accel(void);
void MPU6050_Read_Gyro(void);
void MPU6050_Data_Update(void);

#endif
