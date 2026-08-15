#include "Con_Servo.h"
#include "Robot_Task.h"

// 6 路舵机实例定义
Servo_Typedef Servo_1;
Servo_Typedef Servo_2;
Servo_Typedef Servo_3;
Servo_Typedef Servo_4;
Servo_Typedef Servo_5;
Servo_Typedef Servo_6;

// 统一初始化：6 路全部 180° 舵机、0~180° 限幅、初始 90°
void Con_Servo_Init(void)
{
    //                                PWM 限幅  [pos限幅] 初始角度
    Servo_Init(&Servo_1, &MyPWM_Servo1, 50, 250, 0, 180, Th_ClawA_Open);
    Servo_Init(&Servo_2, &MyPWM_Servo2, 50, 250, 0, 180, Th_ClawB_Open);
    Servo_Init(&Servo_3, &MyPWM_Servo3, 50, 250, 0, 180, Th_Hanger1_Close);
    Servo_Init(&Servo_4, &MyPWM_Servo4, 50, 250, 0, 180, 90);
    Servo_Init(&Servo_5, &MyPWM_Servo5, 50, 250, 0, 180, 90);
    Servo_Init(&Servo_6, &MyPWM_Servo6, 50, 250, 0, 180, 90);
}
