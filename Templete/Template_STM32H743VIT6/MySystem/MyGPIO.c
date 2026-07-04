#include "MySystem.h"

// 0. 全局GPIO声明(需要进行操作的才声明)
MyGPIO_Typedef MyGPIO_LED0          = {LED0_GPIO_Port , LED0_Pin};

MyGPIO_Typedef MyGPIO_Key0          = {KEY0_GPIO_Port , KEY0_Pin};
MyGPIO_Typedef MyGPIO_Key1          = {KEY1_GPIO_Port , KEY1_Pin};
MyGPIO_Typedef MyGPIO_Key2          = {KEY2_GPIO_Port , KEY2_Pin};

MyGPIO_Typedef MyGPIO_OLED_SCL      = {OLED_SCL_GPIO_Port , OLED_SCL_Pin};
MyGPIO_Typedef MyGPIO_OLED_SDA      = {OLED_SDA_GPIO_Port , OLED_SDA_Pin};

MyGPIO_Typedef MyGPIO_Motor_A_IN1   = {Motor_A_IN1_GPIO_Port , Motor_A_IN1_Pin};
MyGPIO_Typedef MyGPIO_Motor_A_IN2   = {Motor_A_IN2_GPIO_Port , Motor_A_IN2_Pin};
MyGPIO_Typedef MyGPIO_Motor_B_IN1   = {Motor_B_IN1_GPIO_Port , Motor_B_IN1_Pin};
MyGPIO_Typedef MyGPIO_Motor_B_IN2   = {Motor_B_IN2_GPIO_Port , Motor_B_IN2_Pin};

// 1. GPIO写
void My_GPIO_WritePin(MyGPIO_Typedef* MyGPIO , int isHigh)
{
	if (isHigh != 0)
    {
        HAL_GPIO_WritePin(MyGPIO->GPIO_Port , MyGPIO->GPIO_Pin , GPIO_PIN_SET) ;
    }
    else
    {
        HAL_GPIO_WritePin(MyGPIO->GPIO_Port , MyGPIO->GPIO_Pin , GPIO_PIN_RESET) ;
    }
}

// 2. GPIO读
int My_GPIO_ReadPin(MyGPIO_Typedef* MyGPIO)
{
	return HAL_GPIO_ReadPin(MyGPIO->GPIO_Port , MyGPIO->GPIO_Pin) ;
}
