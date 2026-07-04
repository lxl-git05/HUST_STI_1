#ifndef __CON_MOTOR_H
#define __CON_MOTOR_H

#include "MySystem.h"
#include "Motor.h"

extern Motor_Typedef Motor_A ;
extern Motor_Typedef Motor_B ;

// ==================== 电机驱动 ====================
#define Motor_Hang_Up_Cnt    0
#define Motor_Hang_Mid_Cnt   1000
#define Motor_Hang_Down_Cnt  6900
#define Motor_Sigan_Next_Cnt 330

// 1. 电机初始化
void Con_Motor_Init(void) ;

// 2. 设置电机goal速度
void Motor_SetSpeed(Motor_Typedef *Motor, float speed) ;

// 3. 得到电机goal速度
int Motor_Get_GoalSpeed(Motor_Typedef *Motor) ;

// 4. 电机停止
void Motor_Stop(Motor_Typedef *Motor) ;

// 5. 电机急刹
void Motor_Brake(Motor_Typedef *Motor) ;

// 7. 电机速度更新与PID控制
void Motor_Speed_Update_Tick(uint32_t Gap_Time_ms) ;

// 8. 设置电机旋转角度
void Motor_SetAngle(Motor_Typedef *Motor , int Angle); 

// 9. 得到电机当前位置
float Motor_Get_Angle(Motor_Typedef *Motor) ;

// 10. 检查电机位置
bool Motor_Is_Angle(Motor_Typedef *Motor , int Angle , int Tolerance) ;

// 电机驱动
void Motor_Hang_Up(void)  ;
void Motor_Hang_Mid(void) ;
void Motor_Hang_Down(void);

bool IS_Motor_Hang_Up(void) ;
bool Is_Motor_Hang_Mid(void) ;
bool Is_Motor_Hanger_Down(void) ;

void Motor_Hua_Next(void) ;
void Motor_Hua_Back(void) ;
bool Motor_Hua_is_Ok(void);
 

#endif
