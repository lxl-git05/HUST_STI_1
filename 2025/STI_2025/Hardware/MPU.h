#ifndef MPU6050_H
#define MPU6050_H

#include "stm32f1xx_hal.h"
#include "main.h"

// 配置选项：注释/取消注释来选择使用硬件I2C还是软件I2C
// #define USE_HARDWARE_I2C    // 取消注释使用硬件I2C(PB6,PB7)，注释使用软件I2C(PB3,PB4)

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

/* USER CODE BEGIN Private defines */
#define MPU6050_ADDR        0x68        // 7位地址
#define I2C_RETRY_COUNT     3
#define I2C_TIMEOUT_MS      100

#ifdef USE_HARDWARE_I2C
    // 使用硬件I2C - 包含硬件I2C头文件
    #include "i2c.h"
    extern I2C_HandleTypeDef hi2c1;
#else
    // 使用软件I2C - 定义软件I2C引脚
    #define SCL_PIN    GPIO_PIN_3
    #define SDA_PIN    GPIO_PIN_4
    #define I2C_PORT   GPIOB

    // 引脚操作宏定义
    #define SCL_HIGH()  HAL_GPIO_WritePin(I2C_PORT, SCL_PIN, GPIO_PIN_SET)
    #define SCL_LOW()   HAL_GPIO_WritePin(I2C_PORT, SCL_PIN, GPIO_PIN_RESET)
    #define SDA_HIGH()  HAL_GPIO_WritePin(I2C_PORT, SDA_PIN, GPIO_PIN_SET)
    #define SDA_LOW()   HAL_GPIO_WritePin(I2C_PORT, SDA_PIN, GPIO_PIN_RESET)
    #define SDA_READ()  HAL_GPIO_ReadPin(I2C_PORT, SDA_PIN)

    // 软件I2C延时 - 增加延时时间
    #define I2C_DELAY() do { \
        volatile uint32_t i = 50; \
        while(i--); \
    } while(0)
#endif

/* USER CODE END Private defines */

// 主要功能函数
HAL_StatusTypeDef MPU6050_Init(void);
MPUData_t MPU6050_Data_Update(void);
void turning_state_judge(MPUData_t *data);
void calculate_angle_from_gyro(float gx, float gy, float gz, float dt);

// I2C读写函数
HAL_StatusTypeDef MPU6050_I2C_Read(uint16_t MemAddress, uint8_t *pData, uint16_t Size);
HAL_StatusTypeDef MPU6050_I2C_Write(uint16_t MemAddress, uint8_t *pData, uint16_t Size);

#ifndef USE_HARDWARE_I2C
// 软件I2C专用函数
void Software_I2C_Init(void);
#endif

// 辅助函数
void MPU6050_Read_Accel(void);
void MPU6050_Read_Gyro(void);
void MPU6050_Read_Temp(void);
void init_adaptive_compensation(void);
MPUData_t process_MPUdata(void);

#endif

