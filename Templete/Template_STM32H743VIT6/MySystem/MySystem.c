#include "MySystem.h"

// ====================================================================
// GPIO 实例定义 — 换芯片时只需修改这里的 port/pin
// ====================================================================
MyGPIO_Typedef MyGPIO_LED0        = { LED0_GPIO_Port,    LED0_Pin };
MyGPIO_Typedef MyGPIO_Key0        = { KEY0_GPIO_Port,    KEY0_Pin };
MyGPIO_Typedef MyGPIO_Key1        = { KEY1_GPIO_Port,    KEY1_Pin };
MyGPIO_Typedef MyGPIO_Key2        = { KEY2_GPIO_Port,    KEY2_Pin };
MyGPIO_Typedef MyGPIO_EC11_Key    = { EC11_Key_GPIO_Port, EC11_Key_Pin };
MyGPIO_Typedef MyGPIO_OLED_SCL    = { OLED_SCL_GPIO_Port, OLED_SCL_Pin };
MyGPIO_Typedef MyGPIO_OLED_SDA    = { OLED_SDA_GPIO_Port, OLED_SDA_Pin };
MyGPIO_Typedef MyGPIO_Motor_A_IN1 = { Motor_A_IN1_GPIO_Port, Motor_A_IN1_Pin };
MyGPIO_Typedef MyGPIO_Motor_A_IN2 = { Motor_A_IN2_GPIO_Port, Motor_A_IN2_Pin };
MyGPIO_Typedef MyGPIO_Motor_B_IN1 = { Motor_B_IN1_GPIO_Port, Motor_B_IN1_Pin };
MyGPIO_Typedef MyGPIO_Motor_B_IN2 = { Motor_B_IN2_GPIO_Port, Motor_B_IN2_Pin };

// ====================================================================
// PWM 实例定义 — 换芯片时只需修改 htim / Channel / Compare_Max / Compare_Min
// ====================================================================
// 舵机 PWM：50Hz（周期20ms），比较值 50~250 tick = 0.5~2.5ms 脉宽
MyPWM_Typedef MyPWM_Servo1      = { &htim1, TIM_CHANNEL_1, 250.0f, 50.0f };  // 舵机1 PE9
MyPWM_Typedef MyPWM_Servo2      = { &htim1, TIM_CHANNEL_2, 250.0f, 50.0f };  // 舵机2 PE11
MyPWM_Typedef MyPWM_Servo3      = { &htim1, TIM_CHANNEL_3, 250.0f, 50.0f };  // 舵机3 PE13
MyPWM_Typedef MyPWM_Servo4      = { &htim1, TIM_CHANNEL_4, 250.0f, 50.0f };  // 舵机4 PE14
MyPWM_Typedef MyPWM_Servo5      = { &htim8, TIM_CHANNEL_3, 250.0f, 50.0f };  // 舵机5 PC8
MyPWM_Typedef MyPWM_Servo6      = { &htim8, TIM_CHANNEL_4, 250.0f, 50.0f };  // 舵机6 PC9
MyPWM_Typedef MyPWM_Motor_A_IN1 = { &htim4, TIM_CHANNEL_1, 1000.0f, 0.0f };  // 电机A
MyPWM_Typedef MyPWM_Motor_B_IN1 = { &htim4, TIM_CHANNEL_2, 1000.0f, 0.0f };  // 电机B

// ====================================================================
// Encoder 实例定义 — 换芯片时只需修改 htim / time_Fre
// ====================================================================
MyEncoder_Typedef Motor_A_Encoder = { &htim2, 4, 0 };
MyEncoder_Typedef Motor_B_Encoder = { &htim3, 4, 0 };
