#ifndef __CONTROL_H
#define __CONTROL_H

#include "AllHeader.h"

// ==================== 全局共享任务表 ====================
// ★ 所有 Mode 统一引用此表，Con_Task_Init(Control_TaskTable, TASK_COUNT)
#include "Con_Task.h"
extern Task_Descriptor_Typedef Control_TaskTable[TASK_COUNT];

// =========================== 通用任务 ===========================

// 1. 任务：等待xms，然后Exit（伴随蜂鸣器）
// TASK_WAIT_TIME: p[0]=等待时间(ms)
void Task_Wait_Time_Setup(float p[4]) ;
bool Task_Wait_Time_IsExit(float p[4]) ;

// =========================== F407 原有任务 ==========================
// 2. 任务：电机旋转一段时间后停止
// TASK_Motor_Speed: p[0]=速度rpm, p[1]=持续时间ms
void Task_Motor_Speed_Setup(float p[4]) ;
bool Task_Motor_Speed_IsExit(float p[4]) ;

// 3. 任务：电机旋转特定角度
// TASK_Motor_Angle: p[0]=旋转角度, p[1]=容忍角度误差
void Task_Motor_Angle_Setup(float p[4]) ;
bool Task_Motor_Angle_IsExit(float p[4]) ;

// =========================== MSPM0 新增小车任务 ===========================

// 4. 任务：电机A旋转特定角度，旋转完成之后停止，Exit
// TASK_MOTOR_A_ANGLE: p[0]=旋转角度°, p[1]=容差°
void Task_Motor_A_Angle_Setup(float p[4]) ;
bool Task_Motor_A_Angle_IsExit(float p[4]) ;

// 5. 任务：电机B旋转特定角度，旋转完成之后停止，Exit
// TASK_MOTOR_B_ANGLE: p[0]=旋转角度°, p[1]=容差°
void Task_Motor_B_Angle_Setup(float p[4]) ;
bool Task_Motor_B_Angle_IsExit(float p[4]) ;

// =========================== 晾衣机器人任务 ===========================

// 舵机角色宏映射（换编号方案只改这里 + Control.c 的 s_ServoMap）
#define SERVO_CLAW_A   (&Servo_1)       // 夹爪A
#define SERVO_CLAW_B   (&Servo_2)       // 夹爪B
#define SERVO_HANGER_1 (&Servo_3)       // 衣架1
#define SERVO_HANGER_2 (&Servo_4)       // 衣架2

// 角色索引（TASK_SERVO_SET 的 p[0]，与 Control.c 的 s_ServoMap 顺序一致）
#define ROBOT_SERVO_CLAW_A   0
#define ROBOT_SERVO_CLAW_B   1
#define ROBOT_SERVO_HANGER_1 2
#define ROBOT_SERVO_HANGER_2 3

// 6. 任务：双电机定位（无超时，堵转需外部急停中断）
// TASK_MOTOR_TO: p[0]=电机(0=A/1=B), p[1]=目标角度°, p[2]=容差°
void Task_Motor_To_Setup(float p[4]) ;
bool Task_Motor_To_IsExit(float p[4]) ;

// 7. 任务：舵机设置，保持 p[2]ms 后 Exit（p[2]=0 立即完成）
// TASK_SERVO_SET: p[0]=角色索引, p[1]=角度, p[2]=保持ms
void Task_Servo_Set_Setup(float p[4]) ;
bool Task_Servo_Set_IsExit(float p[4]) ;

// 8. 任务：双夹爪同步设置，两舵机同时动作，保持 p[2]ms 后 Exit
// TASK_CLAW_SET: p[0]=夹爪A角度, p[1]=夹爪B角度, p[2]=保持ms
void Task_Claw_Set_Setup(float p[4]) ;
bool Task_Claw_Set_IsExit(float p[4]) ;

// 9. 任务：串口发 @Car_Back$# 通知小车倒车（收衣服序列最后一步）
// TASK_SERIAL_CAR_BACK: 无参数，Setup 即发送，立即 Exit
void Task_Serial_CarBack_Setup(float p[4]) ;
bool Task_Serial_CarBack_IsExit(float p[4]) ;

#endif
