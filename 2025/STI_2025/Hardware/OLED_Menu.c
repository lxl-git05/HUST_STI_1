#include "OLED_Menu.h"

// *************菜单变量*************
int MenuIndex ;										// 菜单浮标指示
int MenuIndex_Main_Select  = 1 ; 	// 用户选择界面,在主菜单(main)位于1-6
int MenuIndex_Main_Confirm = 0 ;  // 用户确认执行的页面,默认为主菜单
int Menu_Level = 0 ;         		  // 0:主菜单, 1:子菜单层

int Menu_Open_Flag ;							// OLED开关标志位,0:关 1:开 2:即将关闭
// *************外部变量*************


// *************回调函数*************
void Menu_home(void)
{
	// 菜单显示
	OLED_Printf(0 , 0 , OLED_8X16 , "======Menu======") ;
	
	OLED_Printf(0 , 20  , OLED_6X8 , "Task1") ;
	OLED_Printf(0 , 35  , OLED_6X8 , "Task2") ;
	OLED_Printf(0 , 50  , OLED_6X8 , "Task3") ;
	
	OLED_Printf(60 , 20 , OLED_6X8 , "Task4") ;
	OLED_Printf(60 , 35 , OLED_6X8 , "Task5") ;
	OLED_Printf(60 , 50 , OLED_6X8 , "Task6") ;
	
	// 光标显示
	int x = ((MenuIndex_Main_Select - 1) / 3) ? 60 : 0 ;
	int y = ((MenuIndex_Main_Select - 1) % 3) * 15 + 20 ;
	OLED_Printf( x + 40 , y , OLED_6X8 , "<-") ;
}

void Menu_Task1(void)
{
	OLED_Printf(0 , 0 , OLED_8X16 , "======Task1======") ;
}
void Menu_Task2(void)
{
	OLED_Printf(0 , 0 , OLED_8X16 , "======Task2======") ;
}
void Menu_Task3(void)
{
	OLED_Printf(0 , 0 , OLED_8X16 , "======Task3======") ;
}
void Menu_Task4(void)
{
	OLED_Printf(0 , 0 , OLED_8X16 , "======Task4======") ;
}
void Menu_Task5(void)
{
	OLED_Printf(0 , 0 , OLED_8X16 , "======Task5======") ;
}
void Menu_Task6(void)
{
	OLED_Printf(0 , 0 , OLED_8X16 , "======Task6======") ;
}


// *************主控函数*************
// 菜单控制台
Menu_Typedef Menu[] = 
{
	// 一级菜单,展示内容
	{0 , 1 ,  1 , *(Menu_home)},
	
	// 二级菜单,展示功能
	{1 , 2 ,  1 , *(Menu_Task1)},
	{2 , 3 ,  2 , *(Menu_Task2)},
	{3 , 4 ,  3 , *(Menu_Task3)},
	{4 , 5 ,  4 , *(Menu_Task4)},
	{5 , 6 ,  5 , *(Menu_Task5)},
	{6 , 1 ,  6 , *(Menu_Task6)},
};

// 菜单浮标更新
void Menu_Update(void)
{
	// 菜单开启状态:单击表示next
	if (Menu_Open_Flag == 1)	// OLED开机的时候才作数
	{
		// 主菜单层
		if (Menu_Level == 0)
		{
			// 单击：浮标切换（只是浏览）
			if (Key_Check(0 , KEY_SINGLE))
			{
				MenuIndex_Main_Select = Menu[MenuIndex_Main_Select].Menu_Next ;
			}

			// 长按：确认执行当前选中项
			if (Key_Check(0 , KEY_LONG))
			{
				MenuIndex_Main_Confirm = MenuIndex_Main_Select ;
				Menu_Level = 1 ;  // 进入子菜单层
			}
		}
		// 子菜单层
		else if (Menu_Level == 1)
		{
			// 长按：退出子菜单回到主菜单
			if (Key_Check(0 , KEY_LONG))
			{
				Menu_Level = 0 ;
				MenuIndex_Main_Confirm = 0 ;  // 回到主菜单
			}
		}
	}

	// 菜单状态调整:双击表示开关OLED
	if (Key_Check(0 , KEY_DOUBLE))
	{
		// 将OLED打开
		if (Menu_Open_Flag == 0)
		{
			Menu_Open_Flag = 1 ;
		}
		// 准备将OLED关闭
		else if (Menu_Open_Flag == 1)
		{
			Menu_Open_Flag = 2 ;	// 过渡态(因为OLED必须清屏才可以被视为关闭,所以中间态主要是主管清屏)
		}
	}
	
	// 菜单准备关闭:过渡态,清屏
	if (Menu_Open_Flag == 2)
	{
		// 清屏
		OLED_Clear()  ;
		OLED_Update() ;
		// 标志位置0,关闭OLED
		Menu_Open_Flag = 0 ;
	}
}

// 菜单执行功能
void Menu_Func(void)
{
	// 更新菜单浮标
	Menu_Update() ;
	
	// OLED开机才能起作用
	if (Menu_Open_Flag == 1)
	{
		// 清屏防止滞留字符
		OLED_Clear() ;
		
		// 显示主菜单内容（带光标）
		if (MenuIndex_Main_Confirm == 0)
			Menu[0].Menu_Func();
		else
			Menu[MenuIndex_Main_Confirm].Menu_Func();
		
		// OLED屏幕刷新
		OLED_Update() ;
	}
}

int Menu_Get_Status(void)
{
	return MenuIndex_Main_Confirm ;
}
