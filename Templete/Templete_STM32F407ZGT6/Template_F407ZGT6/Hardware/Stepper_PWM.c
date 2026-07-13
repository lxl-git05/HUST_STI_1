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
    // 第1层限位：前置门禁
    if (!Stepper_PWM_Limit_Check(pStepper, Speed)) {
        Stepper_PWM_Stop(pStepper);
        Stepper_PWM_Limit_LED_Update();
        return;
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

// Speed: 目标速度，acc: 加速度步进（rpm/Tick），0=瞬时响应
void Stepper_PWM_Speed_Set(Stepper_PWM_Typedef* pStepper, float Speed, float acc)
{
    pStepper->Speed_Tar = Speed;
    pStepper->Acc_Val = acc;

    if (acc <= 0.001f) {
        _Stepper_Apply_Speed(pStepper, Speed);   // 无加速度：瞬时响应
    }
    // acc>0: 等待 Speed_Tick 在 20ms 中断中逐步 ramp
}

// 加速度Tick：每20ms调用一次，逐步逼近目标速度
void Stepper_PWM_Speed_Tick(Stepper_PWM_Typedef* pStepper)
{
    if (pStepper->Acc_Val <= 0.001f) return;

    float diff = pStepper->Speed_Tar - pStepper->Speed_Now;
    float applied;
    if (diff > pStepper->Acc_Val)
        applied = pStepper->Speed_Now + pStepper->Acc_Val;
    else if (diff < -pStepper->Acc_Val)
        applied = pStepper->Speed_Now - pStepper->Acc_Val;
    else
        applied = pStepper->Speed_Tar;

    _Stepper_Apply_Speed(pStepper, applied);
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
    pStepper->Pos_Now += pStepper->pulse_angle * dir ;

    // 限位检查（脉冲中断级，每个脉冲都检查，即时性最高）
    if (pStepper->Limit_Enable) {
        if (dir > 0 && pStepper->Pos_Now >= pStepper->Limit_Angle_Max) {
            Stepper_PWM_Stop(pStepper);
        }
        if (dir < 0 && pStepper->Pos_Now <= pStepper->Limit_Angle_Min) {
            Stepper_PWM_Stop(pStepper);
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


