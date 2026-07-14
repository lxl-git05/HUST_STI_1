#include "Elec.h"

void Elec_Init(void)
{
    MyGPIO_WritePin(&MyGPIO_Elec, 0);  // 初始化为低电平
}

void Elec_ON(void)
{
    MyGPIO_WritePin(&MyGPIO_Elec, 1);  // 设置为高电平
}

void Elec_OFF(void)
{
    MyGPIO_WritePin(&MyGPIO_Elec, 0);  // 设置为低电平
}
