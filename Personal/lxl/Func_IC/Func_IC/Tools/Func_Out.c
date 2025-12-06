#include "Func_Out.h"
#include "PWM.h"

// 函数发生器的TIM配置
#define Func_Out_TIM  TIM1
#define Func_Out_htim htim1
#define Func_Out_Channel	TIM_CHANNEL_1

// 函数发生器初始化
void Func_Out_Init(void)
{
	PWM_Init(Func_Out_htim , Func_Out_Channel) ;	// PWM输出
}

// 函数发生器输出函数
void Func_Out_Square(uint32_t frequency, uint32_t duty_cycle)
{
    if (frequency == 0 || duty_cycle > 100) return;

    // 1. 读取 PSC
    uint32_t psc = Func_Out_htim.Init.Prescaler;

    // 2. 计算定时器时钟 = APB 时钟 ×2(如果APB预分频不为1)
    uint32_t timclk;
    
    if (((uint32_t)Func_Out_htim.Instance) >= APB2PERIPH_BASE) 
		{
        // 定时器属于 APB2，例如 TIM1
        uint32_t apb2 = HAL_RCC_GetPCLK2Freq();
        timclk = (RCC->CFGR & RCC_CFGR_PPRE2_DIV1) ? apb2 : apb2 * 2;
    } 
		else 
		{
        // 定时器属于 APB1，例如 TIM2~TIM5
        uint32_t apb1 = HAL_RCC_GetPCLK1Freq();
        timclk = (RCC->CFGR & RCC_CFGR_PPRE1_DIV1) ? apb1 : apb1 * 2;
    }

    // 3. 根据 frequency 计算 ARR
    uint32_t arr = timclk / (psc + 1) / frequency;
    if (arr <= 1) return;
    arr -= 1;

    // 4. 计算 CCR
    uint32_t ccr = (arr + 1) * duty_cycle / 100;

    // 5. 更新 TIM 参数
    __HAL_TIM_SET_AUTORELOAD(&Func_Out_htim, arr);
    __HAL_TIM_SET_COMPARE(&Func_Out_htim, Func_Out_Channel, ccr);
}

