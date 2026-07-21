#include "Stepper_PWM.h"
#include "RGB.h"

Stepper_PWM_Typedef Stepper1;
Stepper_PWM_Typedef Stepper2;

// 更新限位LED：任一电机在限位就亮红灯
void Stepper_PWM_Limit_LED_Update(void)
{
    uint8_t s1 = (Stepper1.Limit_Enable &&
        (Stepper1.Pos_Now >= Stepper1.Limit_Angle_Max || Stepper1.Pos_Now <= Stepper1.Limit_Angle_Min));
    uint8_t s2 = (Stepper2.Limit_Enable &&
        (Stepper2.Pos_Now >= Stepper2.Limit_Angle_Max || Stepper2.Pos_Now <= Stepper2.Limit_Angle_Min));

    if (s1 || s2)  RGB_Set_Color(0, 0, 1);
    else           RGB_Set_Color(0, 0, 0);
}

// =================== 公共函数 ===================

// 初始化
void Stepper_PWM_Init(Stepper_PWM_Typedef* pStepper, MyPWM_Typedef* PWM, MyGPIO_Typedef* GPIO_Dir, float pulse_angle, int8_t Positive_Dir)
{
    // 驱动配置
    pStepper->GPIO_Dir = GPIO_Dir;
    pStepper->PWM  = PWM;
    // 参数配置
    pStepper->pulse_angle = pulse_angle;
    pStepper->Positive_Dir = Positive_Dir;

    // 运行时参数初始化
    pStepper->Pos_Now = 0;
    pStepper->Pos_Tar = 0;
    pStepper->Speed_Now = 0;

    // PID初始化在外部调
    // 限位默认关闭（外部调用Limit_Config启用）
    pStepper->Limit_Angle_Max = 360.0f;
    pStepper->Limit_Angle_Min = -360.0f;
    pStepper->Limit_Enable = 0;
    pStepper->Acc_Val = 0;
    pStepper->Speed_Tar = 0;

    // 位置模式初始化
    pStepper->Pos_MaxSpeed = 0;
    pStepper->Pos_Acc = 0;
    pStepper->Pos_Phase = POS_PHASE_IDLE;
    pStepper->Pos_TotalSteps = 0;
    pStepper->Pos_AccSteps = 0;
    pStepper->Pos_CruiseSteps = 0;
    pStepper->Pos_StepCnt = 0;
    pStepper->Pos_MoveDir = 1;
    pStepper->Pos_StartAngle = 0;
    pStepper->Pos_TargetAngle = 0;

    // 初始化DIR引脚（默认正转）
    MyGPIO_WritePin(pStepper->GPIO_Dir, Positive_Dir > 0 ? 1 : 0);
    // 初始化PWM
    MyPWM_Init(pStepper->PWM);
    MyPWM_SetCompare(pStepper->PWM, 0);  // 最开始占空比为0，也就是无脉冲
    // 配置NVIC并使能更新中断（用于脉冲计数）
    if (pStepper->PWM->htimx->Instance == TIM9) {
        HAL_NVIC_SetPriority(TIM1_BRK_TIM9_IRQn, 1, 0);
        HAL_NVIC_EnableIRQ(TIM1_BRK_TIM9_IRQn);
    } else if (pStepper->PWM->htimx->Instance == TIM12) {
        HAL_NVIC_SetPriority(TIM8_BRK_TIM12_IRQn, 1, 0);
        HAL_NVIC_EnableIRQ(TIM8_BRK_TIM12_IRQn);
    }
    __HAL_TIM_ENABLE_IT(pStepper->PWM->htimx, TIM_IT_UPDATE);
}
// =================== 内部：应用速度到硬件 ===================

