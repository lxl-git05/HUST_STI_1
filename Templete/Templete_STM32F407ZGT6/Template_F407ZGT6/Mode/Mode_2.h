#ifndef __MODE_2_H
#define __MODE_2_H

// ========================== 实验模式 ==============================
// 用途：所有实验性代码、新功能验证都写在这里
// 规则：Setup() 放实验初始化, Loop() 放主循环, Tick() 放20ms定时
// =================================================================

#include "main.h"

void Mode_2_Setup(void);

void Mode_2_Loop(void);

void Mode_2_Exit(void);

void Mode_2_Tick(void);

#endif
