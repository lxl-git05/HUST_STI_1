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
							case Con_Mode_1 : Con_Mode_1_Loop() ; break;
							case Con_Mode_2 : Con_Mode_2_Loop() ; break;
							case Con_Mode_3 : Con_Mode_3_Loop() ; break;
							case Con_Mode_4 : Con_Mode_4_Loop() ; break;
							case Con_Mode_5 : Con_Mode_5_Loop() ; break;
							case Con_Mode_6 : Con_Mode_6_Loop() ; break;
							case Mode_End  : break; // 纯头标记,不需要干活,写case是因为不然会报警
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
							case Con_Mode_1 : Con_Mode_1_Exit() ; break;
							case Con_Mode_2 : Con_Mode_2_Exit() ; break;
							case Con_Mode_3 : Con_Mode_3_Exit() ; break;
							case Con_Mode_4 : Con_Mode_4_Exit() ; break;
							case Con_Mode_5 : Con_Mode_5_Exit() ; break;
							case Con_Mode_6 : Con_Mode_6_Exit() ; break;
							case Mode_End  : break; // 纯头标记,不需要干活,写case是因为不然会报警
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
							case Con_Mode_1 : Con_Mode_1_Setup() ; break;
							case Con_Mode_2 : Con_Mode_2_Setup() ; break;
							case Con_Mode_3 : Con_Mode_3_Setup() ; break;
							case Con_Mode_4 : Con_Mode_4_Setup() ; break;
							case Con_Mode_5 : Con_Mode_5_Setup() ; break;
							case Con_Mode_6 : Con_Mode_6_Setup() ; break;
							case Mode_End  : break; // 纯头标记,不需要干活,写case是因为不然会报警
					}
			}
			curr_mode = next_mode ; // 状态更新
			// 记忆模式：保存到 AT24C02，下次上电自动恢复
			if (curr_mode != Mode_Null)
				Param_AT24C02_Write(&curr_mode) ;
		  OLED_Update() ;
	}
}
