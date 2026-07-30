#ifndef __ORANGE_H
#define __ORANGE_H

#include "MySystem.h"
#include "Serial_porting.h"
#include "MyPID.h"

extern int Oran_Param[6] ;

extern int Oran_Goal ;	// STM32发送给Orange的参数
extern int Oran_real 	;	// 1. 真实值(偏移)
extern int Oran_Speed ;	// 2. 速度

// Oran数据更新
void Oran_Update(void) ;
void Oran_Filter_10ms(void) ;
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
