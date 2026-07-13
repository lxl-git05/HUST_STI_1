#ifndef __STEPPER_PWM_H
#define __STEPPER_PWM_H

#include "MySystem.h"
#include "MyPID.h"

#define STEPPER_DIR_P   (1)    // 正向
#define STEPPER_DIR_N   (-1)   // 反向

typedef struct
{
    // 驱动(默认使能)
    MyPWM_Typedef* PWM;
    MyGPIO_Typedef* GPIO_Dir;           // 方向GPIO
    // 电机参数
    float pulse_angle;                  // 单位脉冲角度(度)
    int8_t Positive_Dir;                // 正方向(1 or -1)
    // 内部参数
    float Pos_Now;                    	// 当前旋转的绝对角度(度)
    float Pos_Tar;                  		// 目标角度(度)
    float Speed_Now;                    // 当前的速度(rpm)，0=停止，>0=正转，<0=反转
		// PID参数
		Pid_Typedef PID_Angle;							// 香橙派角度对应PID
} Stepper_PWM_Typedef;

extern Stepper_PWM_Typedef Stepper1 ;
extern Stepper_PWM_Typedef Stepper2 ;

// 初始化
void Stepper_PWM_Init(Stepper_PWM_Typedef* pStepper , MyPWM_Typedef* PWM , MyGPIO_Typedef* GPIO_Dir , float pulse_angle , int8_t Positive_Dir) ;

// 速度配置(rpm)
void Stepper_PWM_Speed_Set(Stepper_PWM_Typedef* pStepper, float Speed);

// 电机制动（停止）
void Stepper_PWM_Stop(Stepper_PWM_Typedef* pStepper);

// 脉冲中断处理（每脉冲完成调用一次，在TIM12更新中断中调用）
void Stepper_PWM_Pulse_Count(Stepper_PWM_Typedef* pStepper);

// =================== 脉冲输出控制 ===================

#endif
