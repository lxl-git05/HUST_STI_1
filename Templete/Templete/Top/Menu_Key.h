#ifndef __MENU_KEY_H
#define __MENU_KEY_H

#include "DList.h"
#include "OLED.h"
#include "Key.h"
#include "stdbool.h"
#include "Key_Check.h"

extern int Menu_Confirm_index ;		// 菜单确认浮标,根据这个判断上位机的命令

// 菜单系统初始化
void Menu_Init(void);

// 菜单展示界面(放在主函数),所有菜单的通用逻辑:长按KEY1打开OLED,再次长按KEY1关闭OLED,长按KEY2回到主界面
void Menu_Func(void) ;

#endif
