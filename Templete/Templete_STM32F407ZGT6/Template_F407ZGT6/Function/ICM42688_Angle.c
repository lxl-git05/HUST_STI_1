#include "ICM42688_Angle.h"
#include "math.h"

// ==================== 全局变量 ====================
ImuOffset_Typedef  ICM_Offset;
ImuCali_Typedef    ICM_Cali;
ImuReal_Typedef    ICM_Real;

// ==================== 积分角度 ====================
static float gyroAngleX = 0.0f;
static float gyroAngleY = 0.0f;

void ICM42688_Angle_Init(void)
{
    ICM42688_Init();

    // 零偏误差配置
    ICM_Offset.AccErrorX =  0.0477302261f;
    ICM_Offset.AccErrorY = -0.0052581788;
    ICM_Offset.AccErrorZ = -0.496570051f;
    ICM_Offset.GyroErrorX = 0.0411145799f;
    ICM_Offset.GyroErrorY = 0.126367226f;
    ICM_Offset.GyroErrorZ = 0.0512673371f;

    // 角度清零
    gyroAngleX = 0.0f;
    gyroAngleY = 0.0f;

    ICM_Real.roll  = 0.0f;
    ICM_Real.pitch = 0.0f;
    ICM_Real.yaw   = 0.0f;
    ICM_Real.AccX  = 0.0f;
    ICM_Real.AccY  = 0.0f;
    ICM_Real.AccZ  = 1.0f;
}

// ==================== 手动零偏标定 ====================
void ICM42688_Data_Error_Check(int Sample_Cnt)
{
    if (Sample_Cnt < 200 || Sample_Cnt > 2000)
        Sample_Cnt = 1000;

    float sum_ax = 0.0f, sum_ay = 0.0f, sum_az = 0.0f;
    float sum_gx = 0.0f, sum_gy = 0.0f, sum_gz = 0.0f;

    for (int i = 0; i < Sample_Cnt; i++)
    {
        ICM42688_Update_Data();

        sum_ax += ICM_Raw_Data.AX;
        sum_ay += ICM_Raw_Data.AY;
        sum_az += ICM_Raw_Data.AZ;
        sum_gx += ICM_Raw_Data.GX;
        sum_gy += ICM_Raw_Data.GY;
        sum_gz += ICM_Raw_Data.GZ;
    }

    ICM_Offset.AccErrorX = sum_ax / Sample_Cnt;
    ICM_Offset.AccErrorY = sum_ay / Sample_Cnt;
    ICM_Offset.AccErrorZ = sum_az / Sample_Cnt - 1.0f;

    ICM_Offset.GyroErrorX = sum_gx / Sample_Cnt;
    ICM_Offset.GyroErrorY = sum_gy / Sample_Cnt;
    ICM_Offset.GyroErrorZ = sum_gz / Sample_Cnt;

    ICM_Real.roll  = 0.0f;
    ICM_Real.pitch = 0.0f;
    ICM_Real.yaw   = 0.0f;
    ICM_Real.AccX  = 0.0f;
    ICM_Real.AccY  = 0.0f;
    ICM_Real.AccZ  = 1.0f;

    gyroAngleX = 0.0f;
    gyroAngleY = 0.0f;
}

// ==================== 去除零偏 ====================
static void ICM42688_Raw_Error_Update(void)
{
    ICM_Cali.AX = ICM_Raw_Data.AX - ICM_Offset.AccErrorX;
    ICM_Cali.AY = ICM_Raw_Data.AY - ICM_Offset.AccErrorY;
    ICM_Cali.AZ = ICM_Raw_Data.AZ - ICM_Offset.AccErrorZ;

    ICM_Cali.GX = ICM_Raw_Data.GX - ICM_Offset.GyroErrorX;
    ICM_Cali.GY = ICM_Raw_Data.GY - ICM_Offset.GyroErrorY;
    ICM_Cali.GZ = ICM_Raw_Data.GZ - ICM_Offset.GyroErrorZ;
}

// ==================== 互补滤波 ====================
static void ICM42688_Raw_Deal(int Deal_dt_ms)
{
    // 由加速度计算静态角度
    float accAngleX = atan2(ICM_Cali.AY, ICM_Cali.AZ)  * 180.0f / 3.14159265358f;
    float accAngleY = atan2(-ICM_Cali.AX, ICM_Cali.AZ) * 180.0f / 3.14159265358f;

    // 陀螺仪积分
    gyroAngleX += ICM_Cali.GX * Deal_dt_ms * 0.001f;
    gyroAngleY += ICM_Cali.GY * Deal_dt_ms * 0.001f;

    // Yaw：纯积分（无绝对参考，会漂移）
    ICM_Real.yaw += ICM_Cali.GZ * Deal_dt_ms * 0.001f;

    // 互补滤波
    gyroAngleX = ICM_COMP_GYRO * gyroAngleX + ICM_COMP_ACC * accAngleX;
    gyroAngleY = ICM_COMP_GYRO * gyroAngleY + ICM_COMP_ACC * accAngleY;

    // 输出
    ICM_Real.roll  = gyroAngleX;
    ICM_Real.pitch = gyroAngleY;
}

// ==================== 外部 Tick 入口 ====================
void ICM42688_Angle_Update_Tick(void)
{
    ICM42688_Update_Data();
    ICM42688_Raw_Error_Update();
    ICM42688_Raw_Deal(20);
}
