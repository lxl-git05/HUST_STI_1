#include "Con_Stepper.h"
#include "Orange.h"
// 云台运动:电机1水平旋转(顺时针为正)，电机2竖直旋转(顺时针为正)

void Stepper_Init(void)
{
	// 假设 1.8° 步进角，16 细分 → 0.1125°/脉冲，正方向暂用STEPPER_DIR_P
	Stepper_PWM_Init(&Stepper1, &MyPWM_Stepper1, &MyGPIO_Stepper_Dir, 0.1125f, STEPPER_DIR_P);
	Stepper_PWM_Init(&Stepper2, &MyPWM_Stepper2, &MyGPIO_Stepper2_Dir, 0.1125f, STEPPER_DIR_P);
	
	PID_Init(&Stepper1.PID_Angle , 0.217f , 0.0f , 0.829f , 100.0f , -100.0f , 1000.0f) ;
	PID_Init(&Stepper2.PID_Angle , 0.081f , 0.0f , 0.224f , 100.0f , -100.0f , 1000.0f) ;

	// 软件限位配置
//	Stepper_PWM_Limit_Config(&Stepper1, 120.0f, -120.0f);  // 电机1 水平旋转 ±120°
//	Stepper_PWM_Limit_Config(&Stepper2, 50.0f,  -50.0f);   // 电机2 竖直旋转 ±50°
}

// 目标角度PID值更新
void Stepper_PID_Tick(uint32_t Gap_Time_ms)
{
	// ========= 电机1 =========
	// 得到真实值和目标值(香橙派发送来的x,y数据)
	Stepper1.PID_Angle.realPoint_Now = x_real;
	Stepper1.PID_Angle.goalPoint     = x_tar ;
	// 计算PID,得到预设值
	PID_Update(&Stepper1.PID_Angle , Stepper1.PID_Angle.realPoint_Now) ;
	// 输出预设角度
	Stepper_PWM_Speed_Set(&Stepper1 , -Stepper1.PID_Angle.setPoint , Stepper1.Acc_Val) ;

	// ========= 电机2 =========
	// 得到真实值和目标值(香橙派发送来的x,y数据)
	Stepper2.PID_Angle.realPoint_Now = y_real ;
	Stepper2.PID_Angle.goalPoint     = y_tar ;
	// 计算PID,得到预设值
	PID_Update(&Stepper2.PID_Angle , Stepper2.PID_Angle.realPoint_Now) ;
	// 输出预设角度
	Stepper_PWM_Speed_Set(&Stepper2 , Stepper2.PID_Angle.setPoint , Stepper2.Acc_Val) ;
}

// 检测是否到达目标位置
bool Stepper_PID_Is_OK(Stepper_PWM_Typedef *pStepper , int Tolerance_Angle , int Tolerance_Speed) 
{
	if (pStepper->PID_Angle.goalPoint - pStepper->PID_Angle.realPoint_Now > -Tolerance_Angle && 
			pStepper->PID_Angle.goalPoint - pStepper->PID_Angle.realPoint_Now <  Tolerance_Angle && 
			pStepper->PID_Angle.setPoint < Tolerance_Speed && pStepper->PID_Angle.setPoint > -Tolerance_Speed)
	{
		return true ;
	}
	return false ;
}
