#include "AllHeader.h"

//#define Motoe_A_PID_Check

//float Motor_PID_Goal_Check = 0 ;

void Mode_1_Setup(void)
{
//   OLED_Clear() ;
}

// 功能:检查电机A的PID
void Mode_1_Loop(void)
{
//		OLED_Printf(0, 0, OLED_6X8, "=====Mode_1=====") ;
//    // OLED展示真实速度
//    OLED_Printf(0 , 10, OLED_6X8, "A:%.0f" ,  Motor_A.PID_s.realPoint_Now) ;
//    OLED_Printf(60, 10, OLED_6X8, "B:%.0f" ,  Motor_B.PID_s.realPoint_Now) ;
//    // Serial参数更改
//    if (Serial_GetNewPackageFlag_ABC(&Serial1))
//    {
//#ifdef  Motoe_A_PID_Check
//        // 得到数据
//        Serial_SetFloatData(&Serial1, "Kp", "Kp=%f", &Motor_A.PID_s.Kp) ;
//        Serial_SetFloatData(&Serial1, "Ki", "Ki=%f", &Motor_A.PID_s.Ki) ;
//        Serial_SetFloatData(&Serial1, "Kd", "Kd=%f", &Motor_A.PID_s.Kd) ;
//        Serial_SetFloatData(&Serial1, "goalPoint_A", "goalPoint_A=%f", &Motor_PID_Goal_Check) ;
//        Motor_SetSpeed(&Motor_A, Motor_PID_Goal_Check) ;
//        // OLED展示PID参数
//        OLED_Printf(0, 30, OLED_6X8, "%.2f,%.2f,%.2f" , Motor_A.PID_s.Kp , Motor_A.PID_s.Ki , Motor_A.PID_s.Kd) ;
//#else 
//        Serial_SetFloatData(&Serial1, "Kp", "Kp=%f", &Motor_B.PID_s.Kp) ;
//        Serial_SetFloatData(&Serial1, "Ki", "Ki=%f", &Motor_B.PID_s.Ki) ;
//        Serial_SetFloatData(&Serial1, "Kd", "Kd=%f", &Motor_B.PID_s.Kd) ;
//        Serial_SetFloatData(&Serial1, "goalPoint_B", "goalPoint_B=%f", &Motor_PID_Goal_Check) ;
//        Motor_SetSpeed(&Motor_B, Motor_PID_Goal_Check) ;
//        // OLED展示PID参数
//        OLED_Printf(0, 30, OLED_6X8, "%.2f,%.2f,%.2f" , Motor_B.PID_s.Kp , Motor_B.PID_s.Ki , Motor_B.PID_s.Kd) ;
//#endif  
//    }
}

// 打印电机A参数
void Mode_1_Tick(void)
{
//#ifdef  Motoe_A_PID_Check
//    Serial_printf(&Serial1, "%.2f,%.2f,%.2f\n",Motor_A.PID_s.goalPoint ,Motor_A.PID_s.realPoint_Now ,Motor_A.PID_s.setPoint );
//#else 
//    Serial_printf(&Serial1, "%.2f,%.2f,%.2f\n",Motor_B.PID_s.goalPoint ,Motor_B.PID_s.realPoint_Now ,Motor_B.PID_s.setPoint );
//#endif
}

void Mode_1_Exit(void)
{

}
