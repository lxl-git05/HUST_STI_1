#include "Mymain.h"
#include "AllHeader.h"
// =================== 全局变量 ===================

void Mymain(void)
{
	Mode_G_Setup() ;    // 全局初始化

	while (1) 
	{
			OLED_Clear() ;
			Mode_G_Loop() ; 

			if (curr_mode == next_mode)
			{
					switch (curr_mode) 
					{
							case Mode_Null : break; // 只有Global模式在干活
							case 1 : Mode_1_Loop() ; break;
							case 2 : Mode_2_Loop() ; break;
							case 3 : Mode_3_Loop() ; break;
							case 4 : Mode_4_Loop() ; break;
							case Mode_End  : break; // 到头了,不要到这里来,写case是因为不然报出警告
					}
			}
			else // 模式交接,仅在模式转换才触发一次 
			{
					switch (curr_mode) 
					{
							case Mode_Null : break;
							case 1 : Mode_1_Exit() ; break;
							case 2 : Mode_2_Exit() ; break;
							case 3 : Mode_3_Exit() ; break;
							case 4 : Mode_4_Exit() ; break;
							case Mode_End  : break; // 到头了,不要到这里来,写case是因为不然报出警告
					}
					switch (next_mode) 
					{
							case Mode_Null : break;
							case 1 : Mode_1_Setup() ; break;
							case 2 : Mode_2_Setup() ; break;
							case 3 : Mode_3_Setup() ; break;
							case 4 : Mode_4_Setup()  ; break;
							case Mode_End  : break; // 到头了,不要到这里来,写case是因为不然报出警告
					}
			}
			curr_mode = next_mode ; // 状态更新
		  OLED_Update() ;
	}
}
