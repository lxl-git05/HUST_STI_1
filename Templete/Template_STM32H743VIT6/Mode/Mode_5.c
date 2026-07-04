#include "Mode_4.h"
#include "AllHeader.h"

int Robot_Wait_Cmd = Cmd_Wait_Start;

// 定义各项参数
void Mode_4_Setup(void)
{
   OLED_Clear() ;
   OLED_Printf(0,5,OLED_8X16,"================") ;
	 OLED_ShowChinese(0,25,"欢迎使用有衣有靠") ;
	 OLED_Printf(0,45,OLED_8X16,"================") ;
}

// 预备控制
void Mode_4_Loop(void)
{
	OLED_Printf(0, 0, OLED_6X8, "=====Mode_4=====") ;
	
	if(Key_Check(KEY_1 , KEY_SINGLE))
	{
		Robot_Wait_Cmd = Cmd_Robot_1_Go ;
	}
	if(Key_Check(KEY_2 , KEY_SINGLE))
	{
		Robot_Wait_Cmd = Cmd_Robot_2_Go ;
	}
	if (Key_Check(KEY_2 , KEY_DOUBLE))
	{
		Servo_Claw_Close() ;
	}
	
	// 控制台
	Hanger_Control_Change(&Robot_Wait_Cmd) ;
	Hanger_Control() ;
	
	// OLED展示效果
	OLED_Printf(0,20,OLED_6X8 , "S1=%d S2=%d" ,Servo_1.current_pos , Servo_2.current_pos ) ;
	OLED_Printf(0,30,OLED_6X8 , "S2=%d S4=%d" ,Servo_3.current_pos , Servo_4.current_pos ) ;
	// Motor
	OLED_Printf(0,40,OLED_6X8 , "M_A:G=%.2f,R=%.2f" ,Motor_A.PID_Angle.goalPoint , Motor_A.PID_Angle.realPoint_Now ) ;
	OLED_Printf(0,50,OLED_6X8 , "M_B:G=%.2f,R=%.2f" ,Motor_B.PID_Angle.goalPoint , Motor_B.PID_Angle.realPoint_Now ) ;
}

void Mode_4_Tick(void)
{
	
}

void Mode_4_Exit(void)
{
  
}
