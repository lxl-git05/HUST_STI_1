// ========================== Con_Mode_5 ==========================
// 题5: 小车置于A点，钢球置于中心点O，按键启动后沿黑线顺时针行驶一圈并通过A位置，
//       整圈行驶总时间≤30s，行驶过程中钢球须稳定在摆杆中心点附近，误差绝对值≤1cm。
//
// 寻迹加减速 + 多传感器阻尼 + 振荡检测 → RaceCtrl 共享模块
// 球平衡 → 本文件负责 Oran_PID + Stepper
#include "Con_Mode_5.h"

void Con_Mode_5_Setup(void)
{
    // 球平衡初始化（球目标: 中心 O）
    Oran_PID_Init();
    PID_Param_Reset(&PID_Oran);
    PID_Oran.goalPoint = 0.0f;
    Oran_Real_Offset   = 0.0f;
    Oran_FF_Enable     = 1.0f;
    Stepper_PWM_Angle_Gains_Set(&Stepper1, 4.0f, 0.0f, 0.829f, 50.0f, -50.0f);

    // 寻迹比赛控制
    RaceCtrl_Setup();
}

void Con_Mode_5_Loop(void)
{
    OLED_Printf(0, 0, OLED_6X8, "Con_Mode_5  Y:%.0f", IMU_Yaw_Abs_Get());
    RaceCtrl_Loop();
}

void Con_Mode_5_Tick(void)
{
    // 球平衡（必须在 RaceCtrl_Tick 之前，因为阻尼需要 realPoint_Now）
    Oran_Update();
    Oran_PID_Update();

    // 寻迹加减速 + 多传感器阻尼 + 终点检测
    RaceCtrl_Tick();
}

void Con_Mode_5_Exit(void)
{
    RaceCtrl_Exit();
    Stepper_PWM_Stop(&Stepper1);
}
