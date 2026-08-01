#include "Y8_Track.h"
#include "Con_Motor.h"

Pid_Typedef Y8U_PID;
static float Track_Base_Speed = 0;          // 内部 static，外部通过 getter/setter 访问

void  Y8U_SetSpeed(float speed) { Track_Base_Speed = speed; }
float Y8U_GetSpeed(void)        { return Track_Base_Speed; }

// ============== 得到相对中心的偏移（×SCALE + EWMA）==============
float Y8U_GetOffset(void)
{
    static float ewma_offset = 0.0f;   // EWMA 平滑后的偏移

    float total_weight = 0.0f;
    float weighted_sum = 0.0f;

    for (int i = 0; i < Y8U_VALID_COUNT; i++) {
        if (Y8U_ADC[i] > Y8U_THRESHOLD) {
            float w = (float)(Y8U_ADC[i] - Y8U_THRESHOLD);
            total_weight += w;
            weighted_sum += (float)i * w;
        }
    }

    // 有线：重心 → ×SCALE → EWMA 平滑
    if (total_weight >= 1.0f) {
        float pos = weighted_sum / total_weight;
        float raw = (pos - Y8U_CENTER) * Y8U_SCALE;
        ewma_offset += (raw - ewma_offset) * Y8U_EWMA_ALPHA;
    }
    // 丢线：维持上次 EWMA 值，不做任何更新

    return ewma_offset;
}

// ============== PID 初始化 ==============
void Y8U_PID_Init(void)
{
    PID_Init(&Y8U_PID, 0.12f, 0.0f, 0.5f, 50.0f, -50.0f, 1000.0f);
}

// ============== PID 巡线更新（20ms Tick）==============
void Y8U_PID_Update(void)
{
    // 1. 更新真实值（已×SCALE + EWMA 平滑的偏移，目标=0）
    Y8U_PID.realPoint_Now = Y8U_GetOffset();

    // 2. 变增益：偏移越大 Kp 越大（直道柔和，弯道有力）
    {
        float abs_off = (Y8U_PID.realPoint_Now < 0.0f)
                      ? -Y8U_PID.realPoint_Now : Y8U_PID.realPoint_Now;
				// Kp变化
        if (abs_off < Y8U_GAIN_LO) 
				{
            Y8U_PID.Kp = Y8U_KP_BASE;
        } 
				else if (abs_off > Y8U_GAIN_HI) 
				{
            Y8U_PID.Kp = Y8U_KP_HIGH;
        } 
				else 
				{
            float t = (abs_off - Y8U_GAIN_LO) / (Y8U_GAIN_HI - Y8U_GAIN_LO);
            Y8U_PID.Kp = Y8U_KP_BASE + (Y8U_KP_HIGH - Y8U_KP_BASE) * t;
        }
    }

    // 3. PID 计算
    PID_Update(&Y8U_PID, Y8U_PID.realPoint_Now);

    // 3. 差速输出
    Motor_SetSpeed(&Motor_A, Track_Base_Speed - Y8U_PID.setPoint);
    Motor_SetSpeed(&Motor_B, Track_Base_Speed + Y8U_PID.setPoint);
}

// ============== 得到7路有效通道 ADC 之和 ==============
uint16_t Y8U_GetADC_Sum(void)
{
    uint16_t sum = 0;
    for (int i = 0; i < Y8U_VALID_COUNT; i++)
        sum += Y8U_ADC[i];
    return sum;
}

// ============== 速度斜坡（分频追赶，调用才生效）==============
void Y8U_RampTick(float goal, uint8_t cnt)
{
    static uint8_t div = 0;

    if (cnt == 0) return;

    if (++div < cnt) return;
    div = 0;

    if      (Track_Base_Speed < goal) Track_Base_Speed += 1.0f;
    else if (Track_Base_Speed > goal) Track_Base_Speed -= 1.0f;
}

// ============== 横线终点检测（滑动窗口 → 异常值触发，不污染基线）==============
static uint16_t fl_window[FINISH_SUM_WINDOW] = {0};
static uint8_t  fl_idx    = 0;
static uint8_t  fl_filled = 0;

// 每次起跑前调用，清空滑动窗口基线
void Y8U_FinishLine_Reset(void)
{
    for (int i = 0; i < FINISH_SUM_WINDOW; i++)
        fl_window[i] = 0;
    fl_idx    = 0;
    fl_filled = 0;
}

uint8_t Y8U_CheckFinishLine(void)
{
    uint16_t sum = Y8U_GetADC_Sum();

    // 滑动窗口均值
    int n = fl_filled ? FINISH_SUM_WINDOW : fl_idx;
    if (n == 0) {
        fl_window[fl_idx++] = sum;
        return 0;
    }

    float avg = 0;
    for (int i = 0; i < n; i++)
        avg += fl_window[i];
    avg /= n;

    // 异常值 → 检测到横线，不污染基线
    if (sum > avg * FINISH_SUM_RATIO)
        return 1;

    // 正常值 → 加入滑动窗口
    fl_window[fl_idx] = sum;
    fl_idx = (fl_idx + 1) % FINISH_SUM_WINDOW;
    if (!fl_filled && fl_idx == 0) fl_filled = 1;

    return 0;
}
