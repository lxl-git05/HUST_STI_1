#ifndef __MATCH_H
#define __MATCH_H

#include "MySystem.h"

// ==================== 比赛专用逻辑：寻迹加减速控制 ====================
// 包含: S曲线速度规划 + 多传感器融合阻尼 + 球振荡检测 + 终点检测
// 调用者负责: Oran_PID_Init / Oran_Update / Oran_PID_Update / Stepper stop

void     RaceCtrl_Setup(void);
void     RaceCtrl_Start(void);
void     RaceCtrl_Loop(void);
void     RaceCtrl_Tick(void);
void     RaceCtrl_Exit(void);

uint8_t  RaceCtrl_IsRunning(void);
uint8_t  RaceCtrl_IsStopped(void);
float    RaceCtrl_GetTime(void);

#endif
