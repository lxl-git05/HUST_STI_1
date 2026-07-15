#include "MySystem.h"

// ====================================================================
// GPIO 实例定义 — 换芯片时只需修改这里的 port/pin
// ====================================================================
MyGPIO_Typedef MyGPIO_LED0        = { LED0_GPIO_Port,    LED0_Pin };
MyGPIO_Typedef MyGPIO_Key0        = { KEY0_GPIO_Port,    KEY0_Pin };
MyGPIO_Typedef MyGPIO_Key1        = { KEY1_GPIO_Port,    KEY1_Pin };
MyGPIO_Typedef MyGPIO_Key2        = { KEY2_GPIO_Port,    KEY2_Pin };
MyGPIO_Typedef MyGPIO_Key3        = { KEY3_GPIO_Port,    KEY3_Pin };
MyGPIO_Typedef MyGPIO_OLED_SCL    = { OLED_SCL_GPIO_Port, OLED_SCL_Pin };
MyGPIO_Typedef MyGPIO_OLED_SDA    = { OLED_SDA_GPIO_Port, OLED_SDA_Pin };
MyGPIO_Typedef MyGPIO_Motor_A_IN1 = { Motor_A_IN1_GPIO_Port, Motor_A_IN1_Pin };
MyGPIO_Typedef MyGPIO_Motor_A_IN2 = { Motor_A_IN2_GPIO_Port, Motor_A_IN2_Pin };
MyGPIO_Typedef MyGPIO_Motor_B_IN1 = { Motor_B_IN1_GPIO_Port, Motor_B_IN1_Pin };
MyGPIO_Typedef MyGPIO_Motor_B_IN2 = { Motor_B_IN2_GPIO_Port, Motor_B_IN2_Pin };
MyGPIO_Typedef MyGPIO_RGB_R       = { RGB_R_GPIO_Port,     RGB_R_Pin };
MyGPIO_Typedef MyGPIO_RGB_G       = { RGB_G_GPIO_Port,     RGB_G_Pin };
MyGPIO_Typedef MyGPIO_RGB_B       = { RGB_Y_GPIO_Port,     RGB_Y_Pin };

MyGPIO_Typedef MyGPIO_Buzzer      = { Buzzer_GPIO_Port, Buzzer_Pin };
MyGPIO_Typedef MyGPIO_EC11_Key    = { EC11_Key_GPIO_Port, EC11_Key_Pin };

MyGPIO_Typedef MyGPIO_Stepper_En   = {Stepper_En_GPIO_Port  , Stepper_En_Pin } ;
MyGPIO_Typedef MyGPIO_Stepper2_En  = {Stepper_En2_GPIO_Port , Stepper_En2_Pin} ;
MyGPIO_Typedef MyGPIO_Stepper_Dir  = {Stepper_Dir_GPIO_Port  , Stepper_Dir_Pin } ;
MyGPIO_Typedef MyGPIO_Stepper2_Dir = {Stepper_Dir2_GPIO_Port , Stepper_Dir2_Pin} ;

MyGPIO_Typedef MyGPIO_Elec         = {Elec_GPIO_Port , Elec_Pin} ;

MyGPIO_Typedef MyGPIO_Y8_Addr0     = {Y8_Addr_0_GPIO_Port , Y8_Addr_0_Pin} ;
MyGPIO_Typedef MyGPIO_Y8_Addr1     = {Y8_Addr_1_GPIO_Port , Y8_Addr_1_Pin} ;
MyGPIO_Typedef MyGPIO_Y8_Addr2     = {Y8_Addr_2_GPIO_Port , Y8_Addr_2_Pin} ;

// ====================================================================
// PWM 实例定义 — 换芯片时只需修改 htim / Channel / Compare_Max / Compare_Min
// ====================================================================
// 舵机暂时注销
//MyPWM_Typedef MyPWM_Servo1      = { &htim1, TIM_CHANNEL_1, 2000.0f, 500.0f };  // 舵机1
//MyPWM_Typedef MyPWM_Servo2      = { &htim1, TIM_CHANNEL_2, 2000.0f, 500.0f };  // 舵机2
//MyPWM_Typedef MyPWM_Servo3      = { &htim1, TIM_CHANNEL_3, 2000.0f, 500.0f };  // 舵机3
//MyPWM_Typedef MyPWM_Servo4      = { &htim1, TIM_CHANNEL_4, 2000.0f, 500.0f };  // 舵机4
MyPWM_Typedef MyPWM_Motor_A_IN1 = { &htim4, TIM_CHANNEL_3, 1000.0f, 0.0f   };  // 电机A
MyPWM_Typedef MyPWM_Motor_B_IN1 = { &htim4, TIM_CHANNEL_4, 1000.0f, 0.0f   };  // 电机B

MyPWM_Typedef MyPWM_Stepper1 		= { &htim9 ,TIM_CHANNEL_1, 1000.0f, 0.0f   };  // 步进电机1
MyPWM_Typedef MyPWM_Stepper2 		= { &htim12,TIM_CHANNEL_1, 1000.0f, 0.0f   };  // 步进电机2

// ====================================================================
// Encoder 实例定义 — 换芯片时只需修改 htim / time_Fre
// ====================================================================
MyEncoder_Typedef Motor_A_Encoder = { &htim2, 4, 0 };
MyEncoder_Typedef Motor_B_Encoder = { &htim3, 4, 0 };
