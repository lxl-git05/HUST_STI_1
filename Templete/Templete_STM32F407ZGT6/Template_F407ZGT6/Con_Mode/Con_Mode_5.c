// ========================== Con_Mode_5 ==========================
// 小车置于A点，钢球置于中心点O，按键启动后沿黑线顺时针行驶一圈并通过A位置，
// 整圈行驶总时间≤30s，行驶过程中钢球须稳定在摆杆中心点附近，误差绝对值≤1cm。
#include "Con_Mode_5.h"

#define RACE_SPEED    80.0f     // 加速目标(rpm)
#define LOW_SPEED     30.0f     // 低速巡航(rpm)
#define RAMP_CNT      8         // 加减速分频(全程一致)
#define PHASE1_YAW    160.0f    // P1→P2 偏航角(°)

enum { P1_ACCEL, P2_DECEL, P3_LOW, P_STOP };

static uint8_t  phase = P1_ACCEL;
static uint8_t  run   = 0;
static uint32_t t0    = 0;

void Con_Mode_5_Setup(void)
{
    Oran_PID_Init();
    PID_Param_Reset(&PID_Oran);
    PID_Oran.goalPoint = 0.0f;
    Oran_Real_Offset = 0.0f;
    Oran_FF_Enable   = 1.0f;
    Stepper_PWM_Angle_Gains_Set(&Stepper1, 4.0f, 0.0f, 0.829f, 50.0f, -50.0f);
    Y8U_SetSpeed(0);
    phase = P1_ACCEL;
    run   = 0;
}

void Con_Mode_5_Loop(void)
{
    OLED_Printf(0, 0, OLED_6X8, "Con_Mode_5  Y:%.0f", IMU_Yaw_Abs_Get());

    if (!run)
    {
        OLED_Printf(0, 20, OLED_8X16, "KEY1:Go");
        if (Key_Check(KEY_1, KEY_SINGLE))
        {
            IMU_Yaw_Abs_Reset();
            t0    = HAL_GetTick();
            phase = P1_ACCEL;
            run   = 1;
        }
    }
    else if (phase == P_STOP)
    {
        OLED_Printf(0, 20, OLED_8X16, "Done %.1fs", (HAL_GetTick() - t0) / 1000.0f);
    }
    else
    {
        const char *tag = (phase == P1_ACCEL) ? "P1+" : (phase == P2_DECEL) ? "P2-" : "P3~";
        OLED_Printf(0, 10, OLED_6X8, "Spd:%.0f %s", Y8U_GetSpeed(), tag);
        OLED_Printf(0, 20, OLED_8X16, "%.1fs", (HAL_GetTick() - t0) / 1000.0f);
    }
}

void Con_Mode_5_Tick(void)
{
    Oran_Update();
    Oran_PID_Update();

    if (!run) return;

    Y8U_PID_Update();

    // 绿灯: P1=ON  P2=OFF  P3=ON
    RGB_Set_Color(0, (phase != P2_DECEL && phase != P_STOP) ? 1 : 0, 0);

    // 阶段切换
    switch (phase)
    {
    case P1_ACCEL:
        if (IMU_Yaw_Abs_Get() > PHASE1_YAW)
            phase = P2_DECEL;
        break;
    case P2_DECEL:
        if (Y8U_GetSpeed() <= LOW_SPEED + 2.0f)
            phase = P3_LOW;
        break;
    case P3_LOW:
        if (Y8U_CheckFinishLine())
        {
            phase = P_STOP;
            Y8U_SetSpeed(0);
            Motor_SetSpeed(&Motor_A, 0);
            Motor_SetSpeed(&Motor_B, 0);
            RGB_Set_Color(0, 0, 0);
        }
        break;
    }

    // 速度 ramp
    if (phase != P_STOP)
    {
        float target;
        if      (phase == P1_ACCEL) target = RACE_SPEED;
        else if (phase == P2_DECEL) target = LOW_SPEED;
        else                        target = LOW_SPEED;
        Y8U_RampTick(target, RAMP_CNT);
    }
}

void Con_Mode_5_Exit(void)
{
    run   = 0;
    phase = P1_ACCEL;
    Y8U_SetSpeed(0);
    RGB_Set_Color(0, 0, 0);
    Stepper_PWM_Stop(&Stepper1);
}
