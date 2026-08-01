// ========================== Con_Mode_6 ==========================
// 题6: 小车置于 A 点，钢球置于摆杆任意指定位置，按键启动后沿黑线顺时针行驶一圈
//       并通过A位置，整圈行驶总时间≤30s，要求行驶过程中钢球能稳定在摆杆上的
//       任意指定位置附近，误差绝对值≤1cm。
//
// 控制: 旋转编码器调节球目标偏移(mm), KEY2 归零, KEY1 启动比赛
// ★ s_ball_offset 单位 mm, 通过 Oran_Single_Pos 转换为 px 写入 Oran_Real_Offset
// ★ PID_Oran.goalPoint 恒为 0 (与 Con_Mode_3 相同约定)
//
// 寻迹加减速 + 多传感器阻尼 + 振荡检测 → Match/RaceCtrl 共享模块
#include "Con_Mode_6.h"

// 球目标偏移量 (mm), 编码器调节, ±50mm=±5cm
// Oran_Single_Pos=39 px/cm, Real_Offset = mm * Oran_Single_Pos / 10
#define OFFSET_STEP_MM   1.0f     // 编码器每 click 步长
#define OFFSET_MAX_MM   50.0f     // ±50mm = ±5cm
static float s_ball_offset_mm = 0.0f;

// mm → px 转换
static float mm_to_px(float mm) { return mm * (float)Oran_Single_Pos / 10.0f; }

void Con_Mode_6_Setup(void)
{
    // 球平衡初始化
    Oran_PID_Init();
    PID_Param_Reset(&PID_Oran);
    PID_Oran.goalPoint = 0.0f;
    Oran_Real_Offset   = mm_to_px(s_ball_offset_mm);
    Oran_FF_Enable     = 1.0f;
    Stepper_PWM_Angle_Gains_Set(&Stepper1, 4.0f, 0.0f, 0.829f, 50.0f, -50.0f);

    RaceCtrl_Setup();
}

void Con_Mode_6_Loop(void)
{
    // ──── 编码器调节偏移 (mm) ────
    {
        int16_t enc = Encoder_Get();
        if (enc != 0)
        {
            s_ball_offset_mm += (float)enc * OFFSET_STEP_MM;
            if      (s_ball_offset_mm >  OFFSET_MAX_MM) s_ball_offset_mm =  OFFSET_MAX_MM;
            else if (s_ball_offset_mm < -OFFSET_MAX_MM) s_ball_offset_mm = -OFFSET_MAX_MM;
            Oran_Real_Offset = mm_to_px(s_ball_offset_mm);
        }
    }

    // ──── KEY2 单击: 偏移归零 ────
    if (Key_Check(KEY_2, KEY_SINGLE))
    {
        s_ball_offset_mm = 0.0f;
        Oran_Real_Offset = 0.0f;
    }

    // ──── OLED 第0行: 当前偏移 (mm) ────
    OLED_Printf(0, 0, OLED_6X8, "Con_M6 off:%+.0fmm",
                s_ball_offset_mm);

    // ──── 串口 ABC: @Offset=<mm> ────
    if (Serial_GetNewPackageFlag_ABC(&Serial1))
        Serial_SetFloatData(&Serial1, "Offset", "Offset=%f", &s_ball_offset_mm);

    RaceCtrl_Loop();
}

void Con_Mode_6_Tick(void)
{
    Oran_Update();
    Oran_PID_Update();
    RaceCtrl_Tick();
}

void Con_Mode_6_Exit(void)
{
    RaceCtrl_Exit();
    Stepper_PWM_Stop(&Stepper1);
}
