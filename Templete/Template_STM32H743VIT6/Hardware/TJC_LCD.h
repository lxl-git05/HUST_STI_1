#ifndef __TJC_LCD_H
#define __TJC_LCD_H

#include "MySystem.h"

// ============== 移植配置（#ifndef 允许在包含前覆盖）==============

#ifndef TJC_LCD_SERIAL
#define TJC_LCD_SERIAL       Serial4
#endif

#ifndef TJC_LCD_KEY_MAX
#define TJC_LCD_KEY_MAX      6
#endif

#ifndef TJC_LCD_PARAM_MAX
#define TJC_LCD_PARAM_MAX    4
#endif

// ============== 按键号宏 ==============
#define LCD_KEY_1   1
#define LCD_KEY_2   2
#define LCD_KEY_3   3
#define LCD_KEY_4   4
#define LCD_KEY_5   5
#define LCD_KEY_6   6

// ============== 滑块ID宏 ==============
#define LCD_PARAM_1   1
#define LCD_PARAM_2   2
#define LCD_PARAM_3   3
#define LCD_PARAM_4   4

// ============== API ==============
bool LCD_Key_Check(uint8_t key_num);                                     // 按键单击检测，读后清零
bool LCD_Set_Int(uint8_t param_id, int *value, int min, int max);        // 滑块→整数映射
bool LCD_Set_Float(uint8_t param_id, float *value, float min, float max);// 滑块→浮点映射
bool LCD_Cmd_Check(char *keyword);                                       // 通用指令子串匹配

// ============== 波形发送（MCU → LCD）==============
void TJC_LCD_Wave_Send_Float(uint8_t ch, float value);                   // 发送 float 到波形控件 s0 的指定通道(0~3)

// ============== 文本/数值发送（MCU → LCD）==============
void TJC_LCD_Send_Text(const char *comp, const char *text);              // comp.txt="..."（TJC 原生命令，\xFF\xFF\xFF 终止）
void TJC_LCD_Send_Num(const char *comp, int32_t val);                    // comp.val=整数（TJC 原生命令，\xFF\xFF\xFF 终止）

// ============== ABC Float100 参数接收（LCD → MCU）==============
bool LCD_Get_ABC_Float100(char *keyword, float *value);                  // 解析 "keyword=整数" 帧，÷100 返回 float

#endif
