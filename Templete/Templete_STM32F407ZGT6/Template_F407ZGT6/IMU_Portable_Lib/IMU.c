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
