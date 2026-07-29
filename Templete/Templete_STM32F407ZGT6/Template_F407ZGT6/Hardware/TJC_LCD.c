#include "TJC_LCD.h"

// 检查LCD串口指令缓冲区中是否包含指定关键字（通用子串匹配）
// 当前为存根实现，F407 暂不使用 TJC LCD 指令检测
bool LCD_Cmd_Check(char *keyword)
{
    (void)keyword;
    return false;
}

