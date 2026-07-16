#include "Control.h"

// 任务注册地:比赛任务,没实现的暂时使用蜂鸣器延时任务替代
// 1. 前往任务地点(x,y)
void Task_Tar_XY_Setup(float p[4])
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

void Task_Tar_XY_Run(float p[4])
{
	
}
	
bool Task_Tar_XY_IsExit(float p[4])
{
	if (HAL_GetTick() - p[2] > p[0])
	{
		Buzzer_OFF() ;
		return true ;
	}
	return false ;
}

void Task_Tar_XY_Tick(float p[4])
{
	
}
// 2. 向下取/放棋子
void Task_Down_Setup(float p[4])
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

void Task_Down_Run(float p[4])
{
	
}
	
bool Task_Down_IsExit(float p[4])
{
	if (HAL_GetTick() - p[2] > p[0])
	{
		Buzzer_OFF() ;
		return true ;
	}
	return false ;
}

void Task_Down_Tick(float p[4])
{
	
}
// 3. 回到原点
void Task_Back_Setup(float p[4])
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

void Task_Back_Run(float p[4])
{
	
}
	
bool Task_Back_IsExit(float p[4])
{
	if (HAL_GetTick() - p[2] > p[0])
	{
		Buzzer_OFF() ;
		return true ;
	}
	return false ;
}

void Task_Back_Tick(float p[4])
{
	
}

// 4. 取/放棋子
// Task_Elec: p[0]=等待时间(ms)
void Task_Elec_Setup(float p[4])
{
	// 开始计时
	p[1] = HAL_GetTick() ;	
	// 直接开启蜂鸣器，指示正在取/放棋子
	Buzzer_ON() ;					
	// 开始取/放
	if (MyGPIO_ReadPin(&MyGPIO_Elec))	// 正在吸附->那就放下
	{
		MyGPIO_WritePin(&MyGPIO_Elec , 0) ;
	}
	else	// 为0，也就是没在吸附,那就开吸
	{
		MyGPIO_WritePin(&MyGPIO_Elec , 1) ;
	}
}

bool Task_Elec_IsExit(float p[4])
{
	if (HAL_GetTick() - p[1] > p[0])
	{
		Buzzer_OFF() ;
		return true ;
	}
	return false ;
}
// 5. 上升
// Task_Up: p[0]为上升角度 p[1]为容忍角度误差
void Task_Up_Setup(float p[4])
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

void Task_Up_Run(float p[4])
{
	
}
	
bool Task_Up_IsExit(float p[4])
{
	if (HAL_GetTick() - p[2] > p[0])
	{
		Buzzer_OFF() ;
		return true ;
	}
	return false ;
}

void Task_Up_Tick(float p[4])
{
	
}

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

