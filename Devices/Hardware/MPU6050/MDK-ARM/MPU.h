#ifndef MPU6050_H
#define MPU6050_H

#include "stm32f1xx_hal.h"
#include "main.h"
#include "i2c.h"


typedef struct {
    float Ax_, Ay_, Az_;
    float Gx_, Gy_, Gz_;
    float Temperature_;
} MPUData_t;

typedef struct {
    float pitch;  // 俯仰角
    float roll;   // 横滚角  
    float yaw;    // 偏航角
    uint32_t last_time;
} Angle_t;
// 全局声明
extern MPUData_t sensor_data;
extern Angle_t current_angle;
extern int turning_flag;
// 您的寄存器定义和函数声明
#define MPU6050_ADDR    (0x68 << 1)
// ... 其他定义

HAL_StatusTypeDef MPU6050_Init(void);
void MPU6050_Read_Accel(void);
void MPU6050_Read_Gyro(void);
void MPU6050_Data_Update(void);
void calculate_angle_from_gyro(float gx, float gy, float gz, float dt);
void init_adaptive_compensation(void);
void update_adaptive_compensation(float temp, float ax, float ay, float az, float gx, float gy, float gz);
void apply_adaptive_compensation(float temp, float *ax, float *ay, float *az, float *gx, float *gy, float *gz);
uint8_t is_stationary(float ax, float ay, float az, float gx, float gy, float gz);
void apply_low_pass_filter(float *filtered, float new_value, float alpha);
uint8_t is_data_valid(float ax, float ay, float az, float gx, float gy, float gz);
void turning_state_judge(MPUData_t *data);
MPUData_t process_MPUdata(void);
#endif
