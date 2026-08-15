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
							case Mode_1 : Mode_1_Loop() ; break;
							case Mode_2 : Mode_2_Loop() ; break;
							case Mode_3 : Mode_3_Loop() ; break;
							case Mode_4 : Mode_4_Loop() ; break;
							case Mode_5 : Mode_5_Loop() ; break;
							case Mode_6 : Mode_6_Loop() ; break;
							case Mode_End  : break; // 纯兜底,不会运行到这里,写case是因为枚举没default
							default: break;
					}
			}
			else // 模式切换,先执行模式转换再写一次
			{
					switch (curr_mode)
					{
							case Mode_Null : break;
							case Mode_1 : Mode_1_Exit() ; break;
							case Mode_2 : Mode_2_Exit() ; break;
							case Mode_3 : Mode_3_Exit() ; break;
							case Mode_4 : Mode_4_Exit() ; break;
							case Mode_5 : Mode_5_Exit() ; break;
							case Mode_6 : Mode_6_Exit() ; break;
							case Mode_End  : break; // 纯兜底,不会运行到这里,写case是因为枚举没default
							default: break;
					}
					switch (next_mode)
					{
							case Mode_Null : break;
							case Mode_1 : Mode_1_Setup() ; break;
							case Mode_2 : Mode_2_Setup() ; break;
							case Mode_3 : Mode_3_Setup() ; break;
							case Mode_4 : Mode_4_Setup()  ; break;
							case Mode_5 : Mode_5_Setup() ; break;
							case Mode_6 : Mode_6_Setup()  ; break;
							case Mode_End  : break; // 纯兜底,不会运行到这里,写case是因为枚举没default
							default: break;
					}
			}
			// 状态更新 + 模式变化时写一次 AT24C02（不再每次循环写）
		{
			static Mode_Typedef last_saved = Mode_Null ;
			curr_mode = next_mode ;
			if (curr_mode != Mode_Null && curr_mode != last_saved)
			{
				Param_AT24C02_Write(&curr_mode) ;
				last_saved = curr_mode ;
			}
		}
		  OLED_Update() ;
	}
}
