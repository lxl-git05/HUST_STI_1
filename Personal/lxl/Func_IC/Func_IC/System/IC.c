#include "IC.h"

int IsFirstCapture ;
int IC_Val1 , IC_Val2;
int CapturePeriod ;

void IC_Init(void)
{
 // TIM初始化
HAL_TIM_Base_Start_IT(&IC_htim);              
// 启动更新中断
HAL_TIM_IC_Start_IT(&IC_htim, TIM_CHANNEL_1); // 启动输入捕获中断
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
 {
        // 判断是否是TIM5且通道1引发的输入捕获中断
    if (htim->Instance == IC_TIM && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
    {
        if (IsFirstCapture == 0)
        {
            // 记录得到第一次上升沿时间,单位为tick
            IC_Val1 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
            IsFirstCapture = 1;
        }
        else
        {
            // 第二次上升沿
            IC_Val2 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
            if (IC_Val2 > IC_Val1)
                CapturePeriod = IC_Val2 - IC_Val1;
            else
                CapturePeriod = (0xFFFFFFFF - IC_Val1) + IC_Val2 + 1;
            IsFirstCapture = 0;
        }
    }
 }
 
void IC_Capture_Update(void)
{
	HAL_TIM_IC_CaptureCallback(&IC_htim) ;
}

