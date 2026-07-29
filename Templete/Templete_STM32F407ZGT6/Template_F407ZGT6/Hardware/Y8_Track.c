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
