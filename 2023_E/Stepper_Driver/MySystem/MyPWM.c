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
