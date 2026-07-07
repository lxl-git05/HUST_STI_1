#include "Encoder.h"

// 初始化
void Encoder_Init(IRQn_Type IRQN)
{
    // 使能GPIO外部中断
    NVIC_EnableIRQ(IRQN);
}

// ==================== 更新编码器(直接放在中断处理函数子位置) ====================
// 默认两个GPIO是同一个Port,编码器为双边触发
void Encoder_Counter_Tick(GPIO_Regs* Encoder_GPIO_Port , uint32_t Encoder_Pin_1 , uint32_t Encoder_Pin_2 , int32_t* gEncoderCount)   
{
    // 这里假设Pin1是中断模式(双边沿),Pin2只是普通引脚,参考用
    if (DL_GPIO_getEnabledInterruptStatus(Encoder_GPIO_Port, Encoder_Pin_1) == Encoder_Pin_1)
    {
        // 先读电平
        uint32_t pinA = DL_GPIO_readPins(Encoder_GPIO_Port, Encoder_Pin_1) ? 1 : 0;
        uint32_t pinB = DL_GPIO_readPins(Encoder_GPIO_Port, Encoder_Pin_2) ? 1 : 0;

        // 双边沿判断
        if (pinA == pinB)
            (*gEncoderCount)++;
        else
            (*gEncoderCount)--;

        // 清中断
        DL_GPIO_clearInterruptStatus(Encoder_GPIO_Port, Encoder_Pin_1);
    }
}



// ==================== 获取计数值并清零 ====================
int32_t Encoder_Get_CNT(int32_t* gEncoderCount)
{
    int32_t cnt = (*gEncoderCount);
    (*gEncoderCount) = 0;          // 清零，为下一次采样做准备
    return cnt;                 // 正数 = 正转，负数 = 反转
}
