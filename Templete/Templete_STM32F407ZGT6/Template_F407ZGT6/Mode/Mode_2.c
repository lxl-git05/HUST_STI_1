#include "AllHeader.h"

// ==================== IMU Mahony 统一库 实验例程 ====================
// 传感器选择在 Function/IMU.h 的 #define IMU_USE_MPU6050 中切换
//   注释掉 → ICM42688    取消注释 → MPU6050
//
// OLED:  Roll / Pitch / Yaw / Abs + 转到位指示
// 串口:  roll,pitch,yaw,yaw_abs CSV (Serial1, 115200bps)
//
// 测试流程:
//   1. 上电进入 Mode_2 → Init(1) 自动标定零偏（保持静止！）
//   2. OLED 显示 4 行角度数据
//   3. 旋转设备，观察 roll/pitch/yaw 实时变化
//   4. 转到 90° 附近 → OLED 第 5 行显示 "OK"，LED 快闪
//   5. 串口输出 CSV 可用上位机实时绘图

static void Mode_2_Show(void)
{
    OLED_Printf(0, 0,  OLED_6X8, "R:%.1f P:%.1f",
                IMU_Mahony_Real.roll, IMU_Mahony_Real.pitch);
    OLED_Printf(0, 12, OLED_6X8, "Y:%.1f", IMU_Mahony_Real.yaw);
    OLED_Printf(0, 24, OLED_6X8, "Abs:%.1f", IMU_Yaw_Abs_Get());

    // 转到 90° ± 3° → 提示到位
    if (IMU_Turn_Yaw_Is_Ok(90.0f))
    {
        OLED_Printf(0, 36, OLED_6X8, "Turn OK!");
        Flash_Mode_Set(Flash_Mode_Fast);
    }
    else
    {
        OLED_Printf(0, 36, OLED_6X8, "         ");
        Flash_Mode_Set(Flash_Mode_OFF);
    }
}

void Mode_2_Setup(void)
{
    IMU_Mahony_Init(1);         // 1=自动标定(需静止) / 0=跳过标定
    IMU_Yaw_Abs_Reset();        // yaw_abs 归零
}

void Mode_2_Loop(void)
{
    Mode_2_Show();
}

void Mode_2_Tick(void)
{
    IMU_Mahony_Update_Tick();   // 读 IMU → Mahony 解算

    Serial_printf(&Serial1, "%.2f,%.2f,%.2f,%.2f\r\n",
                  IMU_Mahony_Real.roll, IMU_Mahony_Real.pitch,
                  IMU_Mahony_Real.yaw, IMU_Yaw_Abs_Get());
}

void Mode_2_Exit(void)
{
    Flash_Mode_Set(Flash_Mode_OFF);
}
