#include "Encoder.h"

// ��������ʼ��
void Encoder_Init(TIM_HandleTypeDef *htimx)
{
	HAL_TIM_Encoder_Start(htimx, TIM_CHANNEL_ALL);
}

// �õ���������������
int Encoder_Get_CNT(TIM_HandleTypeDef *htimx)
{
	// �õ�һ�β���ʱ���������
	int cnt = __HAL_TIM_GET_COUNTER(htimx);

	// �õ�������,>0Ϊ��,<0Ϊ��
	if(cnt > 0x7fff)
	{
		cnt = cnt - 0xffff;	// ��ת,���������ת,û�仯
	}

	// ����
	__HAL_TIM_SET_COUNTER(htimx, 0);
	
	// ���ر������ڵļ���ֵ
	return cnt;
}