static void _Stepper_Apply_Speed(Stepper_PWM_Typedef* pStepper, float Speed)
{
    // 第1层限位：前置门禁（位置模式下跳过，由位置模式自己的目标控制+Layer2兜底）
    if (pStepper->Pos_Phase == POS_PHASE_IDLE) {
        if (!Stepper_PWM_Limit_Check(pStepper, Speed)) {
            Stepper_PWM_Stop(pStepper);
            Stepper_PWM_Limit_LED_Update();
            return;
        }
    }

    int dir = (Speed * pStepper->Positive_Dir >= 0) ? 1 : 0;
    MyGPIO_WritePin(pStepper->GPIO_Dir, dir);

    float speed_abs = (Speed > 0) ? Speed : -Speed;
    if (speed_abs < 0.01f) {
        MyPWM_SetCompare(pStepper->PWM, 0);
        pStepper->Speed_Now = 0;
        return;
    }

    float freq_hz = speed_abs * (360.0f / pStepper->pulse_angle) / 60.0f;
    uint32_t tim_base = (pStepper->PWM->htimx->Instance == TIM9) ? MySystem_Fre : MySystem_Fre / 2;
    uint32_t tim_clock = tim_base / (pStepper->PWM->htimx->Instance->PSC + 1);
    uint32_t arr = (uint32_t)(tim_clock / freq_hz) - 1;
    if (arr < 1) arr = 1;
    if (arr > 65535) arr = 65535;

    __HAL_TIM_SET_AUTORELOAD(pStepper->PWM->htimx, arr);
    MyPWM_SetCompare(pStepper->PWM, (arr + 1) / 2);
    pStepper->Speed_Now = Speed;
}

// =================== 速度配置(rpm) ===================

// Speed: 目标速度 rpm，acc: 加速度 rpm/s，0=瞬时响应
void Stepper_PWM_Speed_Set(Stepper_PWM_Typedef* pStepper, float Speed, float acc)
{
    // 速度模式覆盖位置模式：取消正在进行的位控运动
    if (pStepper->Pos_Phase != POS_PHASE_IDLE) {
        pStepper->Pos_Phase = POS_PHASE_IDLE;
        pStepper->Pos_StepCnt = 0;
    }

    pStepper->Speed_Tar = Speed;
    pStepper->Acc_Val = acc;

    if (acc <= 0.001f) {
        _Stepper_Apply_Speed(pStepper, Speed);   // 无加速度：瞬时响应
    }
    // acc>0: 等待 Speed_Tick 在 1ms 中断中逐步 ramp
}

// 加速度Tick：每1ms调用一次，逐步逼近目标速度
// Acc_Val 单位: rpm/s，内部 /1000 得到每ms步长
void Stepper_PWM_Speed_Tick(Stepper_PWM_Typedef* pStepper)
{
    // 位置模式下 Speed_Tick 不干预速度（Pos_Tick 接管速度控制）
    if (pStepper->Pos_Phase != POS_PHASE_IDLE) return;

    if (pStepper->Acc_Val <= 0.001f) return;

    float step = pStepper->Acc_Val / 1000.0f;    // rpm/s → rpm/ms
    float diff = pStepper->Speed_Tar - pStepper->Speed_Now;

    if (diff > step)
        pStepper->Speed_Now += step;
    else if (diff < -step)
        pStepper->Speed_Now -= step;
    else
        pStepper->Speed_Now = pStepper->Speed_Tar;

    _Stepper_Apply_Speed(pStepper, pStepper->Speed_Now);
}


// 电机制动（停止）
void Stepper_PWM_Stop(Stepper_PWM_Typedef* pStepper)
{
    MyPWM_SetCompare(pStepper->PWM, 0);
    pStepper->Speed_Now = 0;
}

// =================== 脉冲中断处理（需要在TIM脉冲更新中断中调用） ===================

