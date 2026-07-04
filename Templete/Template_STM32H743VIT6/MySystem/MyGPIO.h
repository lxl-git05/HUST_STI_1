#ifndef __MYGPIO_H
#define __MYGPIO_H

#include "MySystem.h"

// GPIO结构体定义
typedef struct 
{
    GPIO_TypeDef *GPIO_Port;
    uint16_t GPIO_Pin;
} MyGPIO_Typedef;

// GPIO外部声明
extern MyGPIO_Typedef MyGPIO_LED0          ;

extern MyGPIO_Typedef MyGPIO_Key0          ;
extern MyGPIO_Typedef MyGPIO_Key1          ;
extern MyGPIO_Typedef MyGPIO_Key2          ;

extern MyGPIO_Typedef MyGPIO_OLED_SCL      ;
extern MyGPIO_Typedef MyGPIO_OLED_SDA      ;

extern MyGPIO_Typedef MyGPIO_Motor_A_IN1   ;
extern MyGPIO_Typedef MyGPIO_Motor_A_IN2   ;
extern MyGPIO_Typedef MyGPIO_Motor_B_IN1   ;
extern MyGPIO_Typedef MyGPIO_Motor_B_IN2   ;

// GPIO函数声明
void My_GPIO_WritePin(MyGPIO_Typedef* MyGPIO , int isHigh) ;    // 1. GPIO写
int My_GPIO_ReadPin(MyGPIO_Typedef* MyGPIO) ;                   // 2. GPIO读

#endif // !__MYGPIO_H
