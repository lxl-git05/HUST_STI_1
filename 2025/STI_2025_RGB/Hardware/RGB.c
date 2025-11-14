#include "RGB.h"

// RGB初始化
void RGB_Init(void)
{
	// 初始化
	PWM_Init(&RGB_htim , RGB_R_Channel ) ;
	PWM_Init(&RGB_htim , RGB_G_Channel ) ;
	PWM_Init(&RGB_htim , RGB_B_Channel ) ;
	// 初始为不发光
	PWM_SetCompare1(RGB_htim , RGB_R_Channel , 0 ) ;
	PWM_SetCompare1(RGB_htim , RGB_G_Channel , 0 ) ;
	PWM_SetCompare1(RGB_htim , RGB_B_Channel , 0 ) ;
	
}

// RGB调色
void RGB_Set_Color(int R_Color , int G_Color , int B_Color )
{
	// 限幅
	if (R_Color > 100)
		R_Color = 100 ;
	else if (R_Color < 0)
		R_Color = 0 ;
	if (G_Color > 100)
		G_Color = 100 ;
	else if (G_Color < 0)
		G_Color = 0 ;
	if (B_Color > 100)
		B_Color = 100 ;
	else if (B_Color < 0)
		B_Color = 0 ;
	
	// 调色
	PWM_SetCompare1(RGB_htim , RGB_R_Channel , R_Color ) ;
	PWM_SetCompare1(RGB_htim , RGB_G_Channel , G_Color ) ;
	PWM_SetCompare1(RGB_htim , RGB_B_Channel , B_Color ) ;
	
}
