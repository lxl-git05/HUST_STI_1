#ifndef __CONTROL_H
#define __CONTROL_H

#include "AllHeader.h"

// ==================== 全局共享任务表 ====================
// ★ 所有 Con_Mode 统一引用此表，Con_Task_Init(Control_TaskTable, TASK_COUNT)
#include "Con_Task.h"
extern Task_Descriptor_Typedef Control_TaskTable[TASK_COUNT];

// =========================== 业务逻辑中所有需要脱机调试的变量声明 ===========================
// 1. Task_Tar_XY
extern float Tar_XY_Tol_Distance ;
extern float Tar_XY_Tol_Speed ;
extern float Tar_XY_Ratio_X ;
extern float Tar_XY_Ratio_Y ;

// 2. Task_Down
extern float Down_Tar_Angle ;
extern float Down_Tol_Angle;

// 3. Task_Back
extern float Back_Tar_Angle ;
extern float Back_Speed_MAX ;
extern float Back_Acc			 ;
extern float Back_Tol_Angle ;

// 4. Task_Elec
extern float Elec_Wait 		 ;

// 5. Task_Up
extern float Up_Tar_Angle 	 ;
extern float Up_Tol_Angle 	 ;

// =========================== F407 原有任务 ===========================

// 1. 移动xy：前进到指定点(x,y)
// Task_Tar_XY: p[0]=x目标像素, p[1]=y目标像素
void Task_Tar_XY_Setup(float p[4]) ;
void Task_Tar_XY_Run(float p[4]) ;
bool Task_Tar_XY_IsExit(float p[4]) ;
void Task_Tar_XY_Tick(float p[4]) ;

// 2. 下降
// Task_Down: p[0]=下降角度, p[1]=容忍角度误差
void Task_Down_Setup(float p[4]);
void Task_Down_Run(float p[4]) ;
bool Task_Down_IsExit(float p[4]) ;
void Task_Down_Tick(float p[4]) ;

// 3. 回到原点
// Task_Back: p[0]=目标角度 p[1]=最大速度 p[2]=加速度 p[3]=容忍角度误差
void Task_Back_Setup(float p[4]) ;
void Task_Back_Run(float p[4]) ;
bool Task_Back_IsExit(float p[4]) ;
void Task_Back_Tick(float p[4]) ;

// 4. 取/放棋子
// Task_Elec: p[0]=等待时间(ms)
void Task_Elec_Setup(float p[4]) ;
bool Task_Elec_IsExit(float p[4]) ;

// 5. 上升
// Task_Up: p[0]=上升角度 p[1]=容忍角度误差
void Task_Up_Setup(float p[4]) ;
void Task_Up_Run(float p[4]) ;
bool Task_Up_IsExit(float p[4]) ;
void Task_Up_Tick(float p[4]) ;

// 6. 任务：电机旋转一段时间后停止
// TASK_Motor_Speed: p[0]=速度rpm, p[1]=持续时间ms
void Task_Motor_Speed_Setup(float p[4]) ;
bool Task_Motor_Speed_IsExit(float p[4]) ;

// 7. 任务：电机旋转特定角度
// TASK_Motor_Angle: p[0]=旋转角度, p[1]=容忍角度误差
void Task_Motor_Angle_Setup(float p[4]) ;
bool Task_Motor_Angle_IsExit(float p[4]) ;
void Task_Motor_Angle_Tick(float p[4]) ;

// =========================== 通用任务 ===========================

// 1. 任务：等待xms，然后Exit（伴随蜂鸣器）
// TASK_WAIT_TIME: p[0]=等待时间(ms)
void Task_Wait_Time_Setup(float p[4]) ;
bool Task_Wait_Time_IsExit(float p[4]) ;

// =========================== MSPM0 新增小车任务 ===========================

// 2. 任务：电机A旋转特定角度，旋转完成之后停止，Exit
// TASK_MOTOR_A_ANGLE: p[0]=旋转角度°, p[1]=容差°
void Task_Motor_A_Angle_Setup(float p[4]) ;
bool Task_Motor_A_Angle_IsExit(float p[4]) ;
void Task_Motor_A_Angle_Tick(float p[4]) ;

// 3. 任务：电机B旋转特定角度，旋转完成之后停止，Exit
// TASK_MOTOR_B_ANGLE: p[0]=旋转角度°, p[1]=容差°
void Task_Motor_B_Angle_Setup(float p[4]) ;
bool Task_Motor_B_Angle_IsExit(float p[4]) ;
void Task_Motor_B_Angle_Tick(float p[4]) ;

// 4. 任务：步进电机1旋转特定角度，旋转完成之后停止，Exit
// TASK_STEPPER1_ANGLE: p[0]=目标角度°, p[1]=max_speed(0=默认200), p[3]=acc(0=默认200)
void Task_Stepper1_Angle_Setup(float p[4]) ;
bool Task_Stepper1_Angle_IsExit(float p[4]) ;

// 5. 任务：步进电机2旋转特定角度，旋转完成之后停止，Exit
// TASK_STEPPER2_ANGLE: p[0]=目标角度°, p[1]=max_speed(0=默认200), p[3]=acc(0=默认200)
void Task_Stepper2_Angle_Setup(float p[4]) ;
bool Task_Stepper2_Angle_IsExit(float p[4]) ;

// 6. 任务：小车顺时针/逆时针旋转一定角度然后Exit（相对运动，不归零yaw）
// TASK_CAR_YAW: p[0]=相对增量角度°(+顺时针/-逆时针), p[1]=角度容差°(0=默认5°), p[2]=角速度容差°/s(0=默认7°/s)
void Task_Car_Yaw_Setup(float p[4]) ;
void Task_Car_Yaw_Tick(float p[4]) ;
bool Task_Car_Yaw_IsExit(float p[4]) ;

// 7. 任务：香橙派视觉寻迹追踪（存根，待Orange模块移植后激活）
// TASK_ORAN_TRACK: p[0]=goal_x, p[1]=goal_y, p[2]=容差(默认10), p[3]=超时ms(0=不限)
void Task_Oran_Track_Setup(float p[4]) ;
void Task_Oran_Track_Tick(float p[4]) ;
bool Task_Oran_Track_IsExit(float p[4]) ;

// 8. 任务：整车直行（IMU辅助走直线）
// TASK_CAR_STRAIGHT: p[0]=目标距离cm(≤0=一直走), p[1]=容差cm(默认1.0), p[2]=max_speed(0=默认200)
void Task_Car_Straight_Setup(float p[4]) ;
void Task_Car_Straight_Tick(float p[4]) ;
bool Task_Car_Straight_IsExit(float p[4]) ;

#endif
