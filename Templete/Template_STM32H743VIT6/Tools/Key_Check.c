#include "Key_Check.h"
// 参数结构体
typedef struct 
{
    const char *name;   // 名称
    void *var;          // 参数指针
    float step;         // 调节步进
    ParamType type;     // 类型
} ParamItem;

// ============= 按键调参相关参数 =============
#define OLED_MAX_SHOW 5		// OLED最多可显示的数目
int cursor = 0;						// OLED光标
int OLED_Start = 0;				// OLED起始位置

// 主系统链表
ParamItem ParamList[MAX_PARAM];
int ParamCount = 0;				// 参数数量

// ============= 相关函数 =============

// 添加参数
void Key_AddParam(const char *name, void *var, float step, ParamType type)
{
	if (ParamCount >= MAX_PARAM) return;
	// 填充参数
	ParamList[ParamCount].name = name;
	ParamList[ParamCount].var  = var;
	ParamList[ParamCount].step = step;
	ParamList[ParamCount].type = type;
	// 参数数目监控
	ParamCount++;
}
// OLED展示参数
void Key_ShowParams(void)
{
	// 先清屏
	OLED_Clear();
	// 起始位和结束位
	int end = OLED_Start + OLED_MAX_SHOW;
	if (end > ParamCount) end = ParamCount;
	// OLED展示
	for(int i = OLED_Start, line = 0; i < end; i++, line++)
	{
		ParamItem *item = &ParamList[i];

		(i == cursor)
				? OLED_Printf(0, line*11 + 11, OLED_6X8, "> ")
				: OLED_Printf(0, line*11 + 11, OLED_6X8, "  ");

		if(item->type == PARAM_FLOAT)
				OLED_Printf(12, line*11 + 11, OLED_6X8, "%s:%.2f %.2f", item->name, *(float*)item->var , item->step);
		else
				OLED_Printf(12, line*11 + 11, OLED_6X8, "%s:%d %.0f", item->name, *(int*)item->var, item->step);
	}

	OLED_Update();
}

// 参数自增
void Key_ParamUp(void)
{
    ParamItem *item = &ParamList[cursor];

    if(item->type == PARAM_FLOAT)
        *(float*)item->var += item->step;
    else
        *(int*)item->var += (int)item->step;
}

// 参数自减
void Key_ParamDown(void)
{
    ParamItem *item = &ParamList[cursor];

    if(item->type == PARAM_FLOAT)
        *(float*)item->var -= item->step;
    else
        *(int*)item->var -= (int)item->step;
}

// 光标上划
void Key_CursorUp(void)
{
    if (cursor > 0) cursor--;

    if (cursor < OLED_Start)
        OLED_Start = cursor;
}

// 光标下划
void Key_CursorDown(void)
{
    if (cursor < ParamCount - 1) cursor++;

    if (cursor >= OLED_Start + OLED_MAX_SHOW)
        OLED_Start = cursor - OLED_MAX_SHOW + 1;
}

// 核心程序
void Key_Param_Check(void)
{
	// 按键操作
	if(Key_Check(KEY_1, KEY_SINGLE))
	{
		Key_CursorDown();
	}		
	else if (Key_Check(KEY_1, KEY_DOUBLE))
	{
		Key_CursorUp();
	}
	
	if(Key_Check(KEY_2, KEY_SINGLE))
	{
		Key_ParamUp();
	}		
	else if (Key_Check(KEY_2, KEY_DOUBLE))
	{
		// 正步进变为负步进
		ParamItem *item = &ParamList[cursor];
		item->step = - item->step; 
	}
	// OLED展示参数
  Key_ShowParams();
}

