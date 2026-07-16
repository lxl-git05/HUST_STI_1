#include "Allheader.h"

// =========================== 业务逻辑中所有需要脱机调试的变量声明 ===========================
// 1. Task_Tar_XY
float Tar_XY_Tol_Distance = 8 ;	// 8个像素点
float Tar_XY_Tol_Speed 	  = 5 ;	// 容忍速度5

// 2. Task_Down
float Down_Tar_Angle = 360 ;
float Down_Tol_Angle = 5	;

// 3. Task_Back
float Back_Tar_Angle = 30 ;
float Back_Speed_MAX = 60 ;
float Back_Acc			 = 0 	;
float Back_Tol_Angle = 5	;

// 4. Task_Elec
float Elec_Wait 		 = 1000 ;	// ms

// 5. Task_Up
float Up_Tar_Angle 	 = 0 ;
float Up_Tol_Angle 	 = 5	;

// =========================== 任务注册地:比赛任务,没实现的暂时使用蜂鸣器延时任务替代 ===========================
// 任务：移动到目标(x,y)
// Task_Tar_XY: p[0]=容忍差距位置 p[1]=容忍最小速度(视为停车的标准)

void Task_Tar_XY_Setup(float p[4])
{
	// 实验代码
	p[2] = HAL_GetTick() ;	// 开始计时
//	if (p[1] != 0)
//	{
//		Buzzer_ON() ;
//	}
//	else
//	{
//		Buzzer_OFF() ;
//	}
	// 正式代码，目标值在Tick更新
}

void Task_Tar_XY_Run(float p[4])
{
	
}
	
bool Task_Tar_XY_IsExit(float p[4])
{
	// 实验代码
//	if (HAL_GetTick() - p[2] > p[0])
//	{
//		Buzzer_OFF() ;
//		return true ;
//	}
	// 正式代码
	if (Stepper_PID_Is_OK(&Stepper1 , p[0] , p[1]) && Stepper_PID_Is_OK(&Stepper2 , p[0] , p[1]) && HAL_GetTick() - p[2] > 500)
	{
		// 到达目标位置之后停止，进入下个模式
		Stepper_PWM_Stop(&Stepper1) ;
		Stepper_PWM_Stop(&Stepper2) ;
		return true ;
	}
	return false ;
}

void Task_Tar_XY_Tick(float p[4])
{
	Stepper_PID_Tick(20) ;
	Serial_printf(&Serial1, "%.2f,%.2f,%.2f\n",Stepper1.PID_Angle.goalPoint ,Stepper1.PID_Angle.realPoint_Now ,Stepper1.PID_Angle.setPoint );
}
// 2. 向下
// Task_Down:p[0]=目标角度 p[1]=容忍完成偏差
void Task_Down_Setup(float p[4])
{
	// 实验使用
	p[2] = HAL_GetTick() ;	// 开始计时
//	if (p[1] != 0)
//	{
//		Buzzer_ON() ;
//	}
//	else
//	{
//		Buzzer_OFF() ;
//	}
	// 正式代码
	Motor_SetAngle(&Motor_A , p[0]) ;
}

void Task_Down_Run(float p[4])
{
	
}
	
bool Task_Down_IsExit(float p[4])
{
//	if (HAL_GetTick() - p[2] > p[0])
//	{
//		Buzzer_OFF() ;
//		return true ;
//	}
	// 正式代码: 判断静止条件
	if (Motor_Is_Angle(&Motor_A , p[0] , p[1]) && HAL_GetTick() - p[2] > 500)
	{
		Motor_SetSpeed(&Motor_A , 0) ;
		return true ;
	}
	return false ;
}

void Task_Down_Tick(float p[4])
{
	// 正式代码
	Motorx_Angle_Update_Tick(&Motor_A , 1) ;
}
// 3. 回到原点
// Task_Back:p[0]=目标角度 p[1]=最大速度 p[2]=加速度 p[3]=容忍完成角度
void Task_Back_Setup(float p[4])
{
//	p[2] = HAL_GetTick() ;	// 开始计时
//	if (p[1] != 0)
//	{
//		Buzzer_ON() ;
//	}
//	else
//	{
//		Buzzer_OFF() ;
//	}
	// 正式代码
	Stepper_PWM_Pos_Set_Abs(&Stepper1 , p[0] , p[1] , p[2] ) ;
	Stepper_PWM_Pos_Set_Abs(&Stepper2 , p[0] , p[1] , p[2] ) ;
}

void Task_Back_Run(float p[4])
{
	
}
	
bool Task_Back_IsExit(float p[4])
{
	if (Stepper1.Pos_Now < p[3] && Stepper1.Pos_Now > -p[3] && Stepper2.Pos_Now < p[3] && Stepper2.Pos_Now > -p[3])
	{
		Stepper_PWM_Stop(&Stepper1) ;
		Stepper_PWM_Stop(&Stepper2) ;
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
//	if (p[1] != 0)
//	{
//		Buzzer_ON() ;
//	}
//	else
//	{
//		Buzzer_OFF() ;
//	}
	// 正式代码
	Motor_SetAngle(&Motor_A , p[0]) ;
}

void Task_Up_Run(float p[4])
{
	
}
	
bool Task_Up_IsExit(float p[4])
{
//	if (HAL_GetTick() - p[2] > p[0])
//	{
//		Buzzer_OFF() ;
//		return true ;
//	}
	// 正式代码: 判断静止条件
	if (Motor_Is_Angle(&Motor_A , p[0] , p[1])  && HAL_GetTick() - p[2] > 500 )
	{
		Motor_SetSpeed(&Motor_A , 0) ;
		return true ;
	}
	return false ;
}

void Task_Up_Tick(float p[4])
{
	// 正式代码
	Motorx_Angle_Update_Tick(&Motor_A , 1) ;
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

