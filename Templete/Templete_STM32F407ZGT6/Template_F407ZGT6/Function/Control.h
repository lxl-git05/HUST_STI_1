#ifndef __CONTROL_H
#define __CONTROL_H

#include "AllHeader.h"

// 任务注册地-测试任务
// 1. 任务1：等待3s，然后Exit
// TASK_WAIT_TIME: p[0]=等待时间(ms) p[1]=是否在任务中开启蜂鸣器
void Task_Wait_Time_Setup(float p[4]) ;
bool Task_Wait_Time_IsExit(float p[4]) ;
	
// 2. 任务2: 电机旋转一段时间之后停止,Exit
// TASK_Motor_Speed: p[0]=速度rpm, p[1]=持续时间ms
void Task_Motor_Speed_Setup(float p[4]) ;
bool Task_Motor_Speed_IsExit(float p[4]) ;

// 3. 任务3:电机旋转特定角度,旋转完成之后停止,Exit 
// TASK_Motor_Angle:p[0]为旋转角度 p[1]为容忍角度误差
void Task_Motor_Angle_Setup(float p[4]) ;
bool Task_Motor_Angle_IsExit(float p[4]) ;
void Task_Motor_Angle_Tick(float p[4]) ;

#endif

