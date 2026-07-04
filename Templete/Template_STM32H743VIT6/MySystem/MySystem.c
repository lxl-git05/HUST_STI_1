#include "MySystem.h"

// ====================================================================
// GPIO 实例定义 — 换芯片时只需修改这里的 port/pin
// ====================================================================
const MyGPIO_Typedef MyGPIO_LED0        = { LED0_GPIO_Port,    LED0_Pin };
const MyGPIO_Typedef MyGPIO_Key0        = { KEY0_GPIO_Port,    KEY0_Pin };
const MyGPIO_Typedef MyGPIO_Key1        = { KEY1_GPIO_Port,    KEY1_Pin };
const MyGPIO_Typedef MyGPIO_Key2        = { KEY2_GPIO_Port,    KEY2_Pin };
const MyGPIO_Typedef MyGPIO_OLED_SCL    = { OLED_SCL_GPIO_Port, OLED_SCL_Pin };
const MyGPIO_Typedef MyGPIO_OLED_SDA    = { OLED_SDA_GPIO_Port, OLED_SDA_Pin };
const MyGPIO_Typedef MyGPIO_Motor_A_IN1 = { Motor_A_IN1_GPIO_Port, Motor_A_IN1_Pin };
const MyGPIO_Typedef MyGPIO_Motor_A_IN2 = { Motor_A_IN2_GPIO_Port, Motor_A_IN2_Pin };
const MyGPIO_Typedef MyGPIO_Motor_B_IN1 = { Motor_B_IN1_GPIO_Port, Motor_B_IN1_Pin };
const MyGPIO_Typedef MyGPIO_Motor_B_IN2 = { Motor_B_IN2_GPIO_Port, Motor_B_IN2_Pin };
