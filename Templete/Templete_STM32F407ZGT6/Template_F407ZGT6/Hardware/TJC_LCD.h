#ifndef __TJC_LCD_H
#define __TJC_LCD_H

#include "MySystem.h"

// 检查LCD串口指令缓冲区中是否包含指定关键字（通用子串匹配）
// 返回: true=匹配到, false=未匹配
bool LCD_Cmd_Check(char *keyword);

#endif
