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
#define Motor_Trans_Next_Cnt 330

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

// ==================== 位置环（F407 移植） ====================
// 1. 设置电机目标位移(cm)
void Motor_SetPos(Motor_Typedef *Motor , float Pos) ;

// 2. 得到电机当前位移(cm)
float Motor_Get_Pos(Motor_Typedef *Motor) ;

// 3. 检查电机位移（速度检查 + 位置容差）
bool Motor_Is_Pos(Motor_Typedef *Motor , float Pos , float Tolerance , float Speed_Tol) ;

// 4. 电机位置环更新Tick（20ms 周期内调用，输出速度给速度环）
void Motorx_Pos_Update_Tick(Motor_Typedef *Motor , int Dir) ;

// 5. 清除双电机累计位移
void Motor_Pos_Clear(void) ;

// ==================== 整车直行环（F407 移植） ====================
// 直行位置环（A轮距离→速度）与偏航环（IMU yaw→差速修正）
extern Pid_Typedef PID_Car_Straight ;
extern Pid_Typedef PID_Straight_Yaw ;

void PID_Car_Straight_Init(void) ;                       // 初始化（位置PD + 偏航PD）
void PID_Car_Straight_Reset(void) ;                      // 清零编码器 + 记录起始yaw + 清PID历史
void PID_Car_Straight_Tick(void) ;                       // 20ms Tick: 位置PID+梯形限速+yaw PD→差速输出
void PID_Car_Straight_SetSpeedParams(float max_speed) ;  // 配置最高巡航速度(rpm)，0=使用默认200

 

#endif
