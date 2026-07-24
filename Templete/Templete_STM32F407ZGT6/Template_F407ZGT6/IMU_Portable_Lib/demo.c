// ==================== IMU Portable Lib — 使用示例 ====================
// 本文件演示 ICM42688 和 MPU6050 的 Mahony AHRS 使用方法
// 平台: STM32F407ZGT6 | 框架: Mode_2 (20ms Tick + Loop 显示)
//
// ★ 两个 IMU 均挂 I2C1 地址 0x68，物理上只能二选一
// ★ 注释/取消注释下面的宏切换 IMU

//#define USE_MPU6050    // 取消注释 → MPU6050，注释掉 → ICM42688

#ifdef USE_MPU6050

// ==================== MPU6050 Mahony AHRS 示例 ====================
// 测试流程:
//   1. 上电切到 Mode_2 → Init(1) 自动标定（保持静止！）
//   2. 水平旋转 90° → Yaw 约 90°
//   3. 倾斜绕 Z 轴旋转 → Yaw 准确跟踪
//   4. 随意抖动后静置 → Roll/Pitch <0.5s 收敛

void Mode_2_Setup(void)
{
    // Init(1): 自动采样 1000 次标定零偏，设备必须静止！
    // Init(0): 跳过标定，使用 MPU_Mahony_GyroBiasX/Y/Z 当前值
    MPU6050_Mahony_Init(1);
}

void Mode_2_Loop(void)
{
    OLED_Printf(0, 0,  OLED_6X8, "MPU R:%.1f", MPU_Mahony_Real.roll);
    OLED_Printf(0, 12, OLED_6X8, "P:%.1f", MPU_Mahony_Real.pitch);
    OLED_Printf(0, 24, OLED_6X8, "Y:%.1f", MPU_Mahony_Real.yaw);
    OLED_Printf(0, 36, OLED_6X8, "Abs:%.1f", MPU_Yaw_Abs_Get());
}

void Mode_2_Tick(void)
{
    MPU6050_Mahony_Update_Tick();
    Serial_printf(&Serial1, "%.2f,%.2f,%.2f,%.2f\r\n",
                  MPU_Mahony_Real.roll, MPU_Mahony_Real.pitch,
                  MPU_Mahony_Real.yaw, MPU_Yaw_Abs_Get());
}

#else

// ==================== ICM42688 Mahony AHRS 示例 ====================
// 测试流程同上

void Mode_2_Setup(void)
{
    ICM42688_Mahony_Init(0);
}

void Mode_2_Loop(void)
{
    OLED_Printf(0, 0,  OLED_6X8, "ICM R:%.1f", ICM_Mahony_Real.roll);
    OLED_Printf(0, 12, OLED_6X8, "P:%.1f", ICM_Mahony_Real.pitch);
    OLED_Printf(0, 24, OLED_6X8, "Y:%.1f", ICM_Mahony_Real.yaw);
    OLED_Printf(0, 36, OLED_6X8, "Abs:%.1f", ICM_Yaw_Abs_Get());
}

void Mode_2_Tick(void)
{
    ICM42688_Mahony_Update_Tick();
    Serial_printf(&Serial1, "%.2f,%.2f,%.2f,%.2f\r\n",
                  ICM_Mahony_Real.roll, ICM_Mahony_Real.pitch,
                  ICM_Mahony_Real.yaw, ICM_Yaw_Abs_Get());
}

#endif

void Mode_2_Exit(void)
{
    // 退出时可选: 重标定 / 保存零偏到 EEPROM
}
