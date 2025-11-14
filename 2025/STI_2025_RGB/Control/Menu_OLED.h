#ifndef __MENU_OLED_H
#define __MENU_OLED_H

// 不展示引脚,本库与OLED库互相独立,不为从属

#include "OLED.h"
#include "Key.h"

// 菜单显示
void Menu_Display(void) ;

// 通过OLED菜单实现手动档RGB和Servo的参数调控
void Menu_RGB_Servo_Manu_Update(int RGB_Mode , int Servo_Mode , int *RGB_Manu_Num , int *Servo_Manu_Num ) ;

#endif
