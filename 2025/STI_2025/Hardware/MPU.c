#include "MPU.h"
#include "math.h"

// 全局变量
int MPU_flag = 0;
float Ax, Ay, Az;
float Gx, Gy, Gz;
float Temperature;
MPUData_t sensor_data;
Angle_t current_angle;
int turning_flag;
int flag = 0;

/* MPU6050 寄存器定义 */
#define WHO_AM_I_REG    0x75
#define PWR_MGMT_1_REG  0x6B
#define SMPLRT_DIV_REG  0x19
#define CONFIG_REG      0x1A
#define GYRO_CONFIG_REG 0x1B
#define ACCEL_CONFIG_REG 0x1C

int16_t Accel_X_RAW, Accel_Y_RAW, Accel_Z_RAW;
int16_t Gyro_X_RAW, Gyro_Y_RAW, Gyro_Z_RAW;
int16_t Temp_RAW;

typedef struct {
    float accel_temp_coeff[3];
    float gyro_temp_coeff[3];
    float accel_offset[3];
    float gyro_offset[3];
    float learning_rate_temp;
    float learning_rate_offset;
    uint32_t calibration_count;
    uint8_t is_calibrating;
    float temp_history[10];
    float accel_history[3][10];
    float gyro_history[3][10];
    uint8_t history_index;
    float ref_temp;
} AdaptiveCompensation_t;

AdaptiveCompensation_t adaptive_comp;

// 数据滤波变量
float Ax_filtered = 0, Ay_filtered = 0, Az_filtered = 0;
float Gx_filtered = 0, Gy_filtered = 0, Gz_filtered = 0;
const float FILTER_ALPHA = 0.2f;

// 数据有效性检查
uint8_t data_valid = 0;
float prev_Ax = 0, prev_Ay = 0, prev_Az = 0;
float prev_Gx = 0, prev_Gy = 0, prev_Gz = 0;

// 静止检测
uint32_t stationary_count = 0;
const uint32_t STATIONARY_THRESHOLD = 100;

/* USER CODE BEGIN 0 */
#ifndef USE_HARDWARE_I2C
// 软件I2C基础函数实现（静态函数，只在当前文件可见）
static void I2C_Start(void)
{
    SDA_HIGH();
    SCL_HIGH();
    I2C_DELAY();
    SDA_LOW();
    I2C_DELAY();
    SCL_LOW();
}

static void I2C_Stop(void)
{
    SDA_LOW();
    SCL_HIGH();
    I2C_DELAY();
    SDA_HIGH();
    I2C_DELAY();
}

static uint8_t I2C_Wait_Ack(void)
{
    uint8_t ack;
    
    SDA_HIGH();
    SCL_HIGH();
    I2C_DELAY();
    
    ack = SDA_READ();
    
    SCL_LOW();
    I2C_DELAY();
    
    return (ack == 0);
}

static void I2C_Ack(void)
{
    SDA_LOW();
    SCL_HIGH();
    I2C_DELAY();
    SCL_LOW();
    I2C_DELAY();
    SDA_HIGH();
}

static void I2C_NAck(void)
{
    SDA_HIGH();
    SCL_HIGH();
    I2C_DELAY();
    SCL_LOW();
    I2C_DELAY();
}

static uint8_t I2C_SendByte(uint8_t data)
{
    uint8_t i;
    
    for(i = 0; i < 8; i++)
    {
        if(data & 0x80)
            SDA_HIGH();
        else
            SDA_LOW();
        
        SCL_HIGH();
        I2C_DELAY();
        SCL_LOW();
        I2C_DELAY();
        
        data <<= 1;
    }
    
    return I2C_Wait_Ack();
}

static uint8_t I2C_ReadByte(uint8_t ack)
{
    uint8_t i, data = 0;
    
    SDA_HIGH();
    
    for(i = 0; i < 8; i++)
    {
        data <<= 1;
        SCL_HIGH();
        I2C_DELAY();
        
        if(SDA_READ())
            data |= 0x01;
            
        SCL_LOW();
        I2C_DELAY();
    }
    
    if(ack)
        I2C_Ack();
    else
        I2C_NAck();
    
    return data;
}

// 软件I2C初始化
void Software_I2C_Init(void)
{
    // 引脚已在CubeMX中配置为开漏输出
    SCL_HIGH();
    SDA_HIGH();
}

