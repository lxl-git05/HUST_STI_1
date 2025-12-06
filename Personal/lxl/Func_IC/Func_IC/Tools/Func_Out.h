#ifndef __FUNC_OUT_H
#define __FUNC_OUT_H

// 简介:简易函数发生器库函数

#include "main.h"

// 函数发生器初始化
void Func_Out_Init(void) ;
// 方波 : 函数发生器输出方波函数
void Func_Out_Square(uint32_t frequency, uint32_t duty_cycle) ;

// 待添加功能,后续写

// // 正弦波 : 函数发生器输出正弦波函数 amplitude : 振幅
// void Func_Out_Sine(uint32_t frequency, uint32_t amplitude) ;

#endif
