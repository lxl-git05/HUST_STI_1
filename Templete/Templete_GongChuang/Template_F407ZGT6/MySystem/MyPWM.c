#include "MyPWM.h"

// PWM初始化 — 校验ARR配置是否合理
void MyPWM_Init(MyPWM_Typedef *pwm)
{
    uint32_t arr = __HAL_TIM_GET_AUTORELOAD(pwm->htimx);

    // ARR异常则卡死（方便调试发现配置问题）
    if (arr == 0 || arr > 65535) {
        while (1) { }
    }
    HAL_TIM_PWM_Start(pwm->htimx, pwm->Channel);
}

// 设置PWM比较值 — 自动限幅到[Compare_Min, Compare_Max]
void MyPWM_SetCompare(MyPWM_Typedef *pwm, float compare)
{
    if (compare > pwm->Compare_Max) compare = pwm->Compare_Max;
    if (compare < pwm->Compare_Min) compare = pwm->Compare_Min;
    __HAL_TIM_SET_COMPARE(pwm->htimx, pwm->Channel, (uint32_t)compare);
}

// 获取PWM频率
int MyPWM_GetFre(MyPWM_Typedef *pwm)
{
    return MySystem_Fre
         / (pwm->htimx->Instance->ARR + 1)
         / (pwm->htimx->Instance->PSC + 1);
}

// 设置定时器周期值（ARR），用于步进电机动态调速
void MyPWM_SetLoadValue(MyPWM_Typedef *pwm, uint32_t load)
{
    if (pwm == 0 || pwm->htimx == 0) {
        return;
    }
    if (load < 1)  load = 1;
    if (load > 65535) load = 65535;
    __HAL_TIM_SET_AUTORELOAD(pwm->htimx, load);
}

// 获取定时器输入时钟频率(Hz)
uint32_t MyPWM_GetTimClock(MyPWM_Typedef *pwm)
{
    if (pwm == 0 || pwm->htimx == 0) {
        return 0;
    }
    if (pwm->Tim_Clock > 0) {
        return pwm->Tim_Clock;
    }
    return MySystem_Fre;  // 兜底：未配置时用主频
}

// 使能定时器更新中断（用于脉冲计数，步进电机专用）
void MyPWM_EnableIT(MyPWM_Typedef *pwm)
{
    if (pwm == 0 || pwm->htimx == 0) {
        return;
    }
    // 优先级=1（低于 1ms Tick 的优先级 0，保证 Tick 不掉）
    HAL_NVIC_SetPriority(pwm->Tim_IRQn, 1, 0);
    NVIC_ClearPendingIRQ(pwm->Tim_IRQn);
    HAL_NVIC_EnableIRQ(pwm->Tim_IRQn);
    // 使能计数器更新中断（每个 PWM 周期触发一次 → 脉冲计数）
    __HAL_TIM_ENABLE_IT(pwm->htimx, TIM_IT_UPDATE);
}