// 软件I2C读取函数（带重试机制）
HAL_StatusTypeDef MPU6050_I2C_Read(uint16_t MemAddress, uint8_t *pData, uint16_t Size)
{
    uint8_t retry_count = 0;
    uint8_t i;
    
    for(retry_count = 0; retry_count < I2C_RETRY_COUNT; retry_count++)
    {
        // 发送起始条件
        I2C_Start();
        
        // 发送设备地址(写模式) + 等待应答
        if(!I2C_SendByte(MPU6050_ADDR << 1))
        {
            I2C_Stop();
            continue;  // 无应答，重试
        }
        
        // 发送寄存器地址 + 等待应答
        if(!I2C_SendByte((uint8_t)MemAddress))
        {
            I2C_Stop();
            continue;  // 无应答，重试
        }
        
        // 重新启动以开始读取
        I2C_Start();
        
        // 发送设备地址(读模式) + 等待应答
        if(!I2C_SendByte((MPU6050_ADDR << 1) | 0x01))
        {
            I2C_Stop();
            continue;  // 无应答，重试
        }
        
        // 读取数据
        for(i = 0; i < Size; i++)
        {
            if(i == Size - 1)
                pData[i] = I2C_ReadByte(0);  // 最后一个字节发送NACK
            else
                pData[i] = I2C_ReadByte(1);  // 发送ACK
        }
        
        // 发送停止条件
        I2C_Stop();
        
        return HAL_OK;  // 读取成功
    }
    
    return HAL_ERROR;  // 所有重试都失败
}

// 软件I2C写入函数（带重试机制）
HAL_StatusTypeDef MPU6050_I2C_Write(uint16_t MemAddress, uint8_t *pData, uint16_t Size)
{
    uint8_t retry_count = 0;
    uint8_t i;
    
    for(retry_count = 0; retry_count < I2C_RETRY_COUNT; retry_count++)
    {
        // 发送起始条件
        I2C_Start();
        
        // 发送设备地址(写模式) + 等待应答
        if(!I2C_SendByte(MPU6050_ADDR << 1))
        {
            I2C_Stop();
            HAL_Delay(1);
            continue;  // 无应答，重试
        }
        
        // 发送寄存器地址 + 等待应答
        if(!I2C_SendByte((uint8_t)MemAddress))
        {
            I2C_Stop();
            HAL_Delay(1);
            continue;  // 无应答，重试
        }
        
        // 发送数据
        for(i = 0; i < Size; i++)
        {
            if(!I2C_SendByte(pData[i]))
            {
                I2C_Stop();
                HAL_Delay(1);
                break;  // 发送失败，跳出循环重试
            }
        }
        
        // 如果所有数据都发送成功
        if(i == Size)
        {
            I2C_Stop();
            return HAL_OK;  // 写入成功
        }
    }
    
    return HAL_ERROR;  // 所有重试都失败
}

