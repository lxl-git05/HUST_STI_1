#ifndef __CON_MODE_1_H
#define __CON_MODE_1_H

#include "AllHeader.h"

extern Task_Descriptor_Typedef Con_Mode_Table[TASK_COUNT] ;

// ========================== 电机PID调参 ==========================
// KEY_1单击切换Motor A/B，串口设置Kp/Ki/Kd/Angle

void Con_Mode_1_Setup(void);
void Con_Mode_1_Loop(void);
void Con_Mode_1_Tick(void);
void Con_Mode_1_Exit(void);

#endif
