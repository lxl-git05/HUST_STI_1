#ifndef __MOTOR_H
#define __MOTOR_H

#include "main.h"
#include "tim.h"

#include "Encoder.h"
#include "MyPID.h"

// ���PWM
#define Motor_A_PWM_Htim htim1			// PWM��ʱ��λ��
#define Motor_B_PWM_Htim htim1			// PWM��ʱ��λ��

#define Motor_A_PWM_Channel TIM_CHANNEL_1	// PWMͨ��
#define Motor_B_PWM_Channel TIM_CHANNEL_4	// PWMͨ��

// ���Encoder
#define Motor_A_Encoder_htim htim2	// Encoder��ʱ��λ��
#define Motor_B_Encoder_htim htim3	// Encoder��ʱ��λ��

#define Encoder_PID_Gap_Time 20			// ���������ٺ�PID����,����ʱ���Ϊ20ms/��

// ����ٶ�
#define Motor_MAX_Speed 370					// ���ת��(goal�����ֵ)

// ���������
#define DIR_P ( 1)
#define DIR_N (-1)

// *********����*********

// �������
typedef struct
{
	TIM_HandleTypeDef Motor_Encoder_htim ;				// ����ı�����htim
	
	TIM_HandleTypeDef Motor_PWM_htim ;						// �����PWM_TIMx
	uint32_t Motor_PWM_Channel ;			// �����PWMͨ��
	
	GPIO_TypeDef *IN1_Port ; 						// ���IN1��������,����ΪPWM
	uint16_t IN1_Pin  ;									// ���IN1�����ź�
	
	GPIO_TypeDef *IN2_Port ; 						// ���IN2����������,����Ϊ��ͨ����
	uint16_t IN2_Pin  ;									// ���IN2�����ź�
	
	float PPR;                          // ����������
	float ReductionRatio;               // ���ٱ�
	int8_t DIR;                         // ������
	
	int GoalSpeed	;											// ���Ŀ���ٶ�
	int RealSpeed	;											// ���ʵ���ٶ�
	int SetSpeed	;											// ����趨�ٶ�
	
	Pid_Typedef PID_s ;									// PID����
	
}Motor_Typedef ;

// ��ʼ������:����PWM,Encoder,PID��ʼ��
void Motor_A_Init(void) ;																		// Motor_A��ʼ��
// Motor_B��ʼ��
void Motor_B_Init(void)	;																		// Motor_B��ʼ��

// �ٶȺ���
void Motor_SetGoalSpeed(Motor_Typedef *Motor , int speed) ;	// ����Ŀ���ٶ�,������ִ�е��߼�

void Motor_Speed_Update(Motor_Typedef *Motor) ;							// ����,ʹ��M����ʽ,�õ�Motor��ת��:nȦ/s !������ü�ʱ��!

void Motor_SetPWM(Motor_Typedef *Motor , int PWM) ;					// ����PWM,Ļ��ִ�е��ٶ��߼�(setPoint)

// ���PID���������,!�ǵ����ü�ʱ��!
void Motor_PID_Update(Motor_Typedef *Motor) ;

#endif

