#ifndef __KEY_CHECK
#define __KEY_CHECK

#include "OLED.h"
#include "string.h"
#include "Key.h"

#define MAX_PARAM  30			// !!!最多可调节参数数目!!!

typedef enum 
{
    PARAM_FLOAT = 0u,
    PARAM_INT   = 1u,
} ParamType;

// ================ 按键调参系统函数 ================

// 添加参数
void Key_AddParam(const char *name, void *var, float step, ParamType type) ;

// 核心程序
void Key_Param_Check(void); 

#endif
