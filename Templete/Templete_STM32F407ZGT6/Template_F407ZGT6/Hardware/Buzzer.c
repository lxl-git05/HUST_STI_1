#include "Buzzer.h"

#define BUZZER_ON  MyGPIO_WritePin(&MyGPIO_Buzzer, 0) ;
#define BUZZER_OFF MyGPIO_WritePin(&MyGPIO_Buzzer, 1) ;

// Buzzer初始化:需要为低电平
void Buzzer_Init(void)
{
    BUZZER_OFF
}

// Buzzer响
void Buzzer_ON(void)
{
	BUZZER_ON
}

// Buzzer安静
void Buzzer_OFF(void)
{
	BUZZER_OFF
}

