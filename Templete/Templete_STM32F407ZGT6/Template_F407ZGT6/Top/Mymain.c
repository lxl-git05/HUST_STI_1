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
							case 5 : Mode_5_Loop() ; break;
							case 6 : Mode_6_Loop() ; break;
			//							case Con_Mode_1 : Con_Mode_1_Loop() ; break;
							case Con_Mode_2 : Con_Mode_2_Loop() ; break;
							case Con_Mode_3 : Con_Mode_3_Loop() ; break;
case Con_Mode_4 : Con_Mode_4_Loop() ; break;
//							case Con_Mode_5 : Con_Mode_5_Loop() ; break;
//							case Con_Mode_6 : Con_Mode_6_Loop() ; break;
							default: break;
					}
			}
			else // 模式切换,先执行模式转换再写一次
			{
					switch (curr_mode)
					{
							case Mode_Null : break;
							case 1 : Mode_1_Exit() ; break;
							case 2 : Mode_2_Exit() ; break;
							case 3 : Mode_3_Exit() ; break;
							case 4 : Mode_4_Exit() ; break;
							case 5 : Mode_5_Exit() ; break;
							case 6 : Mode_6_Exit() ; break;
			//							case Con_Mode_1 : Con_Mode_1_Exit() ; break;
							case Con_Mode_2 : Con_Mode_2_Exit() ; break;
							case Con_Mode_3 : Con_Mode_3_Exit() ; break;
case Con_Mode_4 : Con_Mode_4_Exit() ; break;
//							case Con_Mode_5 : Con_Mode_5_Exit() ; break;
//							case Con_Mode_6 : Con_Mode_6_Exit() ; break;
							default: break;
					}
					switch (next_mode)
					{
							case Mode_Null : break;
							case 1 : Mode_1_Setup() ; break;
							case 2 : Mode_2_Setup() ; break;
							case 3 : Mode_3_Setup() ; break;
							case 4 : Mode_4_Setup()  ; break;
							case 5 : Mode_5_Setup()  ; break;
							case 6 : Mode_6_Setup()  ; break;
			//							case Con_Mode_1 : Con_Mode_1_Setup() ; break;
							case Con_Mode_2 : Con_Mode_2_Setup() ; break;
							case Con_Mode_3 : Con_Mode_3_Setup() ; break;
case Con_Mode_4 : Con_Mode_4_Setup() ; break;
//							case Con_Mode_5 : Con_Mode_5_Setup() ; break;
//							case Con_Mode_6 : Con_Mode_6_Setup() ; break;
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
