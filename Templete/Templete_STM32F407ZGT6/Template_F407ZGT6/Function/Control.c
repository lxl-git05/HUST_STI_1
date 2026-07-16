#include "Control.h"

// 任务注册地:比赛任务



// 任务注册地-测试任务
// 1. 任务1：等待3s，然后Exit
// TASK_WAIT_TIME: p[0]=等待时间(ms) p[1]=是否在任务中开启蜂鸣器
void Task_Wait_Time_Setup(float p[4])
{
	p[2] = HAL_GetTick() ;	// 开始计时
	if (p[1] != 0)
	{
		Buzzer_ON() ;
	}
	else
	{
		Buzzer_OFF() ;
	}
}
	
bool Task_Wait_Time_IsExit(float p[4])
{
	if (HAL_GetTick() - p[2] > p[0])
	{
		Buzzer_OFF() ;
		return true ;
	}
	return false ;
}

// 2. 任务2: 电机旋转一段时间之后停止,Exit
// TASK_Motor_Speed: p[0]=速度rpm, p[1]=持续时间ms
void Task_Motor_Speed_Setup(float p[4])
{
    Motor_SetSpeed(&Motor_A, p[0]);
    p[2] = HAL_GetTick();  // 记录开始时间戳
}

bool Task_Motor_Speed_IsExit(float p[4])
{
    if (p[1] <= 0) return false;                 // 0=永久运行
		if ((HAL_GetTick() - p[2]) >= p[1])
		{
			// 停车
			Motor_SetSpeed(&Motor_A , 0) ;
			return true;       // 超时退出
		}
    return false ;
}

// 3. 任务3:电机旋转特定角度,旋转完成之后停止,Exit 
// TASK_Motor_Angle:p[0]为旋转角度 p[1]为容忍角度误差
void Task_Motor_Angle_Setup(float p[4])
{
	Motor_SetAngle(&Motor_A , p[0]) ;
}

bool Task_Motor_Angle_IsExit(float p[4])
{
	// 判断静止条件
	if (Motor_Is_Angle(&Motor_A , p[0] , p[1]))
	{
		Motor_SetSpeed(&Motor_A , 0) ;
		return true ;
	}
	return false ;
}

void Task_Motor_Angle_Tick(float p[4])
{
	Motorx_Angle_Update_Tick(&Motor_A , 1) ;
} 

