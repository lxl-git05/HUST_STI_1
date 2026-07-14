#include "Mode_2.h"
#include "AllHeader.h"

// 模式A/B共用参数
#define MODE_MAX_SPEED  120.0f   // 最大速度(rpm)
#define MODE_ACC        60.0f    // 加速度(rpm/s)

// 子模式状态机
typedef enum {
    SUBMODE_NONE = 0,
    SUBMODE_A_FWD,    // 模式A: 正向 +180°
    SUBMODE_A_REV,    // 模式A: 反向 -180°
    SUBMODE_B_POS,    // 模式B: 去 +90°
    SUBMODE_B_NEG,    // 模式B: 去 -180°
} SubMode_Typedef;

static SubMode_Typedef sub_mode = SUBMODE_NONE;

static const char* Phase_Str(uint8_t phase)
{
    switch (phase) {
        case POS_PHASE_IDLE:   return "IDLE";
        case POS_PHASE_ACCEL:  return "ACCEL";
        case POS_PHASE_CRUISE: return "CRUISE";
        case POS_PHASE_DECEL:  return "DECEL";
        default:               return "????";
    }
}

static const char* SubMode_Str(SubMode_Typedef sm)
{
    switch (sm) {
        case SUBMODE_NONE:  return "NONE";
        case SUBMODE_A_FWD: return "A: +180";
        case SUBMODE_A_REV: return "A: -180";
        case SUBMODE_B_POS: return "B: +90";
        case SUBMODE_B_NEG: return "B: -180";
        default:            return "????";
    }
}

void Mode_2_Setup(void)
{
    OLED_Clear();
    sub_mode = SUBMODE_NONE;
}

void Mode_2_Loop(void)
{
    OLED_Printf(0, 0, OLED_6X8, "==== Pos Test ====");

    // KEY1 单击：启动/重启 模式A（相对往复 ±180°）
    if (Key_Check(KEY_1, KEY_SINGLE)) {
        sub_mode = SUBMODE_A_FWD;
        Stepper_PWM_Pos_Set_Rel(&Stepper1, 180.0f, MODE_MAX_SPEED, MODE_ACC);
    }

    // KEY2 单击：启动/重启 模式B（绝对往复 +90° ↔ -180°）
    if (Key_Check(KEY_2, KEY_SINGLE)) {
        sub_mode = SUBMODE_B_POS;
        Stepper_PWM_Pos_Set_Abs(&Stepper1, 90.0f, MODE_MAX_SPEED, MODE_ACC);
    }

    // KEY3 单击：停止循环
    if (Key_Check(KEY_3, KEY_SINGLE)) {
        sub_mode = SUBMODE_NONE;
        Stepper_PWM_Stop(&Stepper1);
        Stepper1.Pos_Phase = POS_PHASE_IDLE;
    }

    // --- 循环往复：当前运动到位后自动触发下一段 ---
    if (sub_mode != SUBMODE_NONE && Stepper1.Pos_Phase == POS_PHASE_IDLE) {
        switch (sub_mode) {
            case SUBMODE_A_FWD:
                sub_mode = SUBMODE_A_REV;
                Stepper_PWM_Pos_Set_Rel(&Stepper1, -180.0f, MODE_MAX_SPEED, MODE_ACC);
                break;
            case SUBMODE_A_REV:
                sub_mode = SUBMODE_A_FWD;
                Stepper_PWM_Pos_Set_Rel(&Stepper1, 180.0f, MODE_MAX_SPEED, MODE_ACC);
                break;
            case SUBMODE_B_POS:
                sub_mode = SUBMODE_B_NEG;
                Stepper_PWM_Pos_Set_Abs(&Stepper1, -180.0f, MODE_MAX_SPEED, MODE_ACC);
                break;
            case SUBMODE_B_NEG:
                sub_mode = SUBMODE_B_POS;
                Stepper_PWM_Pos_Set_Abs(&Stepper1, 90.0f, MODE_MAX_SPEED, MODE_ACC);
                break;
        }
    }

    // OLED 显示
    OLED_Printf(0, 16, OLED_6X8, "Mode: %s", SubMode_Str(sub_mode));
    OLED_Printf(0, 28, OLED_6X8, "Pos: %+07.2f", Stepper1.Pos_Now);
    OLED_Printf(0, 40, OLED_6X8, "Ph:%s Spd:%+05.0f", Phase_Str(Stepper1.Pos_Phase), Stepper1.Speed_Now);
    OLED_Printf(0, 52, OLED_6X8, "Step:%d/%d", (int)Stepper1.Pos_StepCnt, (int)Stepper1.Pos_TotalSteps);
}

void Mode_2_Tick(void)
{
    // 位置模式由 Timer_1ms_Callback 中的 Pos_Tick 驱动，此处无需额外处理
    Serial_printf(&Serial1, "%.2f,%.2f,%d\n", Stepper1.Pos_Now, Stepper1.Speed_Now, Stepper1.Pos_Phase * 10);
}

void Mode_2_Exit(void)
{
    // 退出时停止电机
    sub_mode = SUBMODE_NONE;
    Stepper_PWM_Stop(&Stepper1);
    Stepper1.Pos_Phase = POS_PHASE_IDLE;
    OLED_Clear();
}
