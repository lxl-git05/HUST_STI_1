#include "Mode_2.h"
#include "AllHeader.h"

//#define Motor_A_Angle_Enable

float Motor_Angle_Check ;

void Mode_2_Setup(void)
{
   OLED_Clear() ;
}

void Mode_2_Loop(void)
{
		OLED_Printf(0, 0, OLED_6X8, "=====Mode_Angle=====") ;
		// 按键控制电机旋转角度
		if (Key_Check(KEY_1 , KEY_SINGLE))
		{
			Motor_Angle_Check += 500 ;
		}
		if (Key_Check(KEY_2 , KEY_SINGLE))
		{
			Motor_Angle_Check -= 500 ;
		}
		Motor_SetAngle(&Motor_A , Motor_Angle_Check) ;
		Motor_SetAngle(&Motor_B , Motor_Angle_Check) ;
		// OLED展示真实速度
		Motor_Angle_Update(&Motor_A) ;
		Motor_Angle_Update(&Motor_B) ;
    OLED_Printf(0 , 10, OLED_6X8, "A_Pos: %f" ,  Motor_A.PID_Angle.realPoint_Now) ;
    OLED_Printf(60, 10, OLED_6X8, "B_Pos: %f" ,  Motor_B.PID_Angle.realPoint_Now) ;
		OLED_Printf(0,30,OLED_6X8 , "Angle: %d" ,Motor_Angle_Check ) ;
    // Serial参数更改
    if (Serial_GetNewPackageFlag_ABC(&Serial1))
    {
#ifdef Motor_A_Angle_Enable
        // 得到数据
        Serial_SetFloatData(&Serial1, "Kp", "Kp=%f", &Motor_A.PID_Angle.Kp) ;
        Serial_SetFloatData(&Serial1, "Ki", "Ki=%f", &Motor_A.PID_Angle.Ki) ;
        Serial_SetFloatData(&Serial1, "Kd", "Kd=%f", &Motor_A.PID_Angle.Kd) ;
        Serial_SetFloatData(&Serial1, "Angle", "Angle=%f", &Motor_Angle_Check) ;
        // OLED展示PID参数
        OLED_Printf(0, 30, OLED_6X8, "%.2f,%.2f,%.2f" , Motor_A.PID_Angle.Kp , Motor_A.PID_Angle.Ki , Motor_A.PID_Angle.Kd) ;
#else
				// 得到数据
				Serial_SetFloatData(&Serial1, "Kp", "Kp=%f", &Motor_B.PID_Angle.Kp) ;
				Serial_SetFloatData(&Serial1, "Ki", "Ki=%f", &Motor_B.PID_Angle.Ki) ;
				Serial_SetFloatData(&Serial1, "Kd", "Kd=%f", &Motor_B.PID_Angle.Kd) ;
				Serial_SetFloatData(&Serial1, "Angle", "Angle=%f", &Motor_Angle_Check) ;
				// OLED展示PID参数
				OLED_Printf(0, 30, OLED_6X8, "%.2f,%.2f,%.2f" , Motor_B.PID_Angle.Kp , Motor_B.PID_Angle.Ki , Motor_B.PID_Angle.Kd) ;
#endif
			
		}
		OLED_Printf(0, 50, OLED_6X8, "%.2f,%.2f" , Motor_A.PID_Angle.goalPoint / 1000 , Motor_A.PID_Angle.realPoint_Now / 1000) ;
}

// 20ms让小车旋转固定角度
void Mode_2_Tick(void)
{
#ifdef Motor_A_Angle_Enable
	Serial_printf(&Serial1, "%.2f,%.2f,%.2f\n",Motor_A.PID_Angle.goalPoint ,Motor_A.PID_Angle.realPoint_Now ,Motor_A.PID_Angle.setPoint );
#else 
	Serial_printf(&Serial1, "%.2f,%.2f,%.2f\n",Motor_B.PID_Angle.goalPoint ,Motor_B.PID_Angle.realPoint_Now ,Motor_B.PID_Angle.setPoint );
#endif
}

void Mode_2_Exit(void)
{
  
}
