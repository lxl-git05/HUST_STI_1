#include "Con_Mode_4.h"
// 小车置于A点，钢球置于中心点O，按键启动后沿黑线顺时针行驶并通过B位置，
// AB间行驶时间≤8s，行驶过程中钢球须稳定在摆杆中心点附近，误差绝对值≤1cm。

void Con_Mode_4_Setup(void)
{
    
}

void Con_Mode_4_Loop(void)
{
	OLED_Printf(0, 0, OLED_6X8, "=====Con_Mode_4=====") ;
	
}

void Con_Mode_4_Tick(void)
{
}

void Con_Mode_4_Exit(void)
{
		
}
