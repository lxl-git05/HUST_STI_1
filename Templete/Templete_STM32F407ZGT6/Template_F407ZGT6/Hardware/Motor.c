#include "Motor.h"

// 1. 初始化
void Motor_Init
(								
		Motor_Typedef* pMotor, MyPWM_Typedef* pwm, MyEncoder_Typedef* encoder,
		MyGPIO_Typedef* in1,MyGPIO_Typedef* in2,
		Motor_Param_Typedef* param,int8_t pwm_dir,int8_t encoder_dir,
		Pid_Typedef  PID_s , Pid_Typedef PID_Angle , Pid_Typedef PID_Pos
)
{
    // 1. 初始化PWM
    pMotor->Motor_PWM = pwm;
	MyPWM_Init(pwm) ;
    
    // 2. 初始化编码器
    pMotor->Motor_Encoder = encoder;
	MyEncoder_Init(encoder) ;
    
    // 3. 初始化GPIO
    pMotor->Motor_GPIO_IN1 = in1;
    pMotor->Motor_GPIO_IN2 = in2;
    
    // 4. 初始化参数
    pMotor->Motor_Param = param;
    
    // 5. 初始化方向
    pMotor->PWM_Dir = pwm_dir;
    pMotor->Encoder_Dir = encoder_dir;
    
    // 6. 初始化PID
    pMotor->PID_s = PID_s ;
	pMotor->PID_Angle = PID_Angle ;
	pMotor->PID_Pos = PID_Pos ;
		
	// 7. 状态
	pMotor->State = MOTOR_STOP ;
}

// 2. 设置PWM值
void Motor_SetPWM(Motor_Typedef *Motor , int PWM)
{
	// 确认方向
	 PWM = PWM * Motor->PWM_Dir ;
	// 判断方向,设置速度
	if (PWM >= 0)
	{
		MyGPIO_WritePin(Motor->Motor_GPIO_IN2 , 0 ) ;	// 低电平,0
		MyPWM_SetCompare(Motor->Motor_PWM , PWM) ;		// 变大
	
	}
	else
	{
		MyGPIO_WritePin(Motor->Motor_GPIO_IN2 , 1 ) ;															// 高电平,1
		MyPWM_SetCompare(Motor->Motor_PWM , Motor->Motor_PWM->Compare_Max + PWM) ;			// 变小
	}
}

// 3. 得到一段周期内电机的速度,使用M法测速公式,得到Motor的转速:n圈/s
void Motor_Speed_Update(Motor_Typedef *Motor , uint32_t Gap_Time_ms)
{
	// 得到总脉冲数(含矫正方向)
	int Motor_CNT = MyEncoder_Get_CNT(Motor->Motor_Encoder) * Motor->Encoder_Dir;
	
	// 转速n = 总脉冲数/倍频(4)/单圈脉冲数(13)/减速比(28)/采样时间(Gap_Time_ms)
	Motor->PID_s.realPoint_Now = (float)Motor_CNT * 60 * 1000 / Gap_Time_ms /
		(Motor->Motor_Encoder->time_Fre * Motor->Motor_Param->PPR * Motor->Motor_Param->ReductionRatio)   ;
}

// 4. 得到当前电机旋转的角度
void Motor_Angle_Update(Motor_Typedef *Motor)
{
	// 得到角度 = 圈数 * 360
	float curr_Angle = (float)Motor->Motor_Encoder->total_cnt * 360.0f / 
		(Motor->Motor_Encoder->time_Fre * Motor->Motor_Param->PPR * Motor->Motor_Param->ReductionRatio)   ;
	
	// 记录当前角度
	Motor->PID_Angle.realPoint_Now = curr_Angle ;
}

// 5. 得到当前电机走过的位移(cm)
void Motor_Pos_Update(Motor_Typedef *Motor)
{
	// 位移(cm) = 总脉冲数 * 轮子周长 / (倍频 * PPR * 减速比)
	float curr_Pos = (float)Motor->Motor_Encoder->total_cnt * Motor->Motor_Param->Wheel_Cm /
		(Motor->Motor_Encoder->time_Fre * Motor->Motor_Param->PPR * Motor->Motor_Param->ReductionRatio)   ;

	// 记录当前位置
	Motor->PID_Pos.realPoint_Now = curr_Pos ;
}
