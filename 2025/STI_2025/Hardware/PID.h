#ifndef __PID_H__
#define __PID_H__


typedef struct PID
{
	float goalPoint;		// 目标值

	float Kp;			// kp
	float Ki;			// ki
	float Kd;			// kd

	float LastError;	// 上次误差
	float PreError;		// 本次误差
	float SumError;		// 总误差	
	float dError;		  // 误差微分

}Pid_Typedef;


void PID_Init(Pid_Typedef *pid, float kp, float ki, float kd ,  float goalPoint) ;
float PID_Cal(Pid_Typedef *pid, float ActualValue , float OutputMin , float OutputMax) ;
void PID_Set(Pid_Typedef *pid, float kp, float ki, float kd , float goalPoint) ;


#endif
