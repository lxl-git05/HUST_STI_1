#include "MPU6050_Mahony.h"
#include "math.h"

// ==================== 全局输出 ====================
ImuReal_Typedef MPU_Mahony_Real;

// ==================== 四元数 (w + xi + yj + zk Hamilton 约定) ====================
static float q0 = 1.0f, q1 = 0.0f, q2 = 0.0f, q3 = 0.0f;

// ==================== PI 积分误差 ====================
static float exInt = 0.0f, eyInt = 0.0f, ezInt = 0.0f;

// ==================== 绝对累计偏航角 ====================
static float yaw_abs  = 0.0f;       // 顺时针持续增大，无跳变
static float yaw_prev = 0.0f;       // 上一帧 yaw，用于跳变检测

// ==================== 【陀螺零偏 (deg/s) —— extern，供 AT24C02 读写 和手动编写】 ====================
float MPU_Mahony_GyroBiasX = 0.0f;
float MPU_Mahony_GyroBiasY = 0.0f;
float MPU_Mahony_GyroBiasZ = 0.0f;

// ==================== 角度转换常量 ====================
static const float DEG2RAD = 0.01745329252f;   // PI / 180
static const float RAD2DEG = 57.2957795131f;   // 180 / PI

// ==================== 初始化 ====================
// doCalib=1: 自动标定零偏（原地静止采样）
// doCalib=0: 跳过标定，使用 MPU_Mahony_GyroBiasX/Y/Z 当前值
void MPU6050_Mahony_Init(uint8_t doCalib)
{
    MPU6050_Init();

    // --- 条件标定陀螺零偏 ---
    if (doCalib)
    {
        float sum_gx = 0.0f, sum_gy = 0.0f, sum_gz = 0.0f;

        for (int i = 0; i < MPU_MAHONY_CALIB_SAMPLES; i++)
        {
            MPU6050_Update_Data();
            sum_gx += MPU_Raw_Data.GX;
            sum_gy += MPU_Raw_Data.GY;
            sum_gz += MPU_Raw_Data.GZ;
        }

        MPU_Mahony_GyroBiasX = sum_gx / MPU_MAHONY_CALIB_SAMPLES;
        MPU_Mahony_GyroBiasY = sum_gy / MPU_MAHONY_CALIB_SAMPLES;
        MPU_Mahony_GyroBiasZ = sum_gz / MPU_MAHONY_CALIB_SAMPLES;
    }
    // doCalib=0 时直接使用 MPU_Mahony_GyroBiasX/Y/Z 现有值
    //   → 来自 #define 默认值 或 AT24C02 恢复值（用户自行在 Init 前写入）

    // --- 四元数复位 ---
    q0 = 1.0f;  q1 = 0.0f;  q2 = 0.0f;  q3 = 0.0f;

    // --- PI 积分复位 ---
    exInt = 0.0f;  eyInt = 0.0f;  ezInt = 0.0f;

    // --- 绝对 yaw 复位 ---
    yaw_abs  = 0.0f;
    yaw_prev = 0.0f;

    // --- 输出清零 ---
    MPU_Mahony_Real.roll  = 0.0f;
    MPU_Mahony_Real.pitch = 0.0f;
    MPU_Mahony_Real.yaw   = 0.0f;
    MPU_Mahony_Real.AccX  = 0.0f;
    MPU_Mahony_Real.AccY  = 0.0f;
    MPU_Mahony_Real.AccZ  = 1.0f;
}

// ==================== 手动重新标定零偏 ====================
void MPU6050_Mahony_Calibrate(int samples)
{
    if (samples < 100 || samples > 2000)
        samples = MPU_MAHONY_CALIB_SAMPLES;

    float sum_gx = 0.0f, sum_gy = 0.0f, sum_gz = 0.0f;

    for (int i = 0; i < samples; i++)
    {
        MPU6050_Update_Data();
        sum_gx += MPU_Raw_Data.GX;
        sum_gy += MPU_Raw_Data.GY;
        sum_gz += MPU_Raw_Data.GZ;
    }

    MPU_Mahony_GyroBiasX = sum_gx / samples;
    MPU_Mahony_GyroBiasY = sum_gy / samples;
    MPU_Mahony_GyroBiasZ = sum_gz / samples;

    // 标定后重置角度
    q0 = 1.0f;  q1 = 0.0f;  q2 = 0.0f;  q3 = 0.0f;
    exInt = 0.0f;  eyInt = 0.0f;  ezInt = 0.0f;
    yaw_abs  = 0.0f;
    yaw_prev = 0.0f;
    MPU_Mahony_Real.roll  = 0.0f;
    MPU_Mahony_Real.pitch = 0.0f;
    MPU_Mahony_Real.yaw   = 0.0f;
}

