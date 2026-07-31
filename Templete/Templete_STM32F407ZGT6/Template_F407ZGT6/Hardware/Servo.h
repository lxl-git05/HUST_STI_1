#ifndef __SERVO_H
#define __SERVO_H

#include "MySystem.h"

// 舵机角度范围
#define SERVO_ANGLE_MIN  0.0f
#define SERVO_ANGLE_MAX  180.0f

// 初始化舵机
void Servo_Init(void);

// 设置舵机角度 (0~180°)
void Servo_SetAngle(float angle);

#endif
