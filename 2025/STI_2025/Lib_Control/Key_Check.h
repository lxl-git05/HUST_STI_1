#ifndef __KEY_CHECK
#define __KEY_CHECK

#include "OLED.h"
#include "string.h"
#include "Key.h"

typedef enum 
{
    PARAM_FLOAT = 0,
    PARAM_INT   = 1,
} ParamType;

// 添加参数
void Key_AddParam(const char *name, void *var, float step, ParamType type) ;

// 核心程序
void Key_Param_Check(void); 

#endif
