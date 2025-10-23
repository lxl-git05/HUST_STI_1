#ifndef __MOTOR_H
#define __MOTOR_H

#include "main.h"
#include "tim.h"

#include "Encoder.h"
#include "MyPID.h"

// 电机PWM
#define Motor_A_PWM_Htim htim1			// PWM定时器位置
#define Motor_B_PWM_Htim htim1			// PWM定时器位置

#define Motor_A_PWM_Channel TIM_CHANNEL_1	// PWM通道
#define Motor_B_PWM_Channel TIM_CHANNEL_4	// PWM通道

// 电机Encoder
#define Motor_A_Encoder_htim htim2	// Encoder定时器位置
#define Motor_B_Encoder_htim htim3	// Encoder定时器位置

#define Encoder_PID_Gap_Time 20			// 编码器测速和PID更新,采样时间均为20ms/次

// 电机速度
#define Motor_MAX_Speed 370					// 最大转速(goal的最大值)

// 电机正方向
#define DIR_P ( 1)
#define DIR_N (-1)

// *********函数*********

// 电机参数
typedef struct
{
	TIM_HandleTypeDef Motor_Encoder_htim ;				// 电机的编码器htim
	
	TIM_HandleTypeDef Motor_PWM_htim ;						// 电机的PWM_TIMx
	uint32_t Motor_PWM_Channel ;			// 电机的PWM通道
	
	GPIO_TypeDef *IN1_Port ; 						// 电机IN1引脚类型,必须为PWM
	uint16_t IN1_Pin  ;									// 电机IN1的引脚号
	
	GPIO_TypeDef *IN2_Port ; 						// 电机IN2的引脚类型,建议为普通引脚
	uint16_t IN2_Pin  ;									// 电机IN2的引脚号
	
	float PPR;                          // 编码器线数
	float ReductionRatio;               // 减速比
	int8_t DIR;                         // 正方向
	
	int GoalSpeed	;											// 电机目标速度
	int RealSpeed	;											// 电机实际速度
	int SetSpeed	;											// 电机设定速度
	
	Pid_Typedef PID_s ;									// PID参数
	
}Motor_Typedef ;

// 初始化函数:包含PWM,Encoder,PID初始化
void Motor_A_Init(void) ;																		// Motor_A初始化
// Motor_B初始化
void Motor_B_Init(void)	;																		// Motor_B初始化

// 速度函数
void Motor_SetGoalSpeed(Motor_Typedef *Motor , int speed) ;	// 设置目标速度,调用者执行的逻辑

void Motor_Speed_Update(Motor_Typedef *Motor) ;							// 测速,使用M法公式,得到Motor的转速:n圈/s !务必配置计时器!

void Motor_SetPWM(Motor_Typedef *Motor , int PWM) ;					// 设置PWM,幕后执行的速度逻辑(setPoint)

// 电机PID计算与更新,!记得配置计时器!
void Motor_PID_Update(Motor_Typedef *Motor) ;

#endif

