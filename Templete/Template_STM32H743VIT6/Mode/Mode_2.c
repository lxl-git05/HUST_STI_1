#include "Mode_2.h"
#include "AllHeader.h"

float goal ;
float Kp ;
float Ki ;
float Kd ;

void Mode_2_Setup(void)
{
    OLED_Clear();
}


// ============== Mode_2 主循环 ==============
void Mode_2_Loop(void)
{
		OLED_Printf(0,0,OLED_6X8,"===Mode_2===") ;
    // 按键1
    if (Key_Check(KEY_1, KEY_SINGLE) || LCD_Key_Check(LCD_KEY_1)) 
		{
			Serial_printf(&Serial1 , "Hello\r\n") ;
    }
		// OLED展示参数
		OLED_Printf(0,20,OLED_6X8,"%.2f",goal) ;
		OLED_Printf(0,30,OLED_6X8,"%.2f",Kp) ;
		OLED_Printf(0,40,OLED_6X8,"%.2f",Ki) ;
		OLED_Printf(0,50,OLED_6X8,"%.2f",Kd) ;
		LCD_Set_Float(LCD_PARAM_1 , &Kp , 0 , 1) ;
		LCD_Set_Float(LCD_PARAM_2 , &Ki , 0 , 1) ;
		LCD_Set_Float(LCD_PARAM_3 , &Kd , 0 , 1) ;
}

void Mode_2_Tick(void)
{
	Serial_printf(&Serial1, "%.2f\n",IMU_Yaw_Abs_Get());
}

void Mode_2_Exit(void)
{
	
}
