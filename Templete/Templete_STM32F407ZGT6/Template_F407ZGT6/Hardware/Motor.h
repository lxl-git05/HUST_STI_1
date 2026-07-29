#ifndef __MOTOR_H
#define __MOTOR_H

#include "MySystem.h"
#include "MyPID.h"

// 电机正方向
#define Motor_DIR_P ( 1)
#define Motor_DIR_N (-1)

// 电机自身参数
typedef struct
{
	float PPR;           	// 编码器线数
	float ReductionRatio;	// 减速比
	int Motor_Max_Speed ;	// 最大速度(限速,建议比最高速度还低一些)
	float Wheel_Cm;			// 轮子周长(cm) — 用于位置环距离计算
}Motor_Param_Typedef ;

// 电机状态
typedef enum
{
    MOTOR_STOP = 0, // 停车
    MOTOR_RUN,      // 允许发车
    MOTOR_BRAKE     // 急刹车
}Motor_State_Typedef;


// 电机参数
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
	int8_t PWM_Dir;				// PWM正方向
	int8_t Encoder_Dir ;	// 编码器正方向
	// 6. PID
	Pid_Typedef PID_s ;			// 速度环(一般为内环)
	Pid_Typedef PID_Angle ;		// 角度环(一般为外环)
	Pid_Typedef PID_Pos		;	// 位置环(一般为外环)
	// 7. 状态参数
	Motor_State_Typedef State ;
}Motor_Typedef ;

// 1. 初始化
void Motor_Init
(								
		Motor_Typedef* pMotor, MyPWM_Typedef* pwm, MyEncoder_Typedef* encoder,
		MyGPIO_Typedef* in1,MyGPIO_Typedef* in2,
		Motor_Param_Typedef* param,int8_t pwm_dir,int8_t encoder_dir,
		Pid_Typedef  PID_s , Pid_Typedef PID_Angle , Pid_Typedef PID_Pos
) ;

// 2. 设置PWM值
void Motor_SetPWM(Motor_Typedef *Motor , int PWM) ;

// 3. 得到一段周期内电机的速度,使用M法测速公式,得到Motor的转速:n圈/s
void Motor_Speed_Update(Motor_Typedef *Motor , uint32_t Gap_Time_ms) ;

// 4. 得到当前电机旋转的角度
void Motor_Angle_Update(Motor_Typedef *Motor) ;

// 5. 得到当前电机走过的位移(cm)
void Motor_Pos_Update(Motor_Typedef *Motor) ;
#endif
