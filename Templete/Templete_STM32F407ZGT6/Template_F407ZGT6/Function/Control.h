#ifndef __CONTROL_H
#define __CONTROL_H

#include "AllHeader.h"

// 1. 前往任务地点(x,y)
void Task_Tar_XY_Setup(float p[4]) ;
void Task_Tar_XY_Run(float p[4]) ;
bool Task_Tar_XY_IsExit(float p[4]) ;
void Task_Tar_XY_Tick(float p[4]) ;
// 2. 向下
void Task_Down_Setup(float p[4]); 
void Task_Down_Run(float p[4]) ;
bool Task_Down_IsExit(float p[4]) ;
void Task_Down_Tick(float p[4]) ;
// 3. 回到原点
void Task_Back_Setup(float p[4]) ;
void Task_Back_Run(float p[4]) ;
bool Task_Back_IsExit(float p[4]) ;
void Task_Back_Tick(float p[4]) ;
// 4. 取/放棋子
// Task_Elec: p[0]=等待时间(ms)
void Task_Elec_Setup(float p[4]) ;
bool Task_Elec_IsExit(float p[4]) ;
// 5. 上升
// Task_Up: p[0]为上升角度 p[1]为容忍角度误差
void Task_Up_Setup(float p[4]) ;
void Task_Up_Run(float p[4]) ;
bool Task_Up_IsExit(float p[4]) ;
void Task_Up_Tick(float p[4]) ;

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

