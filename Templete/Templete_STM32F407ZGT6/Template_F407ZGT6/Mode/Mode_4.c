// ========================== Mode_4 ==========================
// 寻迹+球平衡联调：S 曲线速度规划版
//
// ★ 核心原则：
//   1. 加速度全程缓慢 — S 曲线过渡时间长，速度变化幅度小
//   2. 不预判停车 — 越过终点横线后才开始减速，不停车
//   3. 弯道缓 — 弯前提前温和降速，弯中低速，出弯温和加速
//   4. 直道适度快 — 巡航速度不过高，保持球稳定
//
// 状态机：IDLE → RUN → FINISH → STOP
//          KEY1   横线触发   speed≈0
#include "Mode_4.h"
#include "AllHeader.h"

// ==================== 状态机 ====================
enum { S_IDLE, S_RUN, S_FINISH, S_STOP };

// ==================== S 曲线速度规划器 ====================
// smoothstep: f(t)=3t²-2t³, f'(0)=f'(1)=0 → 加速度连续无突变

typedef struct {
    float v0, vt;      // 起始速度, 目标速度 (rpm)
    int   dur;         // 过渡 tick 数 (20ms/tick), 0=瞬时
    int   tick;        // 当前 tick
} SCurve_t;

static void scurve_start(SCurve_t *sc, float from, float to, int dur)
{
    sc->v0   = from;
    sc->vt   = to;
    sc->dur  = (dur > 0) ? dur : 0;
    sc->tick = 0;
}

static float scurve_peek(SCurve_t *sc)
{
    if (sc->dur <= 0 || sc->tick >= sc->dur) return sc->vt;
    float t = (float)sc->tick / (float)sc->dur;
    float s = t * t * (3.0f - 2.0f * t);
    return sc->v0 + (sc->vt - sc->v0) * s;
}

static float scurve_tick(SCurve_t *sc)
{
    if (sc->dur <= 0) return sc->vt;
    sc->tick++;
    if (sc->tick >= sc->dur) return sc->vt;
    float t = (float)sc->tick / (float)sc->dur;
    float s = t * t * (3.0f - 2.0f * t);
    return sc->v0 + (sc->vt - sc->v0) * s;
}

// ==================== 偏航角分段速度表 ====================
// { 偏航启动角(°), 目标速度(rpm), S曲线过渡tick }
// ★ 偏航角必须递增, 从 0.0 开始
// ★ ramp=0 表示该段匀速, 不需要过渡
// ★ 不设预减速段, 终点由 Y8U_CheckFinishLine 触发
// ★ 比赛前根据实际赛道调节

typedef struct {
    float yaw;     // 进入偏航角 (°)
    float speed;   // 目标速度 (rpm)
    int   ramp;    // S 曲线过渡 tick 数 (20ms/tick)
} SpeedSeg_t;

#define SEG_COUNT 6
static SpeedSeg_t seg[SEG_COUNT] = {
    // { yaw,   speed, ramp }   说明
    {   0.0f,  65.0f, 150 },  // 0°:   极缓起步 0→65, 150tick=3.0s
    {  50.0f,  65.0f,   0 },  // 50°:  直道巡航 65
    { 100.0f,  48.0f, 120 },  // 100°: 入弯缓降 65→48, 120tick=2.4s
    { 150.0f,  48.0f,   0 },  // 150°: 弯中保持 48
    { 210.0f,  65.0f, 120 },  // 210°: 出弯缓升 48→65, 120tick=2.4s
    { 280.0f,  65.0f,   0 },  // 280°: 直道巡航至终点 (不预减速!)
};
// ★ 终点停车: Y8U_CheckFinishLine 触发后, S 曲线 →0, 250tick=5.0s 极缓
// ★ 过线后冻结寻迹 PID 5 帧 (100ms), 防止白线 ADC 异常干扰方向
// ★ 偏航角 > 320° 才允许触发终点检测, 防止前半圈误触发
#define FINISH_MIN_YAW  320.0f

// ==================== 运行时状态 ====================
static uint8_t     state       = S_IDLE;
static uint32_t    t0          = 0;
static float       race_time   = 0.0f;
static SCurve_t    sc;
static int         seg_idx     = 0;
static float       prev_off    = 0.0f;   // 上一帧 Y8U 偏移
static uint8_t     finish_hold = 0;      // 过线后冻结寻迹帧数, 防止白线干扰

// ==================== 查找当前偏航角所在速度段 ====================
static int profile_find_seg(float yaw)
{
    int idx = seg_idx;
    for (int i = seg_idx + 1; i < SEG_COUNT; i++) {
        if (yaw >= seg[i].yaw) idx = i;
        else break;
    }
    return idx;
}

// ==================== 宽松积分分离（行驶专用）====================
static void Mode4_IntSep(void)
{
    float r = fabs(PID_Oran.realPoint_Now);
    float s = fabs((float)Oran_Speed);
    if (r > 100.0f && s > 30.0f)
        PID_Oran.SumError = 0.0f;
}

// ==================== Setup ====================
void Mode_4_Setup(void)
{
    Con_Mode_3_Setup();
    PID_Oran.PID_Func = Mode4_IntSep;
    PID_Oran.Kp       = 0.20f;
    PID_Oran.ioutMax  = 2000.0f;
    Oran_FF_Enable    = 1.0f;
    Oran_Damping_K    = 0.10f;

    Y8U_SetSpeed(0);
    state    = S_IDLE;
    seg_idx  = 0;
    scurve_start(&sc, 0, seg[0].speed, seg[0].ramp);
}

