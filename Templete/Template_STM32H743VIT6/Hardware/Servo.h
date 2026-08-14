#ifndef __SERVO_H
#define __SERVO_H

#include "MySystem.h"

typedef struct
{
    // 固定属性
    MyPWM_Typedef *Servo_PWM;   // PWM 句柄（MyPWM_Servo1~6）
    uint16_t pwm_min;           // 0° -> PWM 最小比较值（tick，50 = 0.5ms）
    uint16_t pwm_max;           // 180° -> PWM 最大比较值（tick，250 = 2.5ms）
    // 角度限幅
    uint16_t pos_min;           // 允许的最小角度
    uint16_t pos_max;           // 允许的最大角度
    // 运行状态
    int16_t current_pos;        // 当前角度
} Servo_Typedef;

// 1. 舵机初始化（PWM 启动 + 50Hz 校验 + 写初始角度）
void Servo_Init
(
    Servo_Typedef *Servo, MyPWM_Typedef *Servo_PWM,
    uint16_t pwm_min, uint16_t pwm_max,
    uint16_t pos_min, uint16_t pos_max, int16_t init_pos
);

// 2. 配置角度（双限幅：[pos_min, pos_max] + [0, 180]）
void Servo_SetAngle(Servo_Typedef *Servo, int16_t angle);

// 3. 获取当前角度
int Servo_Get_Angle(Servo_Typedef *Servo);

#endif
