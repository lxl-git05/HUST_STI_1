#include "Con_Mode_5.h"
// 小车置于 A点，钢球置于中心点O，按键启动后沿黑线顺时针行驶一圈
// 并通过A位置，整圈行驶总时间≤30s，要求行驶过程中钢球须稳定在摆杆中心点附近，误差绝对值≤1cm。

void Con_Mode_5_Setup(void)
{
    
}

void Con_Mode_5_Loop(void)
{
	OLED_Printf(0, 0, OLED_6X8, "=====Con_Mode_5=====") ;
	
}

void Con_Mode_5_Tick(void)
{
	
}

void Con_Mode_5_Exit(void)
{
    OLED_Clear();
}
