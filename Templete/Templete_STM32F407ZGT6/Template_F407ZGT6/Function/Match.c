// ========================== Match ==========================
// 比赛专用逻辑：寻迹加减速控制 (RaceCtrl)
// 从 Mode_4 移植，供 Con_Mode_5/6 共用
// 包含: S曲线速度规划 + 多传感器融合阻尼 + 球振荡检测 + 终点检测
// 调用者负责: Oran_PID_Init / Oran_Update / Oran_PID_Update / Stepper stop

#include "Match.h"
#include "AllHeader.h"

// ==================== 状态机 ====================
enum { S_IDLE, S_RUN, S_FINISH, S_STOP };

// ==================== S 曲线速度规划器 ====================
typedef struct {
    float v0, vt;
    int   dur, tick;
} SCurve_t;

static void scurve_start(SCurve_t *sc, float from, float to, int dur)
{
    sc->v0 = from; sc->vt = to; sc->dur = (dur > 0) ? dur : 0; sc->tick = 0;
}
static float scurve_peek(SCurve_t *sc)
{
    if (sc->dur <= 0 || sc->tick >= sc->dur) return sc->vt;
    float t = (float)sc->tick / sc->dur;
    float s = t * t * (3.0f - 2.0f * t);
    return sc->v0 + (sc->vt - sc->v0) * s;
}
static float scurve_tick(SCurve_t *sc)
{
    if (sc->dur <= 0) return sc->vt;
    sc->tick++;
    if (sc->tick >= sc->dur) return sc->vt;
    float t = (float)sc->tick / sc->dur;
    float s = t * t * (3.0f - 2.0f * t);
    return sc->v0 + (sc->vt - sc->v0) * s;
}

// ==================== 偏航角分段速度表 ====================
typedef struct {
    float yaw;
    float speed;
    int   ramp;
} SpeedSeg_t;

#define SEG_COUNT 6
static SpeedSeg_t seg[SEG_COUNT] = {
    {   0.0f,  65.0f, 150 },
    {  50.0f,  65.0f,   0 },
    { 100.0f,  48.0f, 120 },
    { 150.0f,  48.0f,   0 },
    { 210.0f,  65.0f, 120 },
    { 280.0f,  65.0f,   0 },
};
#define FINISH_MIN_YAW  320.0f

// ==================== 运行时状态 ====================
static uint8_t     state       = S_IDLE;
static uint32_t    t0          = 0;
static float       race_time   = 0.0f;
static SCurve_t    sc;
static int         seg_idx     = 0;
static float       prev_off    = 0.0f;
static uint8_t     finish_hold = 0;

// 球振荡检测
static float  ball_hist[6];
static uint8_t ball_idx = 0;

// OLED 调试值
static float dbg_severity = 0.0f;
static int   dbg_osc      = 0;

// ==================== 内部函数 ====================
static int profile_find_seg(float yaw)
{
    int idx = seg_idx;
    for (int i = seg_idx + 1; i < SEG_COUNT; i++) {
        if (yaw >= seg[i].yaw) idx = i;
        else break;
    }
    return idx;
}

static void RaceCtrl_IntSep(void)
{
    float r = fabs(PID_Oran.realPoint_Now);
    float s = fabs((float)Oran_Speed);
    if (r > 100.0f && s > 30.0f)
        PID_Oran.SumError = 0.0f;
}

// ==================== 多传感器融合阻尼 + 振荡检测 ====================
static void RaceCtrl_Damping_Update(void)
{
    // 1. 多传感器弯道严重程度 (连续值 0~1)
    float s_offset   = fabs(Y8U_GetOffset()) / 200.0f;
    float s_yawrate  = IMU_Yaw_Gyro_Get() / 150.0f;
    float s_lataccel = fabs(IMU_Get_Ay()) * 4.0f;
    if (s_offset   > 1.0f) s_offset   = 1.0f;
    if (s_yawrate  > 1.0f) s_yawrate  = 1.0f;
    if (s_lataccel > 1.0f) s_lataccel = 1.0f;

    float severity = s_offset * 0.20f + s_yawrate * 0.50f + s_lataccel * 0.30f;
    if (severity > 1.0f) severity = 1.0f;
    dbg_severity = severity;

    // 2. 速度段基础阻尼
    float base;
    switch (seg_idx) {
        case 0:  base = 0.10f; break;
        case 1:
        case 5:  base = 0.06f; break;
        case 2:
        case 4:  base = 0.12f; break;
        case 3:  base = 0.16f; break;
        default: base = 0.08f; break;
    }
    Oran_Damping_K = base + severity * (0.30f - base);

    // 3. 球速非线性修正
    {
        float abs_spd = fabs((float)Oran_Speed);
        if (abs_spd < 10.0f) {
            float mult = 3.0f - abs_spd * 0.2f;
            Oran_Damping_K *= mult;
        }
    }

    // 4. 出弯清积分
    {
        float off = fabs(Y8U_GetOffset());
        if (prev_off > 60.0f && off <= 60.0f)
            PID_Oran.SumError = 0.0f;
        prev_off = off;
    }

    // 5. 球振荡检测 (6帧内过零 >=3 次)
    ball_hist[ball_idx] = PID_Oran.realPoint_Now;
    ball_idx = (ball_idx + 1) % 6;

    int zero_cross = 0;
    for (int i = 1; i < 6; i++) {
        int p = (ball_idx - i - 1 + 6) % 6;
        int c = (ball_idx - i + 6) % 6;
        if (ball_hist[p] * ball_hist[c] < 0.0f)
            zero_cross++;
    }
    dbg_osc = zero_cross;

    if (zero_cross >= 3) {
        PID_Oran.Kp      = 0.12f;
        PID_Oran.SumError = 0.0f;
        Oran_Damping_K  += 0.06f;
    } else {
        PID_Oran.Kp      = 0.20f;
    }
}

