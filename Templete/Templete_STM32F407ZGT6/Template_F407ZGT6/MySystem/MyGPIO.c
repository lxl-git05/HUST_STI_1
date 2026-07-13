#include "MySystem.h"

// 1. GPIO写
void MyGPIO_WritePin(MyGPIO_Typedef *gpio, int isHigh)
{
    HAL_GPIO_WritePin(gpio->GPIO_Port, gpio->GPIO_Pin,
                      isHigh ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

// 2. GPIO读
int MyGPIO_ReadPin(MyGPIO_Typedef *gpio)
{
    return HAL_GPIO_ReadPin(gpio->GPIO_Port, gpio->GPIO_Pin);
}