#else
// 硬件I2C函数实现
HAL_StatusTypeDef MPU6050_I2C_Read(uint16_t MemAddress, uint8_t *pData, uint16_t Size)
{
    HAL_StatusTypeDef status;
    uint8_t retry_count = 0;
    
    for(retry_count = 0; retry_count < I2C_RETRY_COUNT; retry_count++)
    {
        status = HAL_I2C_Mem_Read(&hi2c1, MPU6050_ADDR << 1, MemAddress, 
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

HAL_StatusTypeDef MPU6050_I2C_Write(uint16_t MemAddress, uint8_t *pData, uint16_t Size)
{
    HAL_StatusTypeDef status;
    uint8_t retry_count = 0;
    
    for(retry_count = 0; retry_count < I2C_RETRY_COUNT; retry_count++)
    {
        status = HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR << 1, MemAddress, 
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
#endif
/* USER CODE END 0 */

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

MPUData_t MPU6050_Data_Update(void){
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
	  MPUData_t sensor_data = process_MPUdata();
	  return sensor_data;
}

void init_adaptive_compensation(void) {
    // 初始化温度补偿系数
    adaptive_comp.accel_offset[0] = 0.101f;
    adaptive_comp.accel_offset[1] = 0.001f;
    
    // 精确调整（你可以修改这些增量值）
    float az_increment = -0.180f;   // 负值表示减小
		float gx_increment = 0.648f;    // 正值表示减小负值（这个不变）
		float gz_increment = -0.100f;   // 负值表示减小

		adaptive_comp.accel_offset[2] = 0.100f + az_increment;  // 0.100 + (-0.180) = -0.080
		adaptive_comp.gyro_offset[0] = -5.0f;    // 根据Gx ≈ -5°/s
    adaptive_comp.gyro_offset[1] = -1.15f;   // 根据Gy ≈ -1.15°/s
    adaptive_comp.gyro_offset[2] = -1.3f;    // 根据Gz ≈ -1.3°/s
    // 温度补偿系数保持不变
    adaptive_comp.accel_temp_coeff[0] = 0.0005f;
    adaptive_comp.accel_temp_coeff[1] = 0.0005f;
    adaptive_comp.accel_temp_coeff[2] = 0.0006f;
    adaptive_comp.gyro_temp_coeff[0] = 0.008f;
    adaptive_comp.gyro_temp_coeff[1] = 0.008f;
    adaptive_comp.gyro_temp_coeff[2] = 0.009f;
    
    adaptive_comp.learning_rate_temp = 0.001f;
    adaptive_comp.learning_rate_offset = 0.005f;
    adaptive_comp.calibration_count = 500;
    adaptive_comp.is_calibrating = 1;
    adaptive_comp.history_index = 0;
    adaptive_comp.ref_temp = 25.0f;
    
    // 清空历史数据
    for(int i = 0; i < 10; i++) {
        adaptive_comp.temp_history[i] = 25.0f;
        for(int j = 0; j < 3; j++) {
            adaptive_comp.accel_history[j][i] = 0.0f;
            adaptive_comp.gyro_history[j][i] = 0.0f;
        }
    }
}

// 静止检测函数
uint8_t is_stationary(float ax, float ay, float az, float gx, float gy, float gz) {
    // 检查加速度计模长是否接近1g
    float accel_magnitude = sqrtf(ax*ax + ay*ay + az*az);
    if(fabsf(accel_magnitude - 1.0f) > 0.1f) {
        return 0;
    }
    
    // 检查陀螺仪是否接近零
    float gyro_magnitude = sqrtf(gx*gx + gy*gy + gz*gz);
    if(gyro_magnitude > 2.0f) { // 2°/s阈值
        return 0;
    }
    
    return 1;
}

// 更新自适应补偿参数
void update_adaptive_compensation(float temp, float ax, float ay, float az, float gx, float gy, float gz) {
    // 更新历史数据
    adaptive_comp.temp_history[adaptive_comp.history_index] = temp;
    adaptive_comp.accel_history[0][adaptive_comp.history_index] = ax;
    adaptive_comp.accel_history[1][adaptive_comp.history_index] = ay;
    adaptive_comp.accel_history[2][adaptive_comp.history_index] = az;
    adaptive_comp.gyro_history[0][adaptive_comp.history_index] = gx;
    adaptive_comp.gyro_history[1][adaptive_comp.history_index] = gy;
    adaptive_comp.gyro_history[2][adaptive_comp.history_index] = gz;
    
    adaptive_comp.history_index = (adaptive_comp.history_index + 1) % 10;
    
    // 静止状态下更新零偏估计
    if(is_stationary(ax, ay, az, gx, gy, gz)) {
        stationary_count++;
        
        if(stationary_count > STATIONARY_THRESHOLD) {
            // 更新加速度计零偏（假设Z轴为1g）
            adaptive_comp.accel_offset[0] += adaptive_comp.learning_rate_offset * ax;
            adaptive_comp.accel_offset[1] += adaptive_comp.learning_rate_offset * ay;
            adaptive_comp.accel_offset[2] += adaptive_comp.learning_rate_offset * (az - 1.0f);
            
            // 更新陀螺仪零偏
            adaptive_comp.gyro_offset[0] += adaptive_comp.learning_rate_offset * gx;
            adaptive_comp.gyro_offset[1] += adaptive_comp.learning_rate_offset * gy;
            adaptive_comp.gyro_offset[2] += adaptive_comp.learning_rate_offset * gz;
            
            stationary_count = STATIONARY_THRESHOLD; // 防止溢出
            
            // 温度补偿参数学习（需要足够的温度变化）
            if(adaptive_comp.calibration_count < 1000) {
                adaptive_comp.calibration_count++;
            }
        }
    } else {
        stationary_count = 0;
    }
    
    // 温度补偿参数学习（简化版本）
    if(adaptive_comp.is_calibrating && adaptive_comp.calibration_count < 1000) {
        // 这里可以添加更复杂的温度模型学习算法
        // 例如基于历史数据的线性回归等
    }
}

// 应用自适应补偿
void apply_adaptive_compensation(float temp, float *ax, float *ay, float *az, float *gx, float *gy, float *gz) {
    float temp_diff = temp - adaptive_comp.ref_temp;
    
    // 应用温度补偿和零偏补偿
    *ax = (*ax - adaptive_comp.accel_offset[0]) * (1.0f - adaptive_comp.accel_temp_coeff[0] * temp_diff);
    *ay = (*ay - adaptive_comp.accel_offset[1]) * (1.0f - adaptive_comp.accel_temp_coeff[1] * temp_diff);
    *az = (*az - adaptive_comp.accel_offset[2]) * (1.0f - adaptive_comp.accel_temp_coeff[2] * temp_diff);
    
    *gx = (*gx - adaptive_comp.gyro_offset[0]) * (1.0f - adaptive_comp.gyro_temp_coeff[0] * temp_diff);
    *gy = (*gy - adaptive_comp.gyro_offset[1]) * (1.0f - adaptive_comp.gyro_temp_coeff[1] * temp_diff);
    *gz = (*gz - adaptive_comp.gyro_offset[2]) * (1.0f - adaptive_comp.gyro_temp_coeff[2] * temp_diff);
}

// 应用低通滤波
void apply_low_pass_filter(float *filtered, float new_value, float alpha) {
    *filtered = alpha * new_value + (1.0f - alpha) * (*filtered);
}

// 数据有效性检查
uint8_t is_data_valid(float ax, float ay, float az, float gx, float gy, float gz) {
    // 检查数据是否在合理范围内
    if (fabsf(ax) > 4.0f || fabsf(ay) > 4.0f || fabsf(az) > 4.0f) return 0;
    if (fabsf(gx) > 500.0f || fabsf(gy) > 500.0f || fabsf(gz) > 500.0f) return 0;
    
    // 检查数据突变（简单的变化率检查）
    float accel_change = fabsf(ax - prev_Ax) + fabsf(ay - prev_Ay) + fabsf(az - prev_Az);
    float gyro_change = fabsf(gx - prev_Gx) + fabsf(gy - prev_Gy) + fabsf(gz - prev_Gz);
    
    if (accel_change > 2.0f || gyro_change > 100.0f) return 0;
    
    return 1;
}

// 改进的数据处理函数
MPUData_t process_MPUdata(void) {
    MPUData_t data;
    
    if(MPU_flag == 1) {
        MPU_flag = 0;
        
        // 更新自适应补偿参数
        update_adaptive_compensation(Temperature, Ax, Ay, Az, Gx, Gy, Gz);
        
        // 应用自适应补偿
        apply_adaptive_compensation(Temperature, &Ax, &Ay, &Az, &Gx, &Gy, &Gz);
        
        // 检查数据有效性
        if(is_data_valid(Ax, Ay, Az, Gx, Gy, Gz)) {
            data_valid = 1;
            
            // 应用低通滤波
            apply_low_pass_filter(&Ax_filtered, Ax, FILTER_ALPHA);
            apply_low_pass_filter(&Ay_filtered, Ay, FILTER_ALPHA);
            apply_low_pass_filter(&Az_filtered, Az, FILTER_ALPHA);
            apply_low_pass_filter(&Gx_filtered, Gx, FILTER_ALPHA);
            apply_low_pass_filter(&Gy_filtered, Gy, FILTER_ALPHA);
            apply_low_pass_filter(&Gz_filtered, Gz, FILTER_ALPHA);
            
            // 更新历史数据
            prev_Ax = Ax; prev_Ay = Ay; prev_Az = Az;
            prev_Gx = Gx; prev_Gy = Gy; prev_Gz = Gz;
        } else {
            data_valid = 0;
        }
    }
    
    // 赋值输出数据
    data.Temperature_ = Temperature;
    if(Ax_filtered >-0.1&&Ax_filtered < 0.1) Ax_filtered =0;
		if(Ay_filtered >-0.1&&Ay_filtered < 0.1) Ay_filtered =0;
		if(Az_filtered >-1.1&&Az_filtered < -0.9) Az_filtered =-1;
		if(Gx_filtered >-1&&Gx_filtered < 1) Gx_filtered =0;
		if(Gy_filtered >-1&&Gy_filtered < 1) Gy_filtered =0;
		if(Gz_filtered >-1&&Gz_filtered < 1) Gz_filtered =0;
    if(data_valid) {
        data.Ax_ = Ax_filtered;
        data.Ay_ = Ay_filtered;
        data.Az_ = Az_filtered;
        data.Gx_ = Gx_filtered;
        data.Gy_ = Gy_filtered;
        data.Gz_ = Gz_filtered;
    } else {
        // 数据无效时输出滤波后的值
        data.Ax_ = Ax_filtered;
        data.Ay_ = Ay_filtered;
        data.Az_ = Az_filtered;
        data.Gx_ = Gx_filtered;
        data.Gy_ = Gy_filtered;
        data.Gz_ = Gz_filtered;
    }
    
    return data;
}

void calculate_angle_from_gyro(float gx, float gy, float gz, float dt) {
    // 积分计算角度变化
    current_angle.pitch += gx * dt;  // 绕X轴旋转影响pitch
    current_angle.roll += gy * dt;   // 绕Y轴旋转影响roll
    current_angle.yaw += gz * dt;    // 绕Z轴旋转影响yaw
}

void turning_state_judge(MPUData_t *data){
	if(data->Gz_>1&&flag == 0){
		current_angle.yaw = 0;
		flag = 1;
	}
	else if(data->Gz_<1&&flag == 1){
		flag = 0;
		turning_flag = 0;
	}
	else if(data->Gz_>1&&flag == 1){
		if(current_angle.yaw > 20) {
			turning_flag = 1;
		}
	}
}