// Menu_Param.h — 调参任务菜单（参照 Control.c 模式）
// 结构: TuneTaskID枚举 → Setup/Run/IsExit/Tick回调 → 任务表 → Con_Task调度
// 按键: KEY_1单击=下一项, KEY_1长按=入队/再次长按=Skip
#ifndef __MENU_PARAM_H
#define __MENU_PARAM_H

#include "MySystem.h"
#include "Con_Task.h"

// ==================== 调参任务枚举 ====================
typedef enum {
    TUNE_MOTOR_A_SPEED = 0,  // Motor_A 速度环
    TUNE_MOTOR_A_ANGLE,      // Motor_A 角度环
    TUNE_MOTOR_A_POS,        // Motor_A 位置环
    TUNE_MOTOR_B_SPEED,      // Motor_B 速度环
    TUNE_MOTOR_B_ANGLE,      // Motor_B 角度环
    TUNE_MOTOR_B_POS,        // Motor_B 位置环
    TUNE_CAR_STRAIGHT,       // 整车直行环（仅1个）
    TUNE_GYRO_YAW,           // IMU偏航角环
    TUNE_GYRO_CAL,           // IMU校准(一次性)
    TUNE_STEPPER_S1,         // 步进电机1（开环）
    TUNE_STEPPER_S2,         // 步进电机2（开环）
    TUNE_Y8_TRACK,           // Y8 巡线PID跟踪
    TUNE_ORANGE_PARAM,       // 香橙派通信参数调节
    TUNE_COUNT
} TuneTaskID;

// ==================== 任务回调声明 ====================
void Tune_MotorA_Speed_Setup (float p[4]);
void Tune_MotorA_Speed_Run   (float p[4]);
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
void Tune_Gyro_Yaw_Setup     (float p[4]);
void Tune_Gyro_Yaw_Run       (float p[4]);
void Tune_Gyro_Yaw_Tick      (float p[4]);
void Tune_Gyro_Cal_Setup     (float p[4]);
void Tune_Gyro_Cal_Run       (float p[4]);
bool Tune_Gyro_Cal_IsExit    (float p[4]);
void Tune_Gyro_Cal_Tick      (float p[4]);
void Tune_Stepper_S1_Run     (float p[4]);
void Tune_Stepper_S2_Run     (float p[4]);
void Tune_Stepper_S1_Tick    (float p[4]);
void Tune_Stepper_S2_Tick    (float p[4]);
void Tune_Y8_Track_Setup     (float p[4]);
void Tune_Y8_Track_Run       (float p[4]);
void Tune_Y8_Track_Tick      (float p[4]);
void Tune_Orange_Setup       (float p[4]);
void Tune_Orange_Run         (float p[4]);
void Tune_Orange_Tick        (float p[4]);
bool Tune_AlwaysFalse        (float p[4]);

// ==================== 任务表（同 Control_TaskTable）====================
extern Task_Descriptor_Typedef Menu_Tune_Table[TUNE_COUNT];

// ==================== 菜单导航 API ====================
void Menu_Tune_Init(void);
void Menu_Tune_Loop(void);
int  Menu_Tune_Cursor(void);

#endif