// ==================== 公开 API ====================

void RaceCtrl_Setup(void)
{
    PID_Oran.PID_Func = RaceCtrl_IntSep;
    PID_Oran.Kp       = 0.20f;
    PID_Oran.ioutMax  = 2000.0f;
    Oran_FF_Enable    = 1.0f;
    Oran_Damping_K    = 0.10f;

    Y8U_SetSpeed(0);
    state    = S_IDLE;
    seg_idx  = 0;
    scurve_start(&sc, 0, seg[0].speed, seg[0].ramp);
}

void RaceCtrl_Start(void)
{
    state    = S_RUN;
    t0       = HAL_GetTick();
    seg_idx  = 0;
    IMU_Yaw_Abs_Reset();
    Y8U_FinishLine_Reset();
    scurve_start(&sc, 0, seg[0].speed, seg[0].ramp);
    Y8U_SetSpeed(0);
		Serial_printf(&Serial2 , "@rec:666$#") ;
}

void RaceCtrl_Loop(void)
{
    if (Serial_GetNewPackageFlag_ABC(&Serial1))
    {
        Serial_SetFloatData(&Serial1, "Kp",   "Kp=%f",   &Y8U_PID.Kp);
        Serial_SetFloatData(&Serial1, "Ki",   "Ki=%f",   &Y8U_PID.Ki);
        Serial_SetFloatData(&Serial1, "Kd",   "Kd=%f",   &Y8U_PID.Kd);
        Serial_SetFloatData(&Serial1, "Goal", "Goal=%f", &Oran_Real_Offset);
        Serial_SetFloatData(&Serial1, "S0", "%f", &seg[0].speed);
        Serial_SetFloatData(&Serial1, "S1", "%f", &seg[1].speed);
        Serial_SetFloatData(&Serial1, "S2", "%f", &seg[2].speed);
        Serial_SetFloatData(&Serial1, "S3", "%f", &seg[3].speed);
        Serial_SetFloatData(&Serial1, "S4", "%f", &seg[4].speed);
        Serial_SetFloatData(&Serial1, "S5", "%f", &seg[5].speed);
    }

    switch (state)
    {
    case S_IDLE:
        OLED_Printf(0, 10, OLED_6X8, "KEY1:Go");
        OLED_Printf(0, 20, OLED_6X8, "Spds:%.0f/%.0f/%.0f",
                    seg[1].speed, seg[3].speed, seg[5].speed);
        if (Key_Check(KEY_1, KEY_SINGLE))
            RaceCtrl_Start();
        break;

    case S_RUN:
        OLED_Printf(0, 10, OLED_6X8, "S%d Sv:%.2f D:%.2f",
                    seg_idx, dbg_severity, Oran_Damping_K);
        OLED_Printf(0, 20, OLED_8X16, "%dosc %.0frpm",
                    dbg_osc, Y8U_GetSpeed());
        break;

    case S_FINISH:
        OLED_Printf(0, 10, OLED_6X8, "Decel... %.0frpm", Y8U_GetSpeed());
        OLED_Printf(0, 20, OLED_8X16, "%.1fs", (HAL_GetTick() - t0) / 1000.0f);
        break;

    case S_STOP:
        OLED_Printf(0, 10, OLED_6X8, "Done!");
        OLED_Printf(0, 20, OLED_8X16, "%.1fs", race_time);
        break;
    }
}

void RaceCtrl_Tick(void)
{
    Motor_Pos_Update(&Motor_A);
    Motor_Pos_Update(&Motor_B);

    if (state == S_IDLE || state == S_STOP) return;

    float yaw = IMU_Yaw_Abs_Get();

    if (state == S_RUN)
    {
        int new_idx = profile_find_seg(yaw);
        if (new_idx != seg_idx)
        {
            float cur = scurve_peek(&sc);
            scurve_start(&sc, cur, seg[new_idx].speed, seg[new_idx].ramp);
            seg_idx = new_idx;
        }

        float cur_spd = scurve_tick(&sc);
        Y8U_SetSpeed(cur_spd);

        if (finish_hold == 0)
            Y8U_PID_Update();
        else
            finish_hold--;

        RaceCtrl_Damping_Update();

        if (yaw > FINISH_MIN_YAW && Y8U_CheckFinishLine())
        {
            state       = S_FINISH;
            finish_hold = 5;
            float cur = scurve_peek(&sc);
            scurve_start(&sc, cur, 0.0f, 250);
        }
    }
    else if (state == S_FINISH)
    {
        float cur_spd = scurve_tick(&sc);
        Y8U_SetSpeed(cur_spd);

        if (finish_hold == 0)
            Y8U_PID_Update();
        else
            finish_hold--;

        RaceCtrl_Damping_Update();

        if (cur_spd < 1.5f)
        {
            state      = S_STOP;
            Serial_printf(&Serial2 , "@stop:666$#") ;
            race_time  = (HAL_GetTick() - t0) / 1000.0f;
            Y8U_SetSpeed(0);
            Motor_SetSpeed(&Motor_A, 0);
            Motor_SetSpeed(&Motor_B, 0);
        }
    }
}

void RaceCtrl_Exit(void)
{
    state = S_IDLE;
    Y8U_SetSpeed(0);
    Motor_SetSpeed(&Motor_A, 0);
    Motor_SetSpeed(&Motor_B, 0);
}

uint8_t RaceCtrl_IsRunning(void) { return (state == S_RUN || state == S_FINISH); }
uint8_t RaceCtrl_IsStopped(void) { return (state == S_STOP); }
float   RaceCtrl_GetTime(void)   { return (state == S_STOP) ? race_time
                                    : (HAL_GetTick() - t0) / 1000.0f; }
