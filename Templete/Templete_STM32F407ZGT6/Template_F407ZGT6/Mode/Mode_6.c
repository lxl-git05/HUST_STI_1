// ========================== 业务逻辑模式 ==========================
#include "Mode_6.h"
#include "AllHeader.h"

extern float Tar_XY_Ratio_X ;
extern float Tar_XY_Ratio_Y ;
float Speed_X = 400;
float Speed_Y = 400;
float Acc = 200 ;

void Mode_6_Setup(void)
{
    OLED_Clear();
		x_tar = 500 ;
		y_tar = 800 ;
}

void Mode_6_Loop(void)
{
	OLED_Printf(0, 0, OLED_6X8, "=====Mode_6=====") ;	
	if (Key_Check(KEY_1 , KEY_SINGLE))
	{
		Stepper_PWM_Pos_Set_Abs(&Stepper1 , -x_tar * Tar_XY_Ratio_X , Speed_X , Acc) ;	// x方向反了，加个负号
		Stepper_PWM_Pos_Set_Abs(&Stepper2 ,  y_tar * Tar_XY_Ratio_Y , Speed_Y , Acc) ;
	}
	if (Key_Check(KEY_2 , KEY_SINGLE))
	{
		Stepper_PWM_Pos_Set_Abs(&Stepper1 , 0 , Speed_X , Acc) ;
		Stepper_PWM_Pos_Set_Abs(&Stepper2 , 0 , Speed_Y , Acc) ;
	}
}

void Mode_6_Tick(void)
{
	Serial_printf(&Serial1, "%.2f,%.2f,%.2f,%.2f\n",Stepper1.Pos_Now , Stepper1.Speed_Now,Stepper2.Pos_Now , Stepper2.Speed_Now);
}


void Mode_6_Exit(void)
{
}
