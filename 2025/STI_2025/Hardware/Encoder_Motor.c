#include "Encoder_Motor.h"

//int Motor_Cnt ;

//// �����ʼ��
//void Motor_Init(void)
//{
//	// �����������ӿ�
//	HAL_TIM_Encoder_Start(&Motor_htim, TIM_CHANNEL_ALL);
//}

//// ��ʼ���Ա���������
//void Motor_Encoder_Tick(void)
//{
//	static int Motor_Encoder_count = 0 ;
//	Motor_Encoder_count ++ ;
//	if ( Motor_Encoder_count == 1000)
//	{
//		Motor_Cnt = __HAL_TIM_GET_COUNTER(&Motor_htim) ;	// �õ�һ���������ڼ�CNT�ĸ���(��Ҫ����˷���CNTˢ��,���ǳ���)
//		
//		__HAL_TIM_SET_COUNTER(&Motor_htim , 0) ;	// ����CNT,�����µĲ�������
//		
//		Motor_Encoder_count = 1 ;
//		
//	}
//	
//}
