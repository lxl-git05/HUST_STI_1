#ifndef MPU6050_H
#define MPU6050_H

#include "stm32f1xx_hal.h"
#include <stdint.h>

// 您的寄存器定义和函数声明
#define MPU6050_ADDR    (0x68 << 1)
// ... 其他定义

HAL_StatusTypeDef MPU6050_Init(void);
void MPU6050_Read_Accel(void);
void MPU6050_Read_Gyro(void);
void MPU6050_Data_Update(void);

#endif
