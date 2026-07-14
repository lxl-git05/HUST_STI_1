#ifndef __PARAM_EDIT_H
#define __PARAM_EDIT_H

#include "OLED.h"
#include "Key.h"

#define MAX_PARAM  30           // 最大参数个数

// ================ AT24C02 联动开关 ================
#define PARAM_USE_AT24C02       1           // 1=启用AT24C02联动, 0=完全解耦

// ================ 按键映射（修改此处即可切换按键功能）================
// F407 按键布局: KEY_0(PE4)=Mode_G模式切换, KEY_1(PE3)=板载按键, KEY_2(PF4)=外部按键, KEY_3(PF5)=EC11编码器按键
#define PARAM_KEY_ENTER_EXIT    KEY_1       // 进入/退出参数编辑（长按）
#define PARAM_KEY_NEXT          KEY_1       // 下一个参数（单击）
#define PARAM_KEY_PREV          KEY_1       // 上一个参数（双击）
#define PARAM_KEY_SAVE          KEY_3       // 保存当前参数到AT24C02（单击）

// ================ 参数类型 ================
typedef enum {
    PARAM_INT8,                 // int8_t
    PARAM_INT16,                // int16_t
    PARAM_INT32,                // int32_t
    PARAM_FLOAT                 // float
} ParamType;

// ================ API ================

void Param_Init(void);          // 初始化参数表
void Param_Clear(void);         // 清空参数表，可重新注册
void Param_Register(const char *name, void *var, float step, ParamType type);  // 注册参数
void Param_Loop(void);          // 主循环调用：状态机（进入/退出编辑 + 编码器 + 按键 + OLED）
int  Param_IsActive(void);      // 返回是否处于参数编辑模式（1=编辑中）

#endif
