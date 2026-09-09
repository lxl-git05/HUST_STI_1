#include "Encoder_Key.h"

int16_t Encoder_Count = 0;   // 编码器计数值

void Encoder_Init(void)
{
    // PF3/PF7 的 GPIO 和 NVIC 配置已在 gpio.c 的 MX_GPIO_Init() 中完成
}

int16_t Encoder_Get(void)
{
    int16_t temp = Encoder_Count;
    Encoder_Count = 0;
    return temp;
}

// HAL_GPIO_EXTI_Callback 由 stm32f4xx_it.c 中的 EXTI3/EXTI9_5_IRQHandler
// 调用 HAL_GPIO_EXTI_IRQHandler，再由后者调用此回调
// 此函数是弱定义的，会覆盖 HAL 库中的空实现
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == EC11_S1_Pin)            // PF3
    {
        // 去抖动：再次确认 PF3 仍为低电平
        if (HAL_GPIO_ReadPin(EC11_S1_GPIO_Port, EC11_S1_Pin) == GPIO_PIN_RESET)
        {
            // 读取 PF7 电平判断方向
            // 下降沿触发时：PF7低→正转++，PF7高→反转--
            if (HAL_GPIO_ReadPin(EC11_S2_GPIO_Port, EC11_S2_Pin) == GPIO_PIN_RESET)
            {
                Encoder_Count++;
            }
        }
    }
    else if (GPIO_Pin == EC11_S2_Pin)       // PF7
    {
        // 去抖动：再次确认 PF7 仍为低电平
        if (HAL_GPIO_ReadPin(EC11_S2_GPIO_Port, EC11_S2_Pin) == GPIO_PIN_RESET)
        {
            // 读取 PF3 电平判断方向
            // 下降沿触发时：PF3低→反转--，PF3高→正转++
            if (HAL_GPIO_ReadPin(EC11_S1_GPIO_Port, EC11_S1_Pin) == GPIO_PIN_RESET)
            {
                Encoder_Count--;
            }
        }
    }
}
