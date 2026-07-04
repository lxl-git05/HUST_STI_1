#ifndef __SERVO_H
#define __SERVO_H

#include "MySystem.h"

typedef enum 
{
    SERVO_TYPE_180,   // 0° ~ 180°
    SERVO_TYPE_360    // 0° ~ 360°（连续旋转）
} Servo_Type_t;

typedef struct 
{
		// 固定参数
    MyPWM_Typedef* Servo_PWM;    // PWM 句柄
    Servo_Type_t type;           // 舵机类型
		uint16_t pwm_min;            // 0度				->	PWM最小值
    uint16_t pwm_max;            // 最大度数	->	PWM最大值
		
		// 自设参数
    uint16_t pos_min;            // 角度限幅：允许的最小角度,并不一定是最小PWM对应的Pos
    uint16_t pos_max;            // 角度限幅：允许的最大角度，仅限幅使用
    int16_t current_pos;         // 当前角度，可用于初始化角度
    int16_t target_pos ;				 // 目标角度
		bool is_Step_Enable;				 // 是否激活步进控制
		uint16_t step_10ms	 ;			 // 步进角度,10ms增加step度,也就是 1s 增加 100 * step 度
		int tick_counter ;					 // 自身步进定时器(10ms)
} Servo_Typedef;

// 1. 舵机初始化
void Servo_Init
(
	Servo_Typedef* Servo, MyPWM_Typedef* Servo_PWM, Servo_Type_t type ,
	uint16_t pwm_min, uint16_t pwm_max ,uint16_t pos_min , uint16_t pos_max ,
	int16_t init_pos , uint16_t step_10ms
);

// 2. 舵机直接设置角度,也就是直接步进到目标角度
void Servo_SetDirectAngle(Servo_Typedef* Servo, int16_t angle);

// 3. 舵机设置目标角度,也就是均匀步进到目标角度
void Servo_SetGoalAngle(Servo_Typedef* Servo, int16_t target_pos);

// 4. 舵机步进控制台,放入1ms中断,自身存在10分频
void Servox_GoalAngle_Tick(Servo_Typedef* Servo) ;

// 5. 得到舵机当前角度
int Servo_Get_Angle(Servo_Typedef* Servo) ;

#endif
