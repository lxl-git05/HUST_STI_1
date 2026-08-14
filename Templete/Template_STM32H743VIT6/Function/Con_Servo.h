#ifndef __CON_SERVO_H
#define __CON_SERVO_H

#include "Servo.h"

// 6 路舵机实例（Con_Servo.c 中定义）
extern Servo_Typedef Servo_1;
extern Servo_Typedef Servo_2;
extern Servo_Typedef Servo_3;
extern Servo_Typedef Servo_4;
extern Servo_Typedef Servo_5;
extern Servo_Typedef Servo_6;

// 统一初始化：6 路全部 0~180° 限幅、初始 90°
void Con_Servo_Init(void);

#endif
