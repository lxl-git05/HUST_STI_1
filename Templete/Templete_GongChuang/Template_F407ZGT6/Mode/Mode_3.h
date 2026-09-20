#ifndef __MODE_3_H
#define __MODE_3_H

#include "main.h"

/* 进入Mode3：初始化并使能Stepper1。 */
void Mode_3_Setup(void);

/* Mode3主循环：处理按键、自动往返序列和OLED显示。 */
void Mode_3_Loop(void);

/* 离开Mode3：立即停机并关闭步进电机输出。 */
void Mode_3_Exit(void);

/* 预留的普通模式节拍回调。 */
void Mode_3_Tick(void);

/* 必须每1ms调用一次，用于步进电机加减速规划。 */
void Mode_3_1ms_Tick(void);

#endif
