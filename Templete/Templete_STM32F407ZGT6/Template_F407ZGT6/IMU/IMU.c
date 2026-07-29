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
