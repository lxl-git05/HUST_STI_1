#ifndef __MYPID_H
#define __MYPID_H

typedef struct PID
{
	float goalPoint;		// 目标参数	

	float Kp;			// 比例系数				
	float Ki;			// 积分系数	
	float Kd;			// 微分系数		

	float LastError;	// 上次的误差
	float PreError;		// 本次误差
	float SumError;		// 累次积分值
	
	// 新增变量
	float realPoint_Now 	 ;	// 当前真实值
	float realPoint_Bef 	 ;	// 上次真实值
	
	float pout ;	// 比例系数输出值
	float iout ;	// 积分系数输出值
	float dout ;	// 微分系数输出值
	
	float OutMax ;		// 输出限幅最大值
	float OutMin ;		// 输出限幅最小值
	float ioutMax;		// 积分限幅
	float deadspace;	// 输入死区,暂时不使用
	float d_style;		// 微分先行系数,默认为0,也就是不微分先行,Init不进行配置,想用自己配
	float d_filter;		// 不完全微分(去除杂波滤波系数),默认为0,也就是不进行滤波,Init不进行配置,想用自己配
	
	float setPoint ;	// 即output,输出值
	
}Pid_Typedef;

// 用来一般化初始化PID结构体
void PID_Init(Pid_Typedef *pid, float kp, float ki, float kd , float OutMax , float OutMin , float ioutMax ) ;

// PID值更新,更新值直接写入PID的Output
void PID_Update(Pid_Typedef *pid, float ActualValue) ;

#endif
