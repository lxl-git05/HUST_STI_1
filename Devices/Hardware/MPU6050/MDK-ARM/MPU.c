#include "MPU.h"

// 全局变量
int MPU_flag = 0;
float Ax, Ay, Az;
float Gx, Gy, Gz;
float Temperature;

/* MPU6050 I2C设备地址定义 */
#define MPU6050_ADDR    (0x68 << 1)  // 若AD0接地，7位地址0x68，左移1位得到8位地址0xD0
#define WHO_AM_I_REG    0x75        // WHO_AM_I寄存器地址，默认值0x68
#define PWR_MGMT_1_REG  0x6B        // 电源管理寄存器1
#define SMPLRT_DIV_REG  0x19        // 采样率分频寄存器
#define CONFIG_REG      0x1A        // 配置寄存器（含DLPF设置）
#define GYRO_CONFIG_REG 0x1B        // 陀螺仪配置寄存器
#define ACCEL_CONFIG_REG 0x1C       // 加速度计配置寄存器

// 添加I2C重试机制
#define I2C_RETRY_COUNT 3
#define I2C_TIMEOUT_MS  50

int16_t Accel_X_RAW, Accel_Y_RAW, Accel_Z_RAW;
int16_t Gyro_X_RAW, Gyro_Y_RAW, Gyro_Z_RAW;
int16_t Temp_RAW;

// 改进的I2C读取函数，带重试机制
HAL_StatusTypeDef MPU6050_I2C_Read(uint16_t MemAddress, uint8_t *pData, uint16_t Size) {
    HAL_StatusTypeDef status;
    uint8_t retry_count = 0;
    
    for(retry_count = 0; retry_count < I2C_RETRY_COUNT; retry_count++) {
        status = HAL_I2C_Mem_Read(&hi2c1, MPU6050_ADDR, MemAddress, 
                                I2C_MEMADD_SIZE_8BIT, pData, Size, I2C_TIMEOUT_MS);
        
        if(status == HAL_OK) {
            return HAL_OK;
        }
        
        // 重试前短暂延时并重置I2C
        HAL_Delay(1);
        if(retry_count == 1) {
            // 第二次重试时尝试重置I2C
            HAL_I2C_DeInit(&hi2c1);
            HAL_I2C_Init(&hi2c1);
        }
    }
    
    return HAL_ERROR;
}

// 改进的I2C写入函数，带重试机制
HAL_StatusTypeDef MPU6050_I2C_Write(uint16_t MemAddress, uint8_t *pData, uint16_t Size) {
    HAL_StatusTypeDef status;
    uint8_t retry_count = 0;
    
    for(retry_count = 0; retry_count < I2C_RETRY_COUNT; retry_count++) {
        status = HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, MemAddress, 
                                 I2C_MEMADD_SIZE_8BIT, pData, Size, I2C_TIMEOUT_MS);
        
        if(status == HAL_OK) {
            return HAL_OK;
        }
        
        HAL_Delay(1);
        if(retry_count == 1) {
            HAL_I2C_DeInit(&hi2c1);
            HAL_I2C_Init(&hi2c1);
        }
    }
    
    return HAL_ERROR;
}

/* 初始化 MPU6050 */
HAL_StatusTypeDef MPU6050_Init(void) {
    uint8_t check, data;
    HAL_StatusTypeDef res;
    
    // 1. 读取 WHO_AM_I 寄存器，检查设备ID是否正确 (0x68)
    res = MPU6050_I2C_Read(WHO_AM_I_REG, &check, 1);
    if(res != HAL_OK || check != 0x68) {
        return HAL_ERROR;  // 通信失败或ID不符
    }
    
    // 2. 解除休眠，将 PWR_MGMT_1 寄存器写0
    data = 0x00;
    MPU6050_I2C_Write(PWR_MGMT_1_REG, &data, 1);
    HAL_Delay(100);  // 延长等待时间，确保芯片完全唤醒
    
    // 3. 设置采样率分频器 SMPLRT_DIV (设置成7获得1kHz采样率)
    data = 0x07;
    MPU6050_I2C_Write(SMPLRT_DIV_REG, &data, 1);
    
    // 4. 配置DLPF，设置数字低通滤波器 (0x06, Accel带宽5Hz, Gyro带宽5Hz)
    data = 0x06;  // 降低带宽以减少噪声
    MPU6050_I2C_Write(CONFIG_REG, &data, 1);
    
    // 5. 配置陀螺仪满量程范围 ±500°/s (0x08) 
    data = 0x08;
    MPU6050_I2C_Write(GYRO_CONFIG_REG, &data, 1);
    
    // 6. 配置加速度计满量程范围 ±4g (0x08)
    data = 0x08;
    MPU6050_I2C_Write(ACCEL_CONFIG_REG, &data, 1);
    
    HAL_Delay(50); // 等待配置稳定
    
    return HAL_OK;
}

void MPU6050_Read_Accel(void) {
    uint8_t buf[6];
    if(MPU6050_I2C_Read(0x3B, buf, 6) == HAL_OK) {
        // 拼接高低字节为 16 位有符号值
        Accel_X_RAW = (int16_t)(buf[0] << 8 | buf[1]);
        Accel_Y_RAW = (int16_t)(buf[2] << 8 | buf[3]);
        Accel_Z_RAW = (int16_t)(buf[4] << 8 | buf[5]);
    } else {
        // 读取失败时保持原值
    }
}
 
// 读取陀螺仪三轴原始数据
void MPU6050_Read_Gyro(void) {
    uint8_t buf[6];
    if(MPU6050_I2C_Read(0x43, buf, 6) == HAL_OK) {
        Gyro_X_RAW = (int16_t)(buf[0] << 8 | buf[1]);
        Gyro_Y_RAW = (int16_t)(buf[2] << 8 | buf[3]);
        Gyro_Z_RAW = (int16_t)(buf[4] << 8 | buf[5]);
    } else {
        // 读取失败时保持原值
    }
}
 
void MPU6050_Read_Temp(void) {
    uint8_t buf[2];
    if(MPU6050_I2C_Read(0x41, buf, 2) == HAL_OK) {
        Temp_RAW = (int16_t)(buf[0] << 8 | buf[1]);
    } else {
        // 读取失败时保持原值
    }
}

void MPU6050_Data_Update(void){
    MPU6050_Read_Accel();
    MPU6050_Read_Gyro();
    MPU6050_Read_Temp();
    
    // 转换为物理量，注意量程变化
    // 加速度计 ±4g 量程: 16384 LSB/g -> 8192 LSB/g
    Ax = Accel_X_RAW / 8192.0f;
    Ay = Accel_Y_RAW / 8192.0f;
    Az = Accel_Z_RAW / 8192.0f;
    
    // 陀螺仪 ±500°/s 量程: 65.5 LSB/°/s
    Gx = Gyro_X_RAW / 65.5f;
    Gy = Gyro_Y_RAW / 65.5f;
    Gz = Gyro_Z_RAW / 65.5f;
    
    Temperature = Temp_RAW / 340.0f + 36.53f;
    MPU_flag = 1;
}
