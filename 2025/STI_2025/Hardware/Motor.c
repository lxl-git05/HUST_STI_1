#include "Motor.h"

Motor_Typedef Motor_A ;
Motor_Typedef Motor_B ;

// Motor_A��ʼ��
void Motor_A_Init(void)
{
	// �趨Motor_A��ز���
	Motor_A.Motor_Encoder_htim = Motor_A_Encoder_htim		;	// ������TIM_htim
	
	Motor_A.Motor_PWM_htim   	= Motor_A_PWM_Htim		  ;		// PWM_TIM
	Motor_A.Motor_PWM_Channel = Motor_A_PWM_Channel  ;
	
	Motor_A.IN1_Port			= Motor_A_IN1_GPIO_Port	;
	Motor_A.IN1_Pin			  = Motor_A_IN1_Pin ;
	
	Motor_A.IN2_Port			= Motor_A_IN2_GPIO_Port	;
	Motor_A.IN2_Pin			  = Motor_A_IN2_Pin ;
	
	Motor_A.PPR = 13.0f ;
	Motor_A.ReductionRatio = 28.0f ;
	
	Motor_A.DIR = DIR_P ;		// �����ж�
	
	// PWM��ʼ��
	HAL_TIM_PWM_Start(&Motor_A.Motor_PWM_htim , Motor_A.Motor_PWM_Channel) ;	
	
	// ��������ʼ��
	Encoder_Init(&Motor_A_Encoder_htim) ;
	
	// PID��ʼ��
	PID_Init(&Motor_A.PID_s , 0.72f , 0.09f , 0.1f , 100 , -100 , 10000 ) ;
	
}

// Motor_B��ʼ��
void Motor_B_Init(void)
{
	// �趨Motor_B��ز���
	Motor_B.Motor_Encoder_htim = Motor_B_Encoder_htim		;	// ������TIM_htim
	
	Motor_B.Motor_PWM_htim   	= Motor_B_PWM_Htim		  ;		// PWM_TIM
	Motor_B.Motor_PWM_Channel = Motor_B_PWM_Channel  ;
	
	Motor_B.IN1_Port			= Motor_B_IN1_GPIO_Port	;
	Motor_B.IN1_Pin			  = Motor_B_IN1_Pin ;
	
	Motor_B.IN2_Port			= Motor_B_IN2_GPIO_Port	;
	Motor_B.IN2_Pin			  = Motor_B_IN2_Pin ;
	
	Motor_B.PPR = 13.0f ;
	Motor_B.ReductionRatio = 28.0f ;
	
	Motor_B.DIR = DIR_P ;	// �����ж�
	
	// PWM��ʼ��
	HAL_TIM_PWM_Start(&Motor_B.Motor_PWM_htim , Motor_B.Motor_PWM_Channel) ;
	
	// ��������ʼ��
	Encoder_Init(&Motor_B_Encoder_htim) ;
	
	// PID��ʼ��
	PID_Init(&Motor_B.PID_s , 0.72f , 0.09f , 0.1f , 100 , -100 , 10000 ) ;
}
// ������ִ�е��߼�,����Ŀ���ٶ�
void Motor_SetGoalSpeed(Motor_Typedef *Motor , int speed)
{
	// ������ֵ
	if (speed >= Motor_MAX_Speed)
	{
		speed = Motor_MAX_Speed ;
	}
	else if (speed <= -Motor_MAX_Speed)
	{
		speed = -Motor_MAX_Speed ;
	}
	// �жϷ���,����Ŀ��ֵ
	Motor->GoalSpeed = speed * Motor->DIR;
}

// ����PWM,Ļ��ִ�е��ٶ��߼�(setPoint)
void Motor_SetPWM(Motor_Typedef *Motor , int PWM)
{
	// ������ֵ
	if (PWM >= Motor_MAX_Speed)
	{
		PWM = Motor_MAX_Speed ;
	}
	else if (PWM <= -Motor_MAX_Speed)
	{
		PWM = -Motor_MAX_Speed ;
	}
	// �жϷ���,�����ٶ�
	if (PWM >= 0)
	{
		HAL_GPIO_WritePin(Motor->IN2_Port , Motor->IN2_Pin , GPIO_PIN_RESET ) ;			// �͵�ƽ,0
		__HAL_TIM_SET_COMPARE(&Motor->Motor_PWM_htim ,Motor->Motor_PWM_Channel , PWM ) ;	// ���	 ,��ֵ���
	}
	else
	{
		HAL_GPIO_WritePin(Motor->IN2_Port , Motor->IN2_Pin , GPIO_PIN_SET ) ;				// �ߵ�ƽ,100
		__HAL_TIM_SET_COMPARE(&Motor->Motor_PWM_htim ,Motor->Motor_PWM_Channel , 100 + PWM ) ;	// ��С  ,��ֵ���
	}
}

// ʹ��M�����ٹ�ʽ,�õ�Motor��ת��:nȦ/s
void Motor_Speed_Update(Motor_Typedef *Motor)
{
	// !!!�Լ����ü�ʱ��!!!

	// �õ���������
	int Motor_CNT = Encoder_Get_CNT(&Motor->Motor_Encoder_htim);
	
	// ת��n = ��������/4��Ƶ/��Ȧ������(13)/���ٱ�(28)/����ʱ��
	// Motor->Motor_RealSpeed = (float)Motor_CNT / 4 / 13 / 28 / Encoder_Gap_Time * 1000 ; ,ֱ�������:4*13*28/1000=1.456
	Motor->RealSpeed = (float)Motor_CNT * 60 * 1000 / (4.0f * Motor->PPR * Motor->ReductionRatio) / Encoder_PID_Gap_Time  ;
}

// ���PID���������
void Motor_PID_Update(Motor_Typedef *Motor)
{
	// ******�ǵ��Լ����ü�ʱ��*******
	
	// ����һ����ʵ�ٶ�
	Motor->PID_s.goalPoint = Motor->GoalSpeed ;
	
	// �õ������ٶ����
	PID_Update(&Motor->PID_s , Motor->RealSpeed) ;
	Motor->SetSpeed = Motor->PID_s.setPoint ;
	
}
