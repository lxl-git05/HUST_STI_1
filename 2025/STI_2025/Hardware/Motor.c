#include "Motor.h"

Motor_Typedef Motor_A ;
Motor_Typedef Motor_B ;

// Motor_A初始化
void Motor_A_Init(void)
{
	// 设定Motor_A相关参数
	Motor_A.Motor_Encoder_htim = Motor_A_Encoder_htim		;	// 编码器TIM_htim
	
	Motor_A.Motor_PWM_htim   	= Motor_A_PWM_Htim		  ;		// PWM_TIM
	Motor_A.Motor_PWM_Channel = Motor_A_PWM_Channel  ;
	
	Motor_A.IN1_Port			= Motor_A_IN1_GPIO_Port	;
	Motor_A.IN1_Pin			  = Motor_A_IN1_Pin ;
	
	Motor_A.IN2_Port			= Motor_A_IN2_GPIO_Port	;
	Motor_A.IN2_Pin			  = Motor_A_IN2_Pin ;
	
	Motor_A.PPR = 13.0f ;
	Motor_A.ReductionRatio = 28.0f ;
	
	Motor_A.DIR = DIR_P ;		// 方向判断
	
	// PWM初始化
	HAL_TIM_PWM_Start(&Motor_A.Motor_PWM_htim , Motor_A.Motor_PWM_Channel) ;	
	
	// 编码器初始化
	Encoder_Init(&Motor_A_Encoder_htim) ;
	
	// PID初始化
	PID_Init(&Motor_A.PID_s , 0.72f , 0.09f , 0.1f , 100 , -100 , 10000 ) ;
	
}

// Motor_B初始化
void Motor_B_Init(void)
{
	// 设定Motor_B相关参数
	Motor_B.Motor_Encoder_htim = Motor_B_Encoder_htim		;	// 编码器TIM_htim
	
	Motor_B.Motor_PWM_htim   	= Motor_B_PWM_Htim		  ;		// PWM_TIM
	Motor_B.Motor_PWM_Channel = Motor_B_PWM_Channel  ;
	
	Motor_B.IN1_Port			= Motor_B_IN1_GPIO_Port	;
	Motor_B.IN1_Pin			  = Motor_B_IN1_Pin ;
	
	Motor_B.IN2_Port			= Motor_B_IN2_GPIO_Port	;
	Motor_B.IN2_Pin			  = Motor_B_IN2_Pin ;
	
	Motor_B.PPR = 13.0f ;
	Motor_B.ReductionRatio = 28.0f ;
	
	Motor_B.DIR = DIR_P ;	// 方向判断
	
	// PWM初始化
	HAL_TIM_PWM_Start(&Motor_B.Motor_PWM_htim , Motor_B.Motor_PWM_Channel) ;
	
	// 编码器初始化
	Encoder_Init(&Motor_B_Encoder_htim) ;
	
	// PID初始化
	PID_Init(&Motor_B.PID_s , 0.72f , 0.09f , 0.1f , 100 , -100 , 10000 ) ;
}
// 调用者执行的逻辑,设置目标速度
void Motor_SetGoalSpeed(Motor_Typedef *Motor , int speed)
{
	// 限制最值
	if (speed >= Motor_MAX_Speed)
	{
		speed = Motor_MAX_Speed ;
	}
	else if (speed <= -Motor_MAX_Speed)
	{
		speed = -Motor_MAX_Speed ;
	}
	// 判断方向,更新目标值
	Motor->GoalSpeed = speed * Motor->DIR;
}

// 设置PWM,幕后执行的速度逻辑(setPoint)
void Motor_SetPWM(Motor_Typedef *Motor , int PWM)
{
	// 限制最值
	if (PWM >= Motor_MAX_Speed)
	{
		PWM = Motor_MAX_Speed ;
	}
	else if (PWM <= -Motor_MAX_Speed)
	{
		PWM = -Motor_MAX_Speed ;
	}
	// 判断方向,设置速度
	if (PWM >= 0)
	{
		HAL_GPIO_WritePin(Motor->IN2_Port , Motor->IN2_Pin , GPIO_PIN_RESET ) ;			// 低电平,0
		__HAL_TIM_SET_COMPARE(&Motor->Motor_PWM_htim ,Motor->Motor_PWM_Channel , PWM ) ;	// 变大	 ,差值变大
	}
	else
	{
		HAL_GPIO_WritePin(Motor->IN2_Port , Motor->IN2_Pin , GPIO_PIN_SET ) ;				// 高电平,100
		__HAL_TIM_SET_COMPARE(&Motor->Motor_PWM_htim ,Motor->Motor_PWM_Channel , 100 + PWM ) ;	// 变小  ,差值变大
	}
}

// 使用M法测速公式,得到Motor的转速:n圈/s
void Motor_Speed_Update(Motor_Typedef *Motor)
{
	// !!!自己配置计时器!!!

	// 得到总脉冲数
	int Motor_CNT = Encoder_Get_CNT(&Motor->Motor_Encoder_htim);
	
	// 转速n = 总脉冲数/4倍频/单圈脉冲数(13)/减速比(28)/采样时间
	// Motor->Motor_RealSpeed = (float)Motor_CNT / 4 / 13 / 28 / Encoder_Gap_Time * 1000 ; ,直接算出来:4*13*28/1000=1.456
	Motor->RealSpeed = (float)Motor_CNT * 60 * 1000 / (4.0f * Motor->PPR * Motor->ReductionRatio) / Encoder_PID_Gap_Time  ;
}

// 电机PID计算与更新
void Motor_PID_Update(Motor_Typedef *Motor)
{
	// ******记得自己配置计时器*******
	
	// 更新一下真实速度
	Motor->PID_s.goalPoint = Motor->GoalSpeed ;
	
	// 得到设置速度输出
	PID_Update(&Motor->PID_s , Motor->RealSpeed) ;
	Motor->SetSpeed = Motor->PID_s.setPoint ;
	
}
