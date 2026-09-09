#ifndef __RGB_H
#define __RGB_H

#include "MySystem.h"

// =========== API ===========
void RGB_Init(void);                               // 初始化(默认灭)
void RGB_Set_Color(int R, int G, int B);           // 设置RGB(0/1, 共阳极: 1=亮)

#endif
