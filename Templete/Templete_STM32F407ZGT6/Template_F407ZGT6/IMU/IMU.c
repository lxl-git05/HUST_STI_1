#include "IMU.h"
#include "math.h"

// ==================== 偏航角到位检测 ====================

// target: 目标 yaw_abs 角度 (°)
// deadband: 死区宽度 (°)，内算到位
uint8_t IMU_Turn_Yaw_Is_Ok_Ex(float target, float deadband)
{
    float yaw_now = IMU_Yaw_Abs_Get();
    float err     = yaw_now - target;

    if (err < 0.0f) err = -err;     // |error|

    return (err <= deadband) ? 1 : 0;
}

// 使用默认死区
uint8_t IMU_Turn_Yaw_Is_Ok(float target)
{
    return IMU_Turn_Yaw_Is_Ok_Ex(target, IMU_TURN_YAW_DEFAULT_DEADBAND);
}

// 获取当前校准后的Z轴角速度绝对值 (°/s)
float IMU_Yaw_Gyro_Get(void)
{
    float raw_gz;
    #ifdef IMU_USE_MPU6050
        raw_gz = MPU_Raw_Data.GZ;
    #else
        raw_gz = ICM_Raw_Data.GZ;
    #endif
    float cal_gz = raw_gz - IMU_Mahony_GyroBiasZ;
    return (cal_gz < 0.0f) ? -cal_gz : cal_gz;
}

// ==================== 世界系水平加速度 ====================
// 将机体加速度旋转到世界系: R_y(pitch) * R_x(roll) * [ax, ay, az]^T
// 世界系Z轴为重力方向，X/Y轴自然不含重力分量

static const float DEG2RAD_IMU = 0.01745329252f;	// PI / 180

float IMU_Get_Ax(void)
{
    float ax, ay, az;
    #ifdef IMU_USE_MPU6050
        ax = MPU_Raw_Data.AX - MPU_Mahony_AccBiasX;
        ay = MPU_Raw_Data.AY - MPU_Mahony_AccBiasY;
        az = MPU_Raw_Data.AZ - MPU_Mahony_AccBiasZ;
    #else
        ax = ICM_Raw_Data.AX - ICM_Mahony_AccBiasX;
        ay = ICM_Raw_Data.AY - ICM_Mahony_AccBiasY;
        az = ICM_Raw_Data.AZ - ICM_Mahony_AccBiasZ;
    #endif

    float roll_rad  = IMU_Mahony_Real.roll  * DEG2RAD_IMU;
    float pitch_rad = IMU_Mahony_Real.pitch * DEG2RAD_IMU;

    float cr = cosf(roll_rad);
    float sr = sinf(roll_rad);
    float cp = cosf(pitch_rad);
    float sp = sinf(pitch_rad);

    // world_x = ax*cp + ay*sr*sp + az*cr*sp
    return ax * cp + ay * sr * sp + az * cr * sp;
}

float IMU_Get_Ay(void)
{
    float ay, az;
    #ifdef IMU_USE_MPU6050
        ay = MPU_Raw_Data.AY - MPU_Mahony_AccBiasY;
        az = MPU_Raw_Data.AZ - MPU_Mahony_AccBiasZ;
    #else
        ay = ICM_Raw_Data.AY - ICM_Mahony_AccBiasY;
        az = ICM_Raw_Data.AZ - ICM_Mahony_AccBiasZ;
    #endif

    float roll_rad  = IMU_Mahony_Real.roll  * DEG2RAD_IMU;

    float cr = cosf(roll_rad);
    float sr = sinf(roll_rad);

    // world_y = ay*cr - az*sr
    return ay * cr - az * sr;
}
