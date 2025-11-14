#include "Menu_OLED.h"

// *************菜单光标显示参数*************
int Menu_Choose_Index  = 0 ;	// 初始不显示
int Menu_Confirm_Index = 0 ;	// 初始不显示

int cur_x_choose;
int cur_y_choose;

int cur_x_confirm ;
int cur_y_confirm ;

// *************函数*************
void Cursor_Display(void) ;	// 光标显示:"<-"表示选择位 , "*"表示确定位

// 主函数:显示菜单
void Menu_Display(void)
{
	// 菜单主标题
	OLED_Printf(0 , 0 , OLED_6X8 , "=========Menu=========" ) ;
	
	// 展示5个参数: R , Y , G , Left , Right
	// 第一列:颜色
	OLED_Clear() ;
	OLED_Printf(0 , 15 , OLED_6X8 , "Red   ") ;
	OLED_Printf(0 , 30 , OLED_6X8 , "Yellow") ;
	OLED_Printf(0 , 45 , OLED_6X8 , "Green ") ;
	// 第二列:方向
	OLED_Printf(63 , 15 , OLED_6X8 , "Left ") ;
	OLED_Printf(63 , 30 , OLED_6X8 , "Right") ;
	OLED_Printf(63 , 45 , OLED_6X8 , "Hello") ;
	
	// 光标展示
	Cursor_Display() ;
}

// 光标显示:"<-"表示选择位 , "*"表示确定位
void Cursor_Display(void)
{
	// 按键1单击表示选择光标下移一位
	if (Key_Check(KEY_1 , KEY_SINGLE))
	{
		Menu_Choose_Index ++ ;
		if (Menu_Choose_Index >= 7)
		{
			Menu_Choose_Index = 1 ;			// 循环
		}
	}
	
	// 选择光标标记位置
	cur_x_choose = ( (Menu_Choose_Index - 1) / 3 ) ? 127 - 15 :  63 - 15 ;
	cur_y_choose = 15 + ( (Menu_Choose_Index - 1) % 3 ) * 15 ;
	
	// 按键2单击表示确定光标标记的参数
	if (Key_Check(KEY_2 , KEY_SINGLE))
	{	
		Menu_Confirm_Index = Menu_Choose_Index ;	// 确定键
		cur_x_confirm = cur_x_choose - 10 ;				// 确定光标要在左边,所以-10
		cur_y_confirm = cur_y_choose      ;				// y 相同
	}
	
	// 展示光标
	if (Menu_Choose_Index != 0)
	{
		OLED_Printf(cur_x_choose , cur_y_choose , OLED_6X8 , "<-") ;
	}
	if (Menu_Confirm_Index != 0)
	{
		OLED_Printf(cur_x_confirm , cur_y_confirm , OLED_6X8 , "*") ;
	}
}

// 通过OLED菜单实现手动档RGB和Servo的参数调控
void Menu_RGB_Servo_Manu_Update(int RGB_Mode , int Servo_Mode , int *RGB_Manu_Num , int *Servo_Manu_Num )
{
	// 手动模式下进行RGB控制操作
	if (RGB_Mode == 0)
	{
		if (Menu_Confirm_Index == 1)
		{
			*RGB_Manu_Num = 1 ;	// 红色
		}
		else if (Menu_Confirm_Index == 2)
		{
			*RGB_Manu_Num = 2 ;	// 黄色
		}
		else if (Menu_Confirm_Index == 3 )
		{
			*RGB_Manu_Num = 3 ;	// 绿色
		}
	}
	// 手动模式下进行Servo控制操作
	if (Servo_Mode == 0)
	{
		if (Menu_Confirm_Index == 4)
		{
			*Servo_Manu_Num = 1 ;	// L
		}
		else if (Menu_Confirm_Index == 5)
		{
			*Servo_Manu_Num = 2 ;	// R
		}
	}
}
