#ifndef __CON_SERVO_H
#define __CON_SERVO_H

#include "Servo.h"

extern Servo_Typedef Servo_1 ;
extern Servo_Typedef Servo_2 ;
extern Servo_Typedef Servo_3 ;
extern Servo_Typedef Servo_4 ;

void Con_Servo_Init(void) ;
void Con_Servo_GoalAngle_Tick(void) ;

// 夹爪张开
void Servo_Claw_Open(void) ;
// 夹爪闭合
void Servo_Claw_Close(void) ;
// 衣架 闭合 95 °
int Servo_Hanger_Close(void) ;
// 衣架 张开 0 °
int Servo_Hanger_Open(void) ;
// 衣架2 闭合
int Servo_Hanger2_Close(void) ;
// 衣架2 张开
int Servo_Hanger2_Open(void) ;

#endif