// ==================== Mahony AHRS 单步更新 ====================
// dt: 采样间隔，单位秒 (20ms → 0.020f)
static void MPU6050_Mahony_Update(float dt)
{
    float recipNorm;
    float halfT = dt * 0.5f;

    // ---- 1. 陀螺仪去零偏 + 转为 rad/s ----
    float gx = (MPU_Raw_Data.GX - MPU_Mahony_GyroBiasX) * DEG2RAD;
    float gy = (MPU_Raw_Data.GY - MPU_Mahony_GyroBiasY) * DEG2RAD;
    float gz = (MPU_Raw_Data.GZ - MPU_Mahony_GyroBiasZ) * DEG2RAD;

    // ---- 2. 加速度归一化 ----
    float ax = MPU_Raw_Data.AX;
    float ay = MPU_Raw_Data.AY;
    float az = MPU_Raw_Data.AZ;
    recipNorm = ax * ax + ay * ay + az * az;
    if (recipNorm < 1e-12f) return;
    recipNorm = 1.0f / sqrtf(recipNorm);
    ax *= recipNorm;  ay *= recipNorm;  az *= recipNorm;

    // ---- 3. 重力方向在四元数中的投影 ----
    // 世界系参考重力 [0, 0, 1] 经 q 旋转到机体系
    float vx = 2.0f * (q1 * q3 - q0 * q2);
    float vy = 2.0f * (q0 * q1 + q2 * q3);
    float vz = q0 * q0 - q1 * q1 - q2 * q2 + q3 * q3;

    // ---- 4. 叉积求误差 ----
    float ex = ay * vz - az * vy;
    float ey = az * vx - ax * vz;
    float ez = ax * vy - ay * vx;

    // ---- 5. PI 修正 ----
    if (MPU_MAHONY_KI > 0.0f)
    {
        exInt += ex * MPU_MAHONY_KI * halfT;
        eyInt += ey * MPU_MAHONY_KI * halfT;
        ezInt += ez * MPU_MAHONY_KI * halfT;
    }
    else
    {
        exInt = 0.0f;  eyInt = 0.0f;  ezInt = 0.0f;
    }

    gx += MPU_MAHONY_KP * ex + exInt;
    gy += MPU_MAHONY_KP * ey + eyInt;
    gz += MPU_MAHONY_KP * ez + ezInt;

    // ---- 6. 四元数积分（一阶 Runge-Kutta） ----
    q0 += (-q1 * gx - q2 * gy - q3 * gz) * halfT;
    q1 += ( q0 * gx + q2 * gz - q3 * gy) * halfT;
    q2 += ( q0 * gy - q1 * gz + q3 * gx) * halfT;
    q3 += ( q0 * gz + q1 * gy - q2 * gx) * halfT;

    // ---- 7. 四元数归一化 ----
    recipNorm = q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3;
    if (recipNorm < 1e-12f) return;
    recipNorm = 1.0f / sqrtf(recipNorm);
    q0 *= recipNorm;  q1 *= recipNorm;
    q2 *= recipNorm;  q3 *= recipNorm;

    // ---- 8. 欧拉角提取 (degrees) ----
    // Roll:  ±180° via atan2
    MPU_Mahony_Real.roll  = atan2f(2.0f * (q0 * q1 + q2 * q3),
                                    1.0f - 2.0f * (q1 * q1 + q2 * q2)) * RAD2DEG;
    // Pitch: ±90° via asinf
    MPU_Mahony_Real.pitch = asinf(2.0f * (q0 * q2 - q3 * q1)) * RAD2DEG;
    // Yaw:   ±180° via atan2
    MPU_Mahony_Real.yaw   = atan2f(2.0f * (q0 * q3 + q1 * q2),
                                    1.0f - 2.0f * (q2 * q2 + q3 * q3)) * RAD2DEG;

    // ---- 8.5 绝对累计 yaw（解绕，顺时针持续增大无跳变） ----
    {
        float delta = MPU_Mahony_Real.yaw - yaw_prev;
        if (delta >  180.0f) delta -= 360.0f;
        if (delta < -180.0f) delta += 360.0f;
        yaw_abs  -= delta;      // 取反：使顺时针 = 增大
        yaw_prev  = MPU_Mahony_Real.yaw;
    }

    // ---- 9. 归一化加速度输出 ----
    MPU_Mahony_Real.AccX = ax;
    MPU_Mahony_Real.AccY = ay;
    MPU_Mahony_Real.AccZ = az;
}

// ==================== 外部 Tick 入口 ====================
void MPU6050_Mahony_Update_Tick(void)
{
    MPU6050_Update_Data();
    MPU6050_Mahony_Update(0.020f);
}

// ==================== 绝对累计偏航角 ====================
// 顺时针持续增大，无 ±180° 跳变，可超过 360°
float MPU_Yaw_Abs_Get(void)
{
    return yaw_abs;
}

// 归零 yaw_abs（不影响当前 yaw 和四元数解算）
void MPU_Yaw_Abs_Reset(void)
{
    yaw_abs  = 0.0f;
    yaw_prev = MPU_Mahony_Real.yaw;   // 避免下一帧产生伪跳变
}
