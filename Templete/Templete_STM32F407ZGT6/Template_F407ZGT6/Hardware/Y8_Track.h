#ifndef __Y8_TRACK_H
#define __Y8_TRACK_H

#include "MySystem.h"
#include "Y8_USART.h"
#include "MyPID.h"

// 有效通道: 0~6（排除故障通道 7），中心索引 = 3.0
#define Y8U_VALID_COUNT  7
#define Y8U_CENTER       3.0f

// 阈值: 白底~200, 设230刚好卡在白底之上
#define Y8U_THRESHOLD    230

// 偏移放大倍率（原始±3 → 放大后±300）
#define Y8U_SCALE        100.0f

// EWMA 平滑系数（0~1，越小越平滑）
#define Y8U_EWMA_ALPHA   0.80f

// PID 基础增益 + 变增益阈值
#define Y8U_KP_BASE      0.12f   // 直道 Kp
#define Y8U_KP_HIGH      0.30f   // 大弯 Kp
#define Y8U_GAIN_LO       80.0f  // 偏移 < 80: 直道 Kp
#define Y8U_GAIN_HI      200.0f  // 偏移 > 200: 大弯 Kp（中间线性过渡）

extern Pid_Typedef Y8U_PID;

// 得到相对中心的偏移（已×SCALE + EWMA 平滑，丢线维持）
float Y8U_GetOffset(void);

// PID 初始化 + 更新（Tick 中调用）
void Y8U_PID_Init(void);
void Y8U_PID_Update(void);

#endif