// 每当TIM计数器溢出（完成一个PWM脉冲）时调用此函数
// Speed_Now: >0=正转，<0=反转，0=停止
void Stepper_PWM_Pulse_Count(Stepper_PWM_Typedef* pStepper)
{
    if (pStepper->Speed_Now == 0)
    {
        return;
    }
    // 更新位置：Pos_Now单位为度
    int dir = (pStepper->Speed_Now * pStepper->Positive_Dir >= 0) ? 1 : -1;
    pStepper->Pos_Now += pStepper->pulse_angle * dir;

    // --- 位置模式：步数累加 + 到位停止 ---
    if (pStepper->Pos_Phase != POS_PHASE_IDLE) {
        pStepper->Pos_StepCnt++;

        // DECEL阶段到位停止（脉冲中断级，比1ms Tick更及时）
        if (pStepper->Pos_Phase == POS_PHASE_DECEL
            && pStepper->Pos_StepCnt >= pStepper->Pos_TotalSteps) {
            Stepper_PWM_Stop(pStepper);
            pStepper->Pos_Phase = POS_PHASE_IDLE;
            Stepper_PWM_Limit_LED_Update();
            return;
        }

        // 快速模式(acc<0.001)：直接以max_speed运行，脉冲中断检测到位即停
        // 用角度判断：按运动方向检查是否越过目标角度
        if (pStepper->Pos_Acc < 0.001f) {
            int reached = (pStepper->Pos_MoveDir > 0)
                ? (pStepper->Pos_Now >= pStepper->Pos_TargetAngle)
                : (pStepper->Pos_Now <= pStepper->Pos_TargetAngle);
            if (reached) {
                Stepper_PWM_Stop(pStepper);
                pStepper->Pos_Phase = POS_PHASE_IDLE;
                Stepper_PWM_Limit_LED_Update();
                return;
            }
        }
    }

    // 限位检查（脉冲中断级，每个脉冲都检查，即时性最高）
    // 位置模式下也保留作为安全兜底
    if (pStepper->Limit_Enable) {
        if (dir > 0 && pStepper->Pos_Now >= pStepper->Limit_Angle_Max) {
            Stepper_PWM_Stop(pStepper);
            pStepper->Pos_Phase = POS_PHASE_IDLE;  // 位控也取消
        }
        if (dir < 0 && pStepper->Pos_Now <= pStepper->Limit_Angle_Min) {
            Stepper_PWM_Stop(pStepper);
            pStepper->Pos_Phase = POS_PHASE_IDLE;
        }
    }

    // 更新限位LED
    Stepper_PWM_Limit_LED_Update();
}

// =================== 限位功能 ===================

// 配置软件限位
void Stepper_PWM_Limit_Config(Stepper_PWM_Typedef* pStepper, float Limit_Angle_Max, float Limit_Angle_Min)
{
    pStepper->Limit_Angle_Max = Limit_Angle_Max;
    pStepper->Limit_Angle_Min = Limit_Angle_Min;
    pStepper->Limit_Enable = 1;
}

// 限位检查：返回1=允许移动，0=被限位阻挡
uint8_t Stepper_PWM_Limit_Check(Stepper_PWM_Typedef* pStepper, float target_speed)
{
    if (!pStepper->Limit_Enable) return 1;

    // 判断目标运动方向：target_speed与Positive_Dir同号为正向
    int moving_positive = (target_speed * pStepper->Positive_Dir >= 0);

    if (moving_positive && pStepper->Pos_Now >= pStepper->Limit_Angle_Max) return 0;
    if (!moving_positive && pStepper->Pos_Now <= pStepper->Limit_Angle_Min) return 0;
    return 1;
}

// =================== 位置功能 ===================

