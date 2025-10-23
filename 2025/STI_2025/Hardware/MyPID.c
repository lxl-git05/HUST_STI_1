#include "stm32f1xx.h"                  // Device header
#include "main.h"
#include "math.h"
#include "MyPID.h"

// 用来一般化初始化PID结构体
void PID_Init(Pid_Typedef *pid, float kp, float ki, float kd , float OutMax , float OutMin , float ioutMax )
{
	pid->Kp = kp;
	pid->Ki = ki;
	pid->Kd = kd;
	
	pid->OutMax = OutMax 	 ;	// 输出限幅最大值
	pid->OutMin = OutMin 	 ;	// 输出限幅最小值
	pid->ioutMax = ioutMax ;	// 积分限幅
}

// PID值更新,更新值直接写入PID的Output
void PID_Update(Pid_Typedef *pid, float ActualValue)
{
	// 更新上次误差
	pid->LastError = pid->PreError;
	// 得到本次误差
	pid->PreError = pid->goalPoint - ActualValue;
	// 更新真实值(Actual)
	pid->realPoint_Bef = pid->realPoint_Now ;
	pid->realPoint_Now = ActualValue ;
	// 微分误差:*微分先行*(默认为0,也就是不先行)
	float dError = (1.0f - pid->d_style)*(pid->PreError - pid->LastError) - pid->d_style * (pid->realPoint_Now - pid->realPoint_Bef);
	// 累次积分
	pid->SumError += pid->PreError ;
	// *积分限幅*
	if (pid->SumError > pid->ioutMax)
	{
		pid->SumError = pid->ioutMax ;
	}
	else if (pid->SumError < -pid->ioutMax)
	{
		pid->SumError = -pid->ioutMax ;
	}
	// 输出参数
	pid->pout = pid->Kp * pid->PreError ;
	pid->iout = pid->Ki * pid->SumError ;	// 这里还可以进行积分分离+变速积分,后续有需要再加入
	pid->dout = pid->Kd * (1.0f - pid->d_filter) * dError + pid->d_filter * pid->dout  ;		// *不完全微分*(滤波,默认不滤波)

	// 得到PID的输出值
	pid->setPoint = pid->pout + pid->iout + pid->dout ;
	
	// 输出限幅
	if ( pid->setPoint > pid->OutMax ) { pid->setPoint = pid->OutMax ; }
	if ( pid->setPoint < pid->OutMin ) { pid->setPoint = pid->OutMin ; }
}
