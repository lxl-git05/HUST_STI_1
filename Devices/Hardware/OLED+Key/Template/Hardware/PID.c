#include "stm32f1xx.h"                  // Device header
#include "main.h"

typedef struct PID
{
	float goalPoint;		// 目标参数	

	float Kp;			// 比例系数				
	float Ki;			// 积分系数	
	float Kd;			// 微分系数		

	float LastError;	// 上次的误差
	float PreError;		// 本次误差
	float SumError;		// 积分误差(误差和)	
	float dError;		// 微分误差(本次-上次)

//	float IMax;			// 积分最值		
//	float POut;			// 比例输出		
//	float IOut;			// 积分输出
//	float DOut;			// 微分输出
}Pid_Typedef;

// 用来一般化初始化PID结构体
void PID_Init(Pid_Typedef *pid, float kp, float ki, float kd , float goalPoint)
{
	  pid->goalPoint = goalPoint;
	
    pid->Kp = kp;
    pid->Ki = ki;
    pid->Kd = kd;

    pid->LastError = 0;
    pid->PreError = 0;
    pid->SumError = 0;
  	pid->dError = 0 ;
}

// 用来确定各个PID的系数,调试专用
void PID_Set(Pid_Typedef *pid, float kp, float ki, float kd , float goalPoint)
{
	  pid->goalPoint = goalPoint;
	
    pid->Kp = kp;
    pid->Ki = ki;
    pid->Kd = kd;
}

// 计算PID
float PID_Cal(Pid_Typedef *pid, float ActualValue , float OutputMin , float OutputMax)
{
	// 更新上次误差
	pid->LastError = pid->PreError;
	// 得到本次误差
	pid->PreError = pid->goalPoint - ActualValue;
	// 微分误差
	pid->dError = pid->PreError - pid->LastError;
	// 特殊处理累次积分误差(防止调参出现突变)
	if ( pid->Ki > 0.00001f || pid->Ki < -0.00001f )
	{
		pid->SumError += pid->PreError;
	}
	else
	{
		pid->SumError = 0 ;
	}
	// 计算PID: Out = Kp * Error0 + Ki * ErrorInt + Kd * ( Error0 - Error1 ) ;
	float Output = pid->Kp * pid->PreError + pid->Ki * pid->SumError + pid->Kd * pid->dError ;
	
	// 输出限幅
	if ( Output > OutputMax ) { Output = OutputMax ; }
	if ( Output < OutputMin ) { Output = OutputMin ; }
	
	return Output ;	// 新的设定值
}
