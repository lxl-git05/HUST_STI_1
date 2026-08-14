// ========================== Con_Mode_4 ==========================
// 小车置于A点，钢球置于中心点O，按键启动后沿黑线顺时针行驶并通过B位置，
// AB间行驶时间≤8s，行驶过程中钢球须稳定在摆杆中心点附近，误差绝对值≤1cm。
#include "Con_Mode_4.h"

enum { S_IDLE, S_ACCEL, S_CRUISE, S_DECEL, S_STOP };

#define RACE_SPD_START  70.0f    // 0~1.5s 慢起步(rpm), 防超调
#define RACE_SPD_MID    95.0f    // 1.5~4s 中速巡航(rpm)
#define RACE_SPD_HI    105.0f    // 4s~ 高速冲刺(rpm)
#define T1_MS           1500     // 慢→中 时刻(ms)
#define T2_MS           4000     // 中→高 时刻(ms)
#define RAMP_CNT          2      // 加减速分频(越大越平滑)
#define STOP_YAW        10.0f    // 过10°停表计时
#define BRAKE_YAW       20.0f    // 过20°开始慢刹车

static uint8_t  state     = S_IDLE;
static uint32_t t0        = 0;
static float    stop_time = 0.0f;

void Con_Mode_4_Setup(void)
{
    Oran_PID_Init();
    PID_Param_Reset(&PID_Oran);
    PID_Oran.goalPoint = 0.0f;
    Oran_Real_Offset = 0.0f;
    Oran_FF_Enable   = 1.0f;        // 行驶中开前馈
    Stepper_PWM_Angle_Gains_Set(&Stepper1, 4.0f, 0.0f, 0.829f, 50.0f, -50.0f);
    Y8U_SetSpeed(0);
    state = S_IDLE;
}

// 寻迹
void Con_Mode_4_Loop(void)
{
    OLED_Printf(0, 0, OLED_6X8, "Con_Mode_4  Y:%.0f", IMU_Yaw_Abs_Get());

    if (state == S_IDLE)
    {
        OLED_Printf(0, 20, OLED_8X16, "KEY1:Go");
        if (Key_Check(KEY_1, KEY_SINGLE))
        {
            IMU_Yaw_Abs_Reset();
            t0    = HAL_GetTick();
            state = S_ACCEL;
        }
    }
    else if (state == S_STOP)
    {
        OLED_Printf(0, 20, OLED_8X16, "Done %.1fs", stop_time);
    }
    else
    {
        OLED_Printf(0, 10, OLED_6X8, "Spd:%.0f", Y8U_GetSpeed());
        OLED_Printf(0, 20, OLED_8X16, "%.1fs", (HAL_GetTick() - t0) / 1000.0f);
    }

}

void Con_Mode_4_Tick(void)
{
    // 球平衡
    Oran_Update();
    Oran_PID_Update();

    if (state == S_IDLE || state == S_STOP)
        return;

    // 寻迹
    Y8U_PID_Update();

    // 状态切换
    switch (state)
    {
    case S_ACCEL:
        state = S_CRUISE;  // 直接进入巡航, 速度由下方三段规划控制
        break;
    case S_CRUISE:
        {
            float yaw = IMU_Yaw_Abs_Get();
            if (yaw > STOP_YAW && stop_time == 0.0f)
                stop_time = (HAL_GetTick() - t0) / 1000.0f;  // 过10°停表
            if (yaw > BRAKE_YAW)
                state = S_DECEL;  // 过20°开始慢刹车
        }
        break;
    case S_DECEL:
        if (Y8U_GetSpeed() < 2.0f)
        {
            state = S_STOP;
            Y8U_SetSpeed(0);
            Motor_SetSpeed(&Motor_A, 0);
            Motor_SetSpeed(&Motor_B, 0);
        }
        break;
    }

    // 三段速度规划: 慢起步→中巡航→快冲刺
    {
        float target;
        uint32_t t = HAL_GetTick() - t0;
        if (state == S_DECEL)
            target = 0.0f;
        else if (t < T1_MS)
            target = RACE_SPD_START;
        else if (t < T2_MS)
            target = RACE_SPD_MID;
        else
            target = RACE_SPD_HI;
        Y8U_RampTick(target, RAMP_CNT);
    }
}

void Con_Mode_4_Exit(void)
{
    state = S_IDLE;
    Y8U_SetSpeed(0);
    Stepper_PWM_Stop(&Stepper1);
}
