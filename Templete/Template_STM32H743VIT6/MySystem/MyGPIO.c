#include "MySystem.h"

// 1. GPIO写
void My_GPIO_WritePin(const MyGPIO_Typedef *gpio, int isHigh)
{
    HAL_GPIO_WritePin(gpio->GPIO_Port, gpio->GPIO_Pin,
                      isHigh ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

// 2. GPIO读
int My_GPIO_ReadPin(const MyGPIO_Typedef *gpio)
{
    return HAL_GPIO_ReadPin(gpio->GPIO_Port, gpio->GPIO_Pin);
}
