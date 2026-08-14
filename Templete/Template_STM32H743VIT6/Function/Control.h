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

#endif
