#include "Menu_Key.h"
// ******************菜单结构体******************
typedef struct
{
    int id;													// 菜单的ID号(序列)
    void (*MenuCallback)(void);			// 菜单的回调函数(功能展示)
} OLED_MenuItem;
// ******************全局变量声明******************
// 初始化参数
DList Menu_list;						// 菜单系统头结点双向链表定义
int Menu_Total_Num = 0 ;		// 菜单总数量
// 交互界面参数
int Menu_Confirm_index ;		// 菜单确认浮标
// ******************菜单回调函数声明******************
void Menu_Main_Callback (void);
void Menu_Check_Callback(void);
void Menu_Task1_Callback(void);
void Menu_Task2_Callback(void);
void Menu_Task3_Callback(void);
void Menu_Task4_Callback(void);
void Menu_Task5_Callback(void);
// ******************核心函数定义****************** 

// 新建新的菜单数据页面
void Menu_New_Init( void (*Menu_Callback)(void) )
{
	// 菜单空间拓展
	OLED_MenuItem *item = malloc(sizeof(OLED_MenuItem));
	// ID号和回调函数(菜单+按键逻辑)定义
	item->id = Menu_Total_Num ++ ;
	item->MenuCallback = Menu_Callback;
	
	// 尾插放入菜单
	DList_PushBack(&Menu_list, item);
}

// 菜单系统初始化
void Menu_Init(void)
{
	// 菜单系统头结点双向链表初始化
	DList_Init(&Menu_list) ;
	// OLED的主菜单(ID:0)
	Menu_New_Init(Menu_Main_Callback)  ;
	// 后续新建界面:
	Menu_New_Init(Menu_Check_Callback) ;
	Menu_New_Init(Menu_Task1_Callback) ;
	Menu_New_Init(Menu_Task2_Callback) ;
	Menu_New_Init(Menu_Task3_Callback) ;
	Menu_New_Init(Menu_Task4_Callback) ;
	Menu_New_Init(Menu_Task5_Callback) ;
}

// 获取菜单项对应序号的指针
OLED_MenuItem* Menu_Get_Item(int MenuIndex)
{
    // 根据 MenuIndex 获取链表中的节点
    DListNode* node = DList_GetNode(&Menu_list, MenuIndex);
    
    // 错误检查（防止越界）
    if(node == NULL)
        return NULL;

    // 转回真正的数据结构类型
    return (OLED_MenuItem*)node->data;
}

// 菜单界面开关逻辑,true为开,false为关
bool Menu_isOpen_Mode(void)
{
	// 菜单界面开关逻辑
	static int Menu_Open_Mode = 0;
	if (Key_Check(KEY_1 , KEY_LONG))
	{
		// 如果是关闭状态就打开
		if (Menu_Open_Mode == 0)
		{
			Menu_Open_Mode = 1; 	// 打开
		}
		// 如果是打开状态就预备关闭
		else if (Menu_Open_Mode == 1)
		{
			Menu_Open_Mode = 2 ;	// 预备关闭
		}
		// 预备关闭
		if (Menu_Open_Mode == 2)
		{
			OLED_Clear()  ;
			OLED_Update() ;
			Menu_Open_Mode = 0 ;
		}
	}
	return Menu_Open_Mode ;
}

// 菜单展示界面(放在主函数),所有菜单的通用逻辑:长按KEY1打开OLED,再次长按KEY1关闭OLED,长按KEY2回到主界面
void Menu_Func(void)
{
	// 菜单界面开关逻辑,长按按键1打开菜单,再次长按按键1关闭菜单,如果菜单打开就执行对应逻辑
	if (Menu_isOpen_Mode() == true)
	{
		// 通用逻辑:长按KEY2回到主界面
		if (Key_Check(KEY_2 , KEY_LONG))
		{
			Menu_Confirm_index = 0 ;
		}
		OLED_Clear() ;
		// OLED菜单展示界面
		OLED_MenuItem* Menu_Now_Item = Menu_Get_Item(Menu_Confirm_index) ;
		Menu_Now_Item->MenuCallback() ;
		// 怕自己忘记更新OLED,直接在这里更新得了
		OLED_Update() ;	
	}
}

// ******************菜单回调函数定义****************** 

void Menu_Main_Callback(void)
{
	// OLED展示界面
	OLED_Printf(0 , 0 , OLED_8X16 , "======Menu======") ;
	
	OLED_Printf(0 , 20  , OLED_6X8 , "Check") ;
	OLED_Printf(0 , 35  , OLED_6X8 , "Task1") ;
	OLED_Printf(0 , 50  , OLED_6X8 , "Task2") ;
	
	OLED_Printf(60 , 20 , OLED_6X8 , "Task3") ;
	OLED_Printf(60 , 35 , OLED_6X8 , "Task4") ;
	OLED_Printf(60 , 50 , OLED_6X8 , "Task5") ;
	
	// 按键逻辑:单击进入下一个菜单
	if (Key_Check(KEY_1 , KEY_SINGLE))
	{
		Menu_Confirm_index ++ ;
		Menu_Confirm_index = Menu_Confirm_index % Menu_Total_Num ;
	}
}

void Menu_Check_Callback(void)
{
	OLED_Printf(0 , 0 , OLED_6X8 , "=========Check=========") ;
	
	// 按键逻辑:单击进入下一个菜单
	if (Key_Check(KEY_1 , KEY_SINGLE))
	{
		Menu_Confirm_index ++ ;
		Menu_Confirm_index = Menu_Confirm_index % Menu_Total_Num ;
	}
	
}

void Menu_Task1_Callback(void)
{
	OLED_Printf(0 , 0 , OLED_6X8 , "=========Task1=========") ;
		
	// 按键逻辑:单击进入下一个菜单
	if (Key_Check(KEY_1 , KEY_SINGLE))
	{
		Menu_Confirm_index ++ ;
		Menu_Confirm_index = Menu_Confirm_index % Menu_Total_Num ;
	}
	
}

void Menu_Task2_Callback(void)
{
	OLED_Printf(0 , 0 , OLED_6X8 , "=========Task2=========") ;
		
	// 按键逻辑:单击进入下一个菜单
	if (Key_Check(KEY_1 , KEY_SINGLE))
	{
		Menu_Confirm_index ++ ;
		Menu_Confirm_index = Menu_Confirm_index % Menu_Total_Num ;
	}
	
}

void Menu_Task3_Callback(void)
{
	OLED_Printf(0 , 0 , OLED_6X8 , "=========Task3=========") ;
		
	// 按键逻辑:单击进入下一个菜单
	if (Key_Check(KEY_1 , KEY_SINGLE))
	{
		Menu_Confirm_index ++ ;
		Menu_Confirm_index = Menu_Confirm_index % Menu_Total_Num ;
	}
	
}

void Menu_Task4_Callback(void)
{
	OLED_Printf(0 , 0 , OLED_6X8 , "=========Task4=========") ;
		
	// 按键逻辑:单击进入下一个菜单
	if (Key_Check(KEY_1 , KEY_SINGLE))
	{
		Menu_Confirm_index ++ ;
		Menu_Confirm_index = Menu_Confirm_index % Menu_Total_Num ;
	}
	
}

void Menu_Task5_Callback(void)
{
	OLED_Printf(0 , 0 , OLED_6X8 , "=========Task5=========") ;
		
	// 按键逻辑:单击进入下一个菜单
	if (Key_Check(KEY_1 , KEY_SINGLE))
	{
		Menu_Confirm_index ++ ;
		Menu_Confirm_index = Menu_Confirm_index % Menu_Total_Num ;
	}
	
}
