#ifndef __MOTOR_H
#define __MOTOR_H

#include "MySystem.h"
#include "MyPID.h"

#define Motor_DIR_P ( 1)
#define Motor_DIR_N (-1)

typedef struct
{
	float PPR;
	float ReductionRatio;
	int Motor_Max_Speed ;
	float Wheel_Cm;			// 轮子周长(cm) — 用于位置环距离计算（按实际带轮周长修改）
}Motor_Param_Typedef ;

typedef enum
{
    MOTOR_STOP = 0,
    MOTOR_RUN,
    MOTOR_BRAKE
}Motor_State_Typedef;

typedef struct
{
	// 1. PWM
	MyPWM_Typedef* Motor_PWM ;
	// 2. Encoder
	MyEncoder_Typedef* Motor_Encoder ;
	// 3. GPIO * 2
	MyGPIO_Typedef* Motor_GPIO_IN1 ;
	MyGPIO_Typedef* Motor_GPIO_IN2 ;
	// 4. Param
	Motor_Param_Typedef* Motor_Param ;
	// 5. Dir
	int8_t PWM_Dir;
	int8_t Encoder_Dir ;
	// 6. PID
	Pid_Typedef PID_s ;
	Pid_Typedef PID_Angle ;
	Pid_Typedef PID_Pos		;	// 位置环(一般为外环)
	// 7. 状态标记
	Motor_State_Typedef State ;
	// 8. 角度环开关（位置环/直行环任务会临时关闭，避免与全局 20ms 角度环冲突）
	uint8_t Angle_Ring_Enable ;
}Motor_Typedef ;

// 1. 初始化
void Motor_Init
(
		Motor_Typedef* pMotor, MyPWM_Typedef* pwm, MyEncoder_Typedef* encoder,
		MyGPIO_Typedef* in1, MyGPIO_Typedef* in2,
		Motor_Param_Typedef* param, int8_t pwm_dir, int8_t encoder_dir,
		Pid_Typedef PID_s, Pid_Typedef PID_Angle, Pid_Typedef PID_Pos
) ;

// 2. 设置PWM值
void Motor_SetPWM(Motor_Typedef *Motor , int PWM) ;

// 3. 得到一轮内的平均速度
void Motor_Speed_Update(Motor_Typedef *Motor , uint32_t Gap_Time_ms) ;

// 4. 得到电机当前转动的角度
void Motor_Angle_Update(Motor_Typedef *Motor) ;

// 5. 得到当前电机走过的位移(cm)
void Motor_Pos_Update(Motor_Typedef *Motor) ;
#endif
