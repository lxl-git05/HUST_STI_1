// Robot_Task.h — 晾衣机器人业务库（参数 + 序列 + 安全 + 命令）
// 任务原语已并入全局任务表（Con_Task.h 枚举 + Control.c 实现）:
//   TASK_MOTOR_TO(p0=电机0A/1B, p1=角度°, p2=容差°, 无超时)
//   TASK_SERVO_SET(p0=角色索引, p1=角度, p2=保持ms, 0=立即完成)
// 舵机角色宏: SERVO_CLAW_A/B、SERVO_HANGER_1/2、ROBOT_SERVO_*（Control.h）
// 阈值: 命名阈值 Th_*（AT24C02 持久化）+ LCD 脱机示教命令标定
// 业务: 晾衣第1轮 + 复位（第2轮后续扩展）
#ifndef __ROBOT_TASK_H
#define __ROBOT_TASK_H

#include "MySystem.h"
#include "Con_Task.h"
#include "Serial_porting.h"
#include "Con_Motor.h"
#include "Con_Servo.h"

// ==================== 常量（V2 实测值）====================
#define ROBOT_ANGLE_TOL_DEFAULT        20      // 电机角度容差 ±20
#define ROBOT_SERVO_HOLD_CLAW_CLOSE_MS 2000    // 夹爪闭合延迟等待
#define ROBOT_SERVO_HOLD_CLAW_OPEN_MS  600     // 夹爪张开延迟等待
#define ROBOT_SERVO_HOLD_HANGER_MS     1500    // 衣架动作延迟等待

// ==================== 命名阈值（AT24C02 持久化，Robot_Task.c 定义）====================
extern int32_t Th_Hanger_Up;        // 丝杆顶位  默认 0
extern int32_t Th_Hanger_Mid;       // 丝杆中位  默认 1000
extern int32_t Th_Hanger_Down;      // 丝杆低位  默认 6900
extern int32_t Th_Sigan_Step;       // 传送带一格 默认 330
extern int32_t Th_ClawA_Open;       // 夹爪A开 默认 50
extern int32_t Th_ClawA_Close;      // 夹爪A闭 默认 85
extern int32_t Th_ClawB_Open;       // 夹爪B开 默认 84
extern int32_t Th_ClawB_Close;      // 夹爪B闭 默认 43
extern int32_t Th_Hanger1_Open;     // 衣架1开 默认 150
extern int32_t Th_Hanger1_Close;    // 衣架1闭 默认 70


// ==================== 业务 API（Mode_4 调用）====================
void    Robot_Task_Init(void);                    // 注册全局任务表 + 清错误（Mode_4_Setup 调用）
void Robot_Hang_Enqueue(void);										// 开始晾衣服
void    Robot_Reset_Start(void);                  // 复位（先清队列，任何状态可用）

#endif
