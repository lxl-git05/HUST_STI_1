#include "Control.h"

// ==================== 全局共享任务表 ====================
// ★ 所有 Mode 统一引用此表: Con_Task_Init(Control_TaskTable, TASK_COUNT)
// ★ 新任务只需在此表中注册即可被所有模式使用
// ★ H743 说明: 电机双环已由 Mode_G 20ms 链的 Motor_Speed_Update_Tick 统一驱动，
//   因此角度类任务的 .Tick 置 NULL，无需任务级 PID 更新
Task_Descriptor_Typedef Control_TaskTable[TASK_COUNT] = {
    [TASK_WAIT_TIME] = {
        .Setup  = Task_Wait_Time_Setup,
        .IsExit = Task_Wait_Time_IsExit,
    },
    [TASK_Motor_Speed] = {
        .Setup  = Task_Motor_Speed_Setup,
        .IsExit = Task_Motor_Speed_IsExit,
    },
    [TASK_Motor_Angle] = {
        .Setup  = Task_Motor_Angle_Setup,
        .IsExit = Task_Motor_Angle_IsExit,
    },
    [TASK_MOTOR_A_ANGLE] = {
        .Setup  = Task_Motor_A_Angle_Setup,
        .IsExit = Task_Motor_A_Angle_IsExit,
    },
    [TASK_MOTOR_B_ANGLE] = {
        .Setup  = Task_Motor_B_Angle_Setup,
        .IsExit = Task_Motor_B_Angle_IsExit,
    },
    [TASK_MOTOR_TO] = {
        .Setup  = Task_Motor_To_Setup,
        .IsExit = Task_Motor_To_IsExit,
    },
    [TASK_SERVO_SET] = {
        .Setup  = Task_Servo_Set_Setup,
        .IsExit = Task_Servo_Set_IsExit,
    },
    [TASK_CLAW_SET] = {
        .Setup  = Task_Claw_Set_Setup,
        .IsExit = Task_Claw_Set_IsExit,
    },
};

// =========================== 通用任务 ===========================

// 1. 任务1: 等待(x)ms，然后Exit
// TASK_WAIT_TIME: p[0]=等待时间(ms)
void Task_Wait_Time_Setup(float p[4])
{
	p[2] = HAL_GetTick() ;	// 开始计时
	Buzzer_On() ;
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

// =========================== F407 原有任务实现 ===========================

// 2. 任务: 电机旋转一段时间之后停止,Exit
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

// 3. 任务:电机旋转特定角度,旋转完成之后停止,Exit
// TASK_Motor_Angle: p[0]为旋转角度 p[1]为容忍角度误差
void Task_Motor_Angle_Setup(float p[4])
{
	Motor_SetAngle(&Motor_A , (int)p[0]) ;
}

bool Task_Motor_Angle_IsExit(float p[4])
{
	// 判断静止条件（H743 版 Motor_Is_Angle 为 3 参：角度+容差）
	if (Motor_Is_Angle(&Motor_A , (int)p[0] , (int)p[1]))
	{
		Motor_SetSpeed(&Motor_A , 0) ;
		return true ;
	}
	return false ;
}

// =========================== MSPM0 新增小车任务 ===========================

// 4. 任务4:电机A旋转特定角度,旋转完成之后停止,Exit
// TASK_Motor_A_Angle: p[0]为旋转角度 p[1]为容忍角度误差
void Task_Motor_A_Angle_Setup(float p[4])
{
	Motor_SetAngle(&Motor_A , (int)p[0]) ;
}

bool Task_Motor_A_Angle_IsExit(float p[4])
{
	// 判断静止条件
	if (Motor_Is_Angle(&Motor_A , (int)p[0] , (int)p[1]))
	{
		Motor_SetSpeed(&Motor_A , 0) ;
		return true ;
	}
	return false ;
}

// 5. 任务5:电机B旋转特定角度,旋转完成之后停止,Exit
// TASK_Motor_B_Angle: p[0]为旋转角度 p[1]为容忍角度误差
void Task_Motor_B_Angle_Setup(float p[4])
{
	Motor_SetAngle(&Motor_B , (int)p[0]) ;
}

bool Task_Motor_B_Angle_IsExit(float p[4])
{
	// 判断静止条件
	if (Motor_Is_Angle(&Motor_B , (int)p[0] , (int)p[1]))
	{
		Motor_SetSpeed(&Motor_B , 0) ;
		return true ;
	}
	return false ;
}

// =========================== 晾衣机器人任务 ===========================

// 舵机角色 → 实例映射表（角色索引与 ROBOT_SERVO_* 宏一致）
static Servo_Typedef *const s_ServoMap[4] = {
    SERVO_CLAW_A, SERVO_CLAW_B, SERVO_HANGER_1, SERVO_HANGER_2
};

// 6. 任务: 双电机定位（无超时，堵转需外部急停中断）A电机是330度逆时针电机 B电机是上下电机
// TASK_MOTOR_TO: p[0]=电机(0=A/1=B), p[1]=目标角度°, p[2]=容差°
void Task_Motor_To_Setup(float p[4])
{
    Motor_Typedef *m = ((int)p[0] == 0) ? &Motor_A : &Motor_B;
    m->Angle_Ring_Enable = 1;           // 角度环由 Mode_G 20ms 链统一驱动，任务无需 Tick
    Motor_SetAngle(m, (int)p[1]);
}

bool Task_Motor_To_IsExit(float p[4])
{
    Motor_Typedef *m = ((int)p[0] == 0) ? &Motor_A : &Motor_B;
    return Motor_Is_Angle(m, (int)p[1], (int)p[2]);
}

// 7. 任务: 舵机设置，保持 p[2]ms 后 Exit（p[2]=0 立即完成）
// TASK_SERVO_SET: p[0]=角色索引, p[1]=角度, p[2]=保持ms
void Task_Servo_Set_Setup(float p[4])
{
    int idx = (int)p[0];
    if (idx < 0 || idx > 3) return;     // 越界保护
    p[3] = HAL_GetTick();               // 记录开始时间戳
    Servo_SetAngle(s_ServoMap[idx], (int16_t)p[1]);
}

bool Task_Servo_Set_IsExit(float p[4])
{
    return ((HAL_GetTick() - p[3]) >= (uint32_t)p[2]);
}

// 8. 任务: 双夹爪同步设置，两舵机同时动作，保持 p[2]ms 后 Exit
// TASK_CLAW_SET: p[0]=夹爪A角度, p[1]=夹爪B角度, p[2]=保持ms
void Task_Claw_Set_Setup(float p[4])
{
    p[3] = HAL_GetTick();               // 记录开始时间戳
    Servo_SetAngle(SERVO_CLAW_A, (int16_t)p[0]);
    Servo_SetAngle(SERVO_CLAW_B, (int16_t)p[1]);
}

bool Task_Claw_Set_IsExit(float p[4])
{
    return ((HAL_GetTick() - p[3]) >= (uint32_t)p[2]);
}
