#ifndef __MYGPIO_H
#define __MYGPIO_H

#include "MySystem.h"

// GPIO结构体定义
typedef struct {
    GPIO_TypeDef *GPIO_Port;
    uint16_t GPIO_Pin;
} MyGPIO_Typedef;

// GPIO外部实例（在MySystem.c中定义）
extern MyGPIO_Typedef MyGPIO_LED0;
extern MyGPIO_Typedef MyGPIO_Key0;
extern MyGPIO_Typedef MyGPIO_Key1;	// 注销，因为Key1也是板载的，导致总是按错位置，索性取消板载位置,Key码顺序移动一位
extern MyGPIO_Typedef MyGPIO_Key2;
extern MyGPIO_Typedef MyGPIO_OLED_SCL;
extern MyGPIO_Typedef MyGPIO_OLED_SDA;
extern MyGPIO_Typedef MyGPIO_Motor_A_IN1;
extern MyGPIO_Typedef MyGPIO_Motor_A_IN2;
extern MyGPIO_Typedef MyGPIO_Motor_B_IN1;
extern MyGPIO_Typedef MyGPIO_Motor_B_IN2;
extern MyGPIO_Typedef MyGPIO_RGB_R;
extern MyGPIO_Typedef MyGPIO_RGB_G;
extern MyGPIO_Typedef MyGPIO_RGB_B;
extern MyGPIO_Typedef MyGPIO_Key3;	

extern MyGPIO_Typedef MyGPIO_Buzzer ;
extern MyGPIO_Typedef MyGPIO_EC11_Key ;

extern MyGPIO_Typedef MyGPIO_Stepper_En   ;
extern MyGPIO_Typedef MyGPIO_Stepper2_En  ;
extern MyGPIO_Typedef MyGPIO_Stepper_Dir  ;
extern MyGPIO_Typedef MyGPIO_Stepper2_Dir ;

extern MyGPIO_Typedef MyGPIO_Elec  ;

extern MyGPIO_Typedef MyGPIO_Y8_Addr0 ;
extern MyGPIO_Typedef MyGPIO_Y8_Addr1 ;
extern MyGPIO_Typedef MyGPIO_Y8_Addr2 ;

// GPIO操作函数
void MyGPIO_WritePin(MyGPIO_Typedef *gpio, int isHigh);
int MyGPIO_ReadPin(MyGPIO_Typedef *gpio);

#endif // !__MYGPIO_H
