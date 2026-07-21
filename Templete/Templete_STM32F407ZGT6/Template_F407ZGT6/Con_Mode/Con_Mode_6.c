#include "Con_Mode_6.h"

// 坐标标定模式:得到3组坐标
int x_pos = 930 ;
int y_pos = 1200;
bool is_Check_X = true ;

void Con_Mode_6_Setup(void)
{
  OLED_Clear();
	// 加入需要调节的值
	Serial_printf(&Serial2 , "@Con_Mode_6:6$#") ;
	// 加入需要调节的参数
	// 初始化参数编辑器
	Param_Init();
	Param_Register("Oran_X_A",  &Oran_X_A,0.1f,PARAM_FLOAT);
	Param_Register("Oran_X_B",  &Oran_X_B,10.0f,PARAM_FLOAT);
	Param_Register("Oran_Y_A",  &Oran_Y_A,0.1f,PARAM_FLOAT);
	Param_Register("Oran_Y_B",  &Oran_Y_B,10.0f,PARAM_FLOAT);
}

void Con_Mode_6_Loop(void)
{
	if (!Param_IsActive())
	{
		// 正常工作代码
		// 展示标定点
		OLED_Printf(0,5,OLED_6X8 ,  "Dot1:%d,%d",Oran_Check_XY[0] , Oran_Check_XY[1]) ;
		OLED_Printf(0,15,OLED_6X8 , "Dot2:%d,%d",Oran_Check_XY[2] , Oran_Check_XY[3]) ;
		OLED_Printf(0,25,OLED_6X8 , "Dot3:%d,%d",Oran_Check_XY[4] , Oran_Check_XY[5]) ;
		OLED_Printf(0,35,OLED_6X8 , "x:a,b:%.2f,%.2f ",Oran_X_A , Oran_X_B) ;
		OLED_Printf(0,45,OLED_6X8 , "y:a,b:%.2f,%.2f ",Oran_Y_A , Oran_Y_B) ;
		OLED_Printf(0,55,OLED_6X8 , "pos:x:%d y:%d ",x_pos , y_pos) ;
		// 开始调节位置:不进行坐标映射
		if (Key_Check(KEY_1 , KEY_SINGLE))
		{
			Stepper_PWM_Pos_Set_Abs(&Stepper1 , -x_pos , 400 , 200) ;	// x方向反了，加个负号
			Stepper_PWM_Pos_Set_Abs(&Stepper2 ,  y_pos , 400 , 200) ;
		}
		// 开始调节位置:进行坐标映射 
		if (Key_Check(KEY_1 , KEY_DOUBLE))
		{
			Stepper_PWM_Pos_Set_Abs(&Stepper1 , -(x_pos * Oran_X_A + Oran_X_B)  , 400 , 200) ;	// x方向反了，加个负号
			Stepper_PWM_Pos_Set_Abs(&Stepper2 ,   y_pos * Oran_Y_A + Oran_Y_B  , 400 , 200) ;
		}
		// 回位
		if (Key_Check(KEY_2 , KEY_SINGLE))
		{
			Stepper_PWM_Pos_Set_Abs(&Stepper1 ,  0 , 400 , 200) ;	// x方向反了，加个负号
			Stepper_PWM_Pos_Set_Abs(&Stepper2 ,  0 , 400 , 200) ;
		}
		// 选择调节的位置参数
		if (Key_Check(KEY_2 , KEY_DOUBLE))
		{
			is_Check_X = !is_Check_X ;
		}
		// 旋转编码器调节
		if (is_Check_X)
		{
			x_pos += Encoder_Get() * 10 ;
		}
		else
		{
			y_pos += Encoder_Get() * 10 ;
		}
	}
	
	Param_Loop();
}

void Con_Mode_6_Tick(void)
{
	
}

void Con_Mode_6_Exit(void)
{
    OLED_Clear();
}
