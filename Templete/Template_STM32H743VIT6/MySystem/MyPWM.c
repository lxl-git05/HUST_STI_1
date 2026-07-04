#include "MyPWM.h"

// 0. PWM定义
MyPWM_Typedef MyPWM_Servo1      = {&htim1 , TIM_CHANNEL_1 , 2000.0f};

MyPWM_Typedef MyPWM_Servo2      = {&htim1 , TIM_CHANNEL_2 , 2000.0f};	// 由于引脚损坏，所以改为TIM8_CH3: PC8

MyPWM_Typedef MyPWM_Servo3      = {&htim1 , TIM_CHANNEL_3 , 2000.0f};
MyPWM_Typedef MyPWM_Servo4      = {&htim1 , TIM_CHANNEL_4 , 2000.0f};
MyPWM_Typedef MyPWM_Motor_A_IN1 = {&htim4 , TIM_CHANNEL_1 , 1000.0f};
MyPWM_Typedef MyPWM_Motor_B_IN1 = {&htim4 , TIM_CHANNEL_2 , 1000.0f};

// 1. PWM初始化 - 检查ARR和PWM_MAX是否相等
void MyPWM_Init(MyPWM_Typedef* MyPWM)
{
    // 获取定时器的自动重装载值
    uint32_t arr = __HAL_TIM_GET_AUTORELOAD(MyPWM->htimx);
    
    // 检查ARR和PWM_MAX是否相等
    if(arr + 1 != MyPWM->PWM_MAX)
    {
        // 不相等，进入死循环
        while(1)
        {
            // 666乱写MAX
        }
    }
    // 启动PWM输出
    HAL_TIM_PWM_Start(MyPWM->htimx, MyPWM->Channel);
}

// 2. 设置PWM值 - 添加限幅功能
void MyPWM_SetCompare(MyPWM_Typedef* MyPWM, uint16_t Compare)
{
    uint16_t limited_compare = Compare;
    
    // 限幅：确保Compare值在0到PWM_MAX之间
    if(Compare > MyPWM->PWM_MAX)
    {
        limited_compare = MyPWM->PWM_MAX;
    }
    else if(Compare < 0)
    {
        limited_compare = 0;
    }
    
    // 设置比较值
    __HAL_TIM_SET_COMPARE(MyPWM->htimx, MyPWM->Channel, limited_compare);
}

// 3. 得到PWM的频率
int MyPWM_GetFre(MyPWM_Typedef* MyPWM)
{
		return MySystem_Fre / (MyPWM->htimx->Instance->ARR + 1) / (MyPWM->htimx->Instance->PSC + 1) ;
}