// ==================== Loop ====================
void Mode_4_Loop(void)
{
    OLED_Printf(0, 0, OLED_6X8, "M4 S-Crv Y:%.0f", IMU_Yaw_Abs_Get());

    // 串口 ABC 在线调参
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
        OLED_Printf(0, 20, OLED_6X8, "Speeds:%.0f/%.0f/%.0f",
                    seg[1].speed, seg[3].speed, seg[5].speed);
        if (Key_Check(KEY_1, KEY_SINGLE))
        {
            state    = S_RUN;
            t0       = HAL_GetTick();
            seg_idx  = 0;
            IMU_Yaw_Abs_Reset();
            Y8U_FinishLine_Reset();   // 清空终点检测滑动窗口, 重新建立基线
            scurve_start(&sc, 0, seg[0].speed, seg[0].ramp);
            Y8U_SetSpeed(0);
        }
        break;

    case S_RUN:
        OLED_Printf(0, 10, OLED_6X8, "S%d(%.0f) D:%.2f",
                    seg_idx, seg[seg_idx].speed, Oran_Damping_K);
        OLED_Printf(0, 20, OLED_8X16, "%.1fs %.0frpm",
                    (HAL_GetTick() - t0) / 1000.0f, Y8U_GetSpeed());
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

// ==================== Tick (20ms) ====================
void Mode_4_Tick(void)
{
    // 球平衡（始终运行，IDLE 也能观察球是否稳住）
    Con_Mode_3_Tick();
    Motor_Pos_Update(&Motor_A);
    Motor_Pos_Update(&Motor_B);

    if (state == S_IDLE || state == S_STOP) return;

    float yaw = IMU_Yaw_Abs_Get();

    // === S_RUN: 按速度段表行驶 ===
    if (state == S_RUN)
    {
        // 1. 查找当前速度段
        int new_idx = profile_find_seg(yaw);
        if (new_idx != seg_idx)
        {
            float cur = scurve_peek(&sc);
            scurve_start(&sc, cur, seg[new_idx].speed, seg[new_idx].ramp);
            seg_idx = new_idx;
        }

        // 2. S 曲线推进
        float cur_spd = scurve_tick(&sc);
        Y8U_SetSpeed(cur_spd);

        // 3. 寻迹 PID（过线冻结期内跳过，防止白线干扰方向）
        if (finish_hold == 0)
            Y8U_PID_Update();
        else
            finish_hold--;

        // 4. 弯道动态阻尼
        {
            float off = fabs(Y8U_GetOffset());
            if      (off > 120.0f) Oran_Damping_K = 0.26f;
            else if (off > 60.0f)  Oran_Damping_K = 0.18f;
            else                   Oran_Damping_K = 0.08f;

            if (prev_off > 60.0f && off <= 60.0f)
                PID_Oran.SumError = 0.0f;
            prev_off = off;
        }

        // 5. 终点检测 → 触发缓停车（需偏航角门槛 + 滑动窗口双重确认）
        if (yaw > FINISH_MIN_YAW && Y8U_CheckFinishLine())
        {
            state       = S_FINISH;
            race_time   = (HAL_GetTick() - t0) / 1000.0f;  // 过线即停表
            finish_hold = 5;   // 冻结寻迹 5 帧 (100ms), 等白线完全过去
            float cur = scurve_peek(&sc);
            scurve_start(&sc, cur, 0.0f, 250);  // 250tick=5.0s 极缓慢停车
        }
    }
    // === S_FINISH: 缓慢减速到 0 ===
    else if (state == S_FINISH)
    {
        float cur_spd = scurve_tick(&sc);
        Y8U_SetSpeed(cur_spd);

        // 停车期间: 冻结期内跳过 PID, 之后恢复寻迹纠偏
        if (finish_hold == 0)
            Y8U_PID_Update();
        else
            finish_hold--;

        // 弯道阻尼（防止停车段有弯）
        {
            float off = fabs(Y8U_GetOffset());
            if      (off > 120.0f) Oran_Damping_K = 0.26f;
            else if (off > 60.0f)  Oran_Damping_K = 0.18f;
            else                   Oran_Damping_K = 0.08f;
        }

        // 速度降到 ~0 后正式停车
        if (cur_spd < 1.5f)
        {
            state      = S_STOP;
            Y8U_SetSpeed(0);
            Motor_SetSpeed(&Motor_A, 0);
            Motor_SetSpeed(&Motor_B, 0);
        }
    }

    // === 串口 CSV（调试用, 需要时取消注释）===
//    Serial_printf(&Serial1, "%.2f,%.2f,%.2f,%.2f,%.2f\r\n",
//        yaw, Y8U_GetSpeed(), Y8U_GetOffset(),
//        PID_Oran.realPoint_Now, PID_Oran.setPoint);
}

// ==================== Exit ====================
void Mode_4_Exit(void)
{
    state = S_IDLE;
    Y8U_SetSpeed(0);
    Motor_SetSpeed(&Motor_A, 0);
    Motor_SetSpeed(&Motor_B, 0);
    Stepper_PWM_Stop(&Stepper1);
}