// 绝对角度旋转
// target_angle: 目标绝对角度（度）
// max_speed: 最大速度（rpm），取绝对值
// acc: 加速度（rpm/s），<0.001=快速模式（直接到位，脉冲中断停止）
void Stepper_PWM_Pos_Set_Abs(Stepper_PWM_Typedef* pStepper, float target_angle, float max_speed, float acc)
{
    // 1. 取消速度模式
    pStepper->Speed_Tar = 0;
    pStepper->Acc_Val = 0;

    // 2. 计算角度差
    float delta_angle = target_angle - pStepper->Pos_Now;
    float abs_delta = (delta_angle > 0) ? delta_angle : -delta_angle;

    // 3. 零步运动检查
    if (abs_delta < pStepper->pulse_angle * 0.5f) {
        pStepper->Pos_Phase = POS_PHASE_IDLE;
        return;
    }

    // 4. 方向与总步数
    int8_t dir = (delta_angle >= 0) ? 1 : -1;
    int32_t total_steps = (int32_t)(abs_delta / pStepper->pulse_angle);
    if (total_steps < 1) total_steps = 1;

    // 5. 确保max_speed为正
    float max_spd = (max_speed > 0) ? max_speed : -max_speed;
    if (max_spd < 0.01f) max_spd = 60.0f;  // 默认60rpm

    // 6. 确保acc为正
    float acc_abs = (acc > 0) ? acc : -acc;

    // 7. 存储运动参数
    pStepper->Pos_TargetAngle = target_angle;
    pStepper->Pos_StartAngle  = pStepper->Pos_Now;
    pStepper->Pos_MaxSpeed    = max_spd;
    pStepper->Pos_Acc         = acc_abs;
    pStepper->Pos_MoveDir     = dir;
    pStepper->Pos_StepCnt     = 0;
    pStepper->Pos_TotalSteps  = total_steps;

    // 8. 快速模式（acc < 0.001）：直接以max_speed运行，脉冲中断到位停止
    if (acc_abs < 0.001f) {
        pStepper->Pos_Phase      = POS_PHASE_CRUISE;
        pStepper->Pos_AccSteps   = 0;
        pStepper->Pos_CruiseSteps = 0;
        _Stepper_Apply_Speed(pStepper, dir * max_spd);
        return;
    }

    // 9. 匀加速运动预计算
    // 公式: N = (v² - v₀²) * 3 / (acc * pulse_angle)
    float v_min = STEPPER_V_MIN;
    float a_eff = acc_abs * pStepper->pulse_angle / 3.0f;  // rpm²/step

    int32_t acc_steps;
    if (max_spd > v_min) {
        acc_steps = (int32_t)((max_spd * max_spd - v_min * v_min) / a_eff);
        if (acc_steps < 1) acc_steps = 1;
    } else {
        acc_steps = 1;  // max_speed太小，几乎瞬时
    }

    // 10. 场景判定
    if (acc_steps * 2 >= total_steps) {
        // 场景B：三角形（行程不足，无法达到max_speed）
        pStepper->Pos_AccSteps   = total_steps / 2;
        pStepper->Pos_CruiseSteps = 0;
        // 减速段步数 = 总步数 - 加速段步数（处理奇偶）
    } else {
        // 场景A：梯形（行程充足，能达到max_speed）
        pStepper->Pos_AccSteps   = acc_steps;
        pStepper->Pos_CruiseSteps = total_steps - 2 * acc_steps;
    }

    // 11. 启动：初始速度 = 方向 * 最小速度
    pStepper->Pos_Phase = POS_PHASE_ACCEL;
    _Stepper_Apply_Speed(pStepper, dir * v_min);
}

// 相对角度旋转
void Stepper_PWM_Pos_Set_Rel(Stepper_PWM_Typedef* pStepper, float relative_angle, float max_speed, float acc)
{
    float target = pStepper->Pos_Now + relative_angle;
    Stepper_PWM_Pos_Set_Abs(pStepper, target, max_speed, acc);
}

// =================== 角度到达检测 ===================

