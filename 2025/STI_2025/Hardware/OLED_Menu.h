#ifndef __OLED_MENU_H
#define __OLED_MENU_H

#include "main.h"
#include "OLED.h"
#include "Key.h"

typedef struct 
{
	int Menu_Index ;								 // 菜单索引
	int Menu_Next	 ;								 // 下一个索引
	int Menu_Enter ; 								 // 进入子菜单
	void (*Menu_Func)(void); 				 //当前状态应该执行的操作
}Menu_Typedef ;

// 菜单执行功能
void Menu_Func(void) ;

// 得到菜单执行任务光标
int Menu_Get_Status(void) ;

#endif
