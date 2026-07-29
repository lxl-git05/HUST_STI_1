#ifndef __Y8_DRIVER_H
#define __Y8_DRIVER_H

#include "MySystem.h"
#include "MyPID.h"

// ========================== Y8 8路寻迹传感器驱动 ==========================
// 通信协议: 2线同步串行 (CLK + DAT)
//   - CLK: 主控输出时钟, 空闲低电平
//   - DAT: 主控输入, 辅助板在CLK上升沿更新数据
//   - MSB先出, 每bit约20μs (5μs高+5μs低)
//   - 1=白色/浅色地面, 0=黑色/深色线条
// 引脚: CLK=PF15, DAT=PF13 (上拉输入)
// ========================================================================

// 8路传感器数据 (bit i 展开为 Y8_Data[i])
//   Y8_Data[0] = 第1路传感器 (第1个CLK)
//   Y8_Data[7] = 第8路传感器 (第8个CLK)
//   值: 1=白, 0=黑
extern uint8_t Y8_Data[8];

// 8路传感器距中心横向距离 (mm), 左负右正
extern const int8_t Y8_Width[8];

// 滤波后的角度偏移值 (度), 正=右偏, 0=居中
extern float Y8_Bias;

extern Pid_Typedef PID_Track ;

extern float Track_Base_Speed ;

// 数据更新: 读取8路传感器并展开到 Y8_Data[8]
// ★ 应在中断中调用以保证实时性 (如Timer_20ms_Callback)
void Y8_Data_Update(void);

// 得到滤波后的角度偏移值 (PID巡线real输入, goal=0)
// cnt: 单次调用内采样次数 (typ 10), 用于多数投票去噪
// 返回: 滤波后的角度偏移 (度), 正=右偏, 0=居中
float Y8_Angle_Bias_Get(uint16_t cnt);

// Y8初始化
void Y8_Init(void) ;

// Y8巡线更新 + 巡线
void Y8_PID_Update(void) ;

#endif
