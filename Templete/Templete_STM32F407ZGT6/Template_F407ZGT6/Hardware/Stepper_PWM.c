#include "Stepper_PWM.h"

Stepper_PWM_Typedef Stepper1;
Stepper_PWM_Typedef Stepper2;

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

// 速度配置(rpm)
// Speed: 目标速度（转/分钟），正=正向(POSITIVE_DIR)，负=反向
void Stepper_PWM_Speed_Set(Stepper_PWM_Typedef* pStepper, float Speed)
{
    // 计算方向：Speed和Positive_Dir同号为正向，异号为反向
    int dir = (Speed * pStepper->Positive_Dir >= 0) ? 1 : 0;
    MyGPIO_WritePin(pStepper->GPIO_Dir, dir);

    // 计算目标速度的绝对值
    float speed_abs = (Speed > 0) ? Speed : -Speed;
    if (speed_abs < 0.01f) {
        // 速度为0，停止
        MyPWM_SetCompare(pStepper->PWM, 0);
        pStepper->Speed_Now = 0;
        return;
    }

    // 计算需要的脉冲频率
    // freq_hz = RPM × (360° / pulse_angle) / 60
    float freq_hz = speed_abs * (360.0f / pStepper->pulse_angle) / 60.0f;

    // 根据目标频率计算ARR
    // APB2定时器(TIM9): MySystem_Fre,  APB1定时器(TIM12): MySystem_Fre/2
    uint32_t tim_base = (pStepper->PWM->htimx->Instance == TIM9) ? MySystem_Fre : MySystem_Fre / 2;
    uint32_t tim_clock = tim_base / (pStepper->PWM->htimx->Instance->PSC + 1);
    uint32_t arr = (uint32_t)(tim_clock / freq_hz) - 1;

    // 限幅
    if (arr < 1) arr = 1;
    if (arr > 65535) arr = 65535;

    // 设置ARR和占空比50%
    __HAL_TIM_SET_AUTORELOAD(pStepper->PWM->htimx, arr);
    MyPWM_SetCompare(pStepper->PWM, (arr + 1) / 2);

    // 更新内部参数（保留原始值，包含正负号）
    pStepper->Speed_Now = Speed;
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
}
