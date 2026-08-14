// Menu_Param.h — 调参任务菜单（参照 Control.c 模式）
// 结构: TuneTaskID枚举 → Setup/Run/IsExit/Tick回调 → 任务表 → Con_Task调度
// 按键: KEY_1单击=下一项, KEY_1长按=入队/再次长按=Skip
#ifndef __MENU_PARAM_H
#define __MENU_PARAM_H

#include "MySystem.h"
#include "Con_Task.h"

// ==================== 调参任务枚举 ====================
// H743 裁剪说明: F4 原有 Oran/Stepper 等 Tune 项已删除，保留 IMU 两项 + 恢复 7 个电机/直行调参项
// 枚举值 = 任务表下标（Con_Task 按值索引任务表）
typedef enum {
    TUNE_GYRO_CAL = 0,       // IMU校准(一次性)
    TUNE_GYRO_YAW,           // IMU偏航角环
    TUNE_MOTOR_A_SPEED,      // Motor_A 速度环
    TUNE_MOTOR_A_ANGLE,      // Motor_A 角度环
    TUNE_MOTOR_A_POS,        // Motor_A 位置环
    TUNE_MOTOR_B_SPEED,      // Motor_B 速度环
    TUNE_MOTOR_B_ANGLE,      // Motor_B 角度环
    TUNE_MOTOR_B_POS,        // Motor_B 位置环
    TUNE_CAR_STRAIGHT,       // 整车直行环（仅1个）
    TUNE_COUNT
} TuneTaskID;

// ==================== 任务回调声明 ====================
void Tune_Gyro_Yaw_Setup     (float p[4]);
void Tune_Gyro_Yaw_Run       (float p[4]);
void Tune_Gyro_Yaw_Tick      (float p[4]);
void Tune_Gyro_Cal_Setup     (float p[4]);
void Tune_Gyro_Cal_Run       (float p[4]);
bool Tune_Gyro_Cal_IsExit    (float p[4]);
void Tune_Gyro_Cal_Tick      (float p[4]);
void Tune_MotorA_Speed_Setup (float p[4]);
void Tune_MotorA_Speed_Run   (float p[4]);
void Tune_MotorA_Speed_Tick  (float p[4]);
void Tune_MotorA_Angle_Setup (float p[4]);
void Tune_MotorA_Angle_Run   (float p[4]);
void Tune_MotorA_Angle_Tick  (float p[4]);
void Tune_MotorA_Pos_Setup   (float p[4]);
void Tune_MotorA_Pos_Run     (float p[4]);
void Tune_MotorA_Pos_Tick    (float p[4]);
void Tune_MotorB_Speed_Setup (float p[4]);
void Tune_MotorB_Speed_Run   (float p[4]);
void Tune_MotorB_Speed_Tick  (float p[4]);
void Tune_MotorB_Angle_Setup (float p[4]);
void Tune_MotorB_Angle_Run   (float p[4]);
void Tune_MotorB_Angle_Tick  (float p[4]);
void Tune_MotorB_Pos_Setup   (float p[4]);
void Tune_MotorB_Pos_Run     (float p[4]);
void Tune_MotorB_Pos_Tick    (float p[4]);
void Tune_Car_Straight_Setup (float p[4]);
void Tune_Car_Straight_Run   (float p[4]);
void Tune_Car_Straight_Tick  (float p[4]);
bool Tune_AlwaysFalse        (float p[4]);

// ==================== 任务表（同 Control_TaskTable）====================
extern Task_Descriptor_Typedef Menu_Tune_Table[TUNE_COUNT];

// ==================== 菜单导航 API ====================
void Menu_Tune_Init(void);
void Menu_Tune_Loop(void);
int  Menu_Tune_Cursor(void);

#endif
