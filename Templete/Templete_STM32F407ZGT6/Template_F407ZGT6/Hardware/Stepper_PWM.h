#ifndef __STEPPER_PWM_H
#define __STEPPER_PWM_H

#include "MySystem.h"
#include "MyPID.h"

#define STEPPER_DIR_P   (1)    // 正向
#define STEPPER_DIR_N   (-1)   // 反向

#define STEPPER_V_MIN   (1.0f)  // 位置模式最小起停速度(rpm)

// 位置模式阶段枚举
#define POS_PHASE_IDLE   0      // 空闲
#define POS_PHASE_ACCEL  1      // 加速段
#define POS_PHASE_CRUISE 2      // 匀速段
#define POS_PHASE_DECEL  3      // 减速段

typedef struct
{
    // 驱动(默认使能)
    MyPWM_Typedef* PWM;
    MyGPIO_Typedef* GPIO_Dir;           // 方向GPIO
    // 电机参数
    float pulse_angle;                  // 单位脉冲角度(度)
    int8_t Positive_Dir;                // 正方向(1 or -1)
    // 内部参数
    float Pos_Now;                      // 当前旋转的绝对角度(度)
    float Pos_Tar;                      // 目标角度(度)
    float Speed_Now;                    // 当前的速度，0=停止，>0=正转，<0=反转
    // 限位功能（纯软件，基于Pos_Now角度检测）
    float Limit_Angle_Max;
    float Limit_Angle_Min;
    uint8_t Limit_Enable;
    float Acc_Val;                      // 加速度 rpm/s，0=瞬时响应
    float Speed_Tar;                    // 目标速度（rpm/s），用于加速度ramp
    // --- 位置模式 ---
    float   Pos_MaxSpeed;               // 运动最大速度(rpm)
    float   Pos_Acc;                    // 加速度(rpm/s)
    uint8_t Pos_Phase;                  // 当前阶段 @POS_PHASE_IDLE/ACCEL/CRUISE/DECEL
    int32_t Pos_TotalSteps;             // 运动总步数
    int32_t Pos_AccSteps;               // 加速段步数
    int32_t Pos_CruiseSteps;            // 匀速段步数
    int32_t Pos_StepCnt;                // 当前已走步数
    int8_t  Pos_MoveDir;                // 运动方向(+1/-1)
    float   Pos_StartAngle;             // 运动起始角度
    float   Pos_TargetAngle;            // 运动目标绝对角度
    // PID参数
    Pid_Typedef PID_Angle;              // 香橙派角度对应PID
} Stepper_PWM_Typedef;

extern Stepper_PWM_Typedef Stepper1 ;
extern Stepper_PWM_Typedef Stepper2 ;

// 初始化
void Stepper_PWM_Init(Stepper_PWM_Typedef* pStepper , MyPWM_Typedef* PWM , MyGPIO_Typedef* GPIO_Dir , float pulse_angle , int8_t Positive_Dir) ;

// 速度配置(rpm)
void Stepper_PWM_Speed_Set(Stepper_PWM_Typedef* pStepper, float Speed , float acc);

// 电机制动（停止）
void Stepper_PWM_Stop(Stepper_PWM_Typedef* pStepper);

// 脉冲中断处理（每脉冲完成调用一次，在TIM12更新中断中调用）
void Stepper_PWM_Pulse_Count(Stepper_PWM_Typedef* pStepper);

// 限位配置与检查  
void Stepper_PWM_Limit_Config(Stepper_PWM_Typedef* pStepper, float Limit_Angle_Max, float Limit_Angle_Min);
uint8_t Stepper_PWM_Limit_Check(Stepper_PWM_Typedef* pStepper, float target_speed);

void Stepper_PWM_Speed_Tick(Stepper_PWM_Typedef* pStepper);

// =================== 位置功能 ===================

// 绝对角度旋转
void Stepper_PWM_Pos_Set_Abs(Stepper_PWM_Typedef* pStepper, float target_angle, float max_speed, float acc);

// 相对角度旋转
void Stepper_PWM_Pos_Set_Rel(Stepper_PWM_Typedef* pStepper, float relative_angle, float max_speed, float acc);

// 位置模式1ms Tick（速度ramp + 阶段切换）
void Stepper_PWM_Pos_Tick(Stepper_PWM_Typedef* pStepper);

// 角度到达检测（速度≈0 且 角度≈目标）
bool Stepper_PWM_Is_Angle(void);                                        // 双电机同时判定
bool Stepper_PWM_Is_Angle_Stepper(const Stepper_PWM_Typedef* pStepper); // 单电机判定

#endif
