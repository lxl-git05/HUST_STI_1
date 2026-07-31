#ifndef __ORANGE_H
#define __ORANGE_H

#include "MySystem.h"
#include "Serial_porting.h"
#include "MyPID.h"

extern int Oran_Param[6] ;

extern int Oran_Goal ;	// STM32发送给Orange的参数
extern int Oran_real 	;	// 1. 真实值(偏移)
extern int Oran_Speed ;	// 2. 速度

extern int Oran_Single_Pos ;

// Oran数据更新
void Oran_Update(void) ;
void Oran_Filter_10ms(void) ;
// ============== Oran_real 滑窗异常剔除（参考 Y8U_CheckFinishLine） ==============
// 注释掉下面这行即可关闭本功能
//#define ORAN_OUTLIER_FILTER

#ifdef ORAN_OUTLIER_FILTER
#define ORAN_WINDOW_SIZE    10      // 窗口帧数（10帧×10ms = 100ms基线）
#define ORAN_WINDOW_RATIO   4.0f    // 异常阈值：|raw| > 基线×400%
#define ORAN_WINDOW_MIN     20      // 基线下限 + 窗口激活阈值（|raw|>20 才检测）
#define ORAN_WINDOW_MAX_REJ 3       // 连续拒绝上限：超此帧数强制接受（真快速运动逃逸）
#endif
// 调阈值处理
void Oran_Send_Data(int *Data) ;
// 坐标映射
void Oran_XY_Change(void) ;

// ================== 位置环 PID ==================
extern Pid_Typedef PID_Oran ;	// 铁球位置PID
extern float Oran_Real_Offset ;	// real偏移量, 模拟Orange Pi输入
extern float Oran_KpHi ;        // 低速Kp
extern float Oran_KpLo ;        // 高速Kp
extern float Oran_KpSpdThrLo ;  // 低速阈值
extern float Oran_KpSpdThrHi ;  // 高速阈值
void Oran_PID_Init(void) ;
void Oran_PID_Update(void) ;
// IMU加速度前馈
extern float Oran_FF_Alpha ;  // 低通系数
extern float Oran_FF_Len   ;  // 板长 cm
extern float Oran_FF_Lift  ;  // 步进每度升降 cm/°
extern float Oran_Damping_K ;  // 速度阻尼系数, 串口在线调
extern float Oran_FF_Enable  ;  // 加速度前馈使能: 1=开, 0=关(Mode_3静止用)
extern float ff_angle ;

// ================== 速度环 PID ==================
extern Pid_Typedef PID_Oran_Speed ;	// 球速PID
extern float Oran_SPD_Kp_Lo ;		// 低速Kp
extern int   Oran_SPD_Thr_Lo ;		// 低速阈值
extern int   Oran_SPD_Thr_Hi ;		// 高速阈值
void Oran_Speed_PID_Init(void) ;
void Oran_Speed_PID_Update(void) ;

// ================== 串级 PID(位置环→速度环) ==================
void Oran_Cascade_Init(void) ;
void Oran_Cascade_Update(void) ;

// ================== 角度跟踪测试（Mode_2 独立测试用） ==================
extern float Oran_Angle_Test_Target ;   // 目标角度（度），可串口修改
void Oran_Angle_Test_Init(void) ;       // 初始化 Stepper1 角度跟踪PID
void Oran_Angle_Test_Update(void) ;     // 10ms调用：设置目标角度

#endif
