#ifndef __MYGPIO_H
#define __MYGPIO_H

#include "MySystem.h"

// GPIO结构体定义
typedef struct {
    GPIO_TypeDef *GPIO_Port;
    uint16_t GPIO_Pin;
} MyGPIO_Typedef;

// GPIO外部实例（const，在MySystem.c中定义）
extern const MyGPIO_Typedef MyGPIO_LED0;
extern const MyGPIO_Typedef MyGPIO_Key0;
extern const MyGPIO_Typedef MyGPIO_Key1;
extern const MyGPIO_Typedef MyGPIO_Key2;
extern const MyGPIO_Typedef MyGPIO_OLED_SCL;
extern const MyGPIO_Typedef MyGPIO_OLED_SDA;
extern const MyGPIO_Typedef MyGPIO_Motor_A_IN1;
extern const MyGPIO_Typedef MyGPIO_Motor_A_IN2;
extern const MyGPIO_Typedef MyGPIO_Motor_B_IN1;
extern const MyGPIO_Typedef MyGPIO_Motor_B_IN2;

// GPIO操作函数
void My_GPIO_WritePin(const MyGPIO_Typedef *gpio, int isHigh);
int My_GPIO_ReadPin(const MyGPIO_Typedef *gpio);

#endif // !__MYGPIO_H