// 内部辅助：判断单个步进电机是否到达目标位置
// 返回 true 需同时满足：
//   1. 位置模式阶段为 IDLE（无进行中的位控运动）
//   2. |Speed_Now| < 0.01 rpm（电机已停止）
//   3. |Pos_Now - Pos_TargetAngle| <= 1.5 * pulse_angle（角度在容差内）
static bool _Stepper_Is_Angle_Single(const Stepper_PWM_Typedef* pStepper)
{
    // 条件1：位控运动中直接返回未到达
    if (pStepper->Pos_Phase != POS_PHASE_IDLE)
        return false;

    // 条件2：速度必须接近零（沿用 _Stepper_Apply_Speed 的 0.01 约定）
    float speed_abs = (pStepper->Speed_Now > 0) ? pStepper->Speed_Now : -pStepper->Speed_Now;
    if (speed_abs >= 0.01f)
        return false;

    // 条件3：角度在容差内（2.5 个脉冲 = 容忍 1 脉冲超调 + 浮点累积误差）
    float tolerance = 2.5f * pStepper->pulse_angle;
    float diff = pStepper->Pos_Now - pStepper->Pos_TargetAngle;
    if (diff < 0.0f) diff = -diff;

    if (diff <= tolerance)
        return true;

    return false;
}

// 公开 API：判断两个步进电机是否都已到达目标角度
// 速度 ≈ 0 且 当前角度 ≈ 目标角度，两个电机都满足才返回 true
bool Stepper_PWM_Is_Angle(void)
{
    return _Stepper_Is_Angle_Single(&Stepper1)
        && _Stepper_Is_Angle_Single(&Stepper2);
}

// 单电机版本：供需要单独判断某个电机的场景使用
bool Stepper_PWM_Is_Angle_Stepper(const Stepper_PWM_Typedef* pStepper)
{
    return _Stepper_Is_Angle_Single(pStepper);
}

// 位置模式 1ms Tick：速度ramp + 阶段切换
void Stepper_PWM_Pos_Tick(Stepper_PWM_Typedef* pStepper)
{
    if (pStepper->Pos_Phase == POS_PHASE_IDLE) return;

    // 快速模式：速度已在Pos_Set中设置，脉冲中断负责停止，Tick不需要干预
    if (pStepper->Pos_Acc < 0.001f) return;

    float v_min = STEPPER_V_MIN;
    int32_t step_cnt = pStepper->Pos_StepCnt;  // 由脉冲中断实时更新
    float speed_mag = (pStepper->Speed_Now > 0) ? pStepper->Speed_Now : -pStepper->Speed_Now;

    // --- 阶段切换（基于实际步数） ---
    switch (pStepper->Pos_Phase) {
    case POS_PHASE_ACCEL:
        if (step_cnt >= pStepper->Pos_AccSteps) {
            if (pStepper->Pos_CruiseSteps > 0) {
                pStepper->Pos_Phase = POS_PHASE_CRUISE;
            } else {
                pStepper->Pos_Phase = POS_PHASE_DECEL;
            }
        }
        break;

    case POS_PHASE_CRUISE:
        if (step_cnt >= pStepper->Pos_AccSteps + pStepper->Pos_CruiseSteps) {
            pStepper->Pos_Phase = POS_PHASE_DECEL;
        }
        break;

    case POS_PHASE_DECEL:
        // 到位停止由脉冲中断处理，1ms Tick作为兜底
        if (step_cnt >= pStepper->Pos_TotalSteps) {
            Stepper_PWM_Stop(pStepper);
            pStepper->Pos_Phase = POS_PHASE_IDLE;
            return;
        }
        break;
    }

    // --- 速度ramp（基于当前阶段） ---
    switch (pStepper->Pos_Phase) {
    case POS_PHASE_ACCEL:
        speed_mag += pStepper->Pos_Acc / 1000.0f;
        if (speed_mag > pStepper->Pos_MaxSpeed) {
            speed_mag = pStepper->Pos_MaxSpeed;
        }
        break;

    case POS_PHASE_CRUISE:
        speed_mag = pStepper->Pos_MaxSpeed;
        break;

    case POS_PHASE_DECEL:
        speed_mag -= pStepper->Pos_Acc / 1000.0f;
        if (speed_mag < v_min) {
            speed_mag = v_min;
        }
        break;
    }

    _Stepper_Apply_Speed(pStepper, speed_mag * pStepper->Pos_MoveDir);
}
