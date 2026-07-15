// ========================== 实验模式 — Con_Task 测试 ==========================
// 预设任务序列:
//   1. Motor_A 以 30rpm 正转 5s
//   2. Motor_A 旋转到 360°
// 手动入队:
//   KEY_1 单击 → 追加 TASK_MOTOR_SPEED (30rpm, 5s)
//   KEY_2 单击 → 追加 TASK_MOTOR_ANGLE (360°, 容差20)
//   KEY_3 单击 → 清空队列 + 停车
#include "Mode_2.h"
#include "AllHeader.h"

// ==================== 任务回调 ====================

// TASK_MOTOR_SPEED: p[0]=速度rpm, p[1]=持续时间ms
static void Task_MotorSpeed_Setup(float p[4])
{
    Motor_SetSpeed(&Motor_A, p[0]);
    p[2] = HAL_GetTick();  // 记录开始时间戳
}

static bool Task_MotorSpeed_IsExit(float p[4])
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

// TASK_MOTOR_ANGLE: p[0]=目标角度°, p[1]=容差°
static void Task_MotorAngle_Setup(float p[4])
{
    Motor_SetAngle(&Motor_A, (int)p[0]);
}

// Tick: 角度环 PID（只在角度任务活跃时由 Con_Task_Tick 调用）
static void Task_MotorAngle_Tick(float p[4])
{
    Motorx_Angle_Update_Tick(&Motor_A, 1);
}

static bool Task_MotorAngle_IsExit(float p[4])
{
    int tol = (p[1] > 0) ? (int)p[1] : 20;
		if (Motor_Is_Angle(&Motor_A, (int)p[0], tol))
		{
			// 停车
			Motor_SetSpeed(&Motor_A , 0) ;
			return true;       // 超时退出
		}
		return false ;
}

// ==================== 任务表 ====================
static const Task_Descriptor_Typedef Mode2_Task_Table[TASK_COUNT] = {
    [TASK_MOTOR_SPEED] = { .Setup = Task_MotorSpeed_Setup, .IsExit = Task_MotorSpeed_IsExit },
    [TASK_MOTOR_ANGLE] = { .Setup = Task_MotorAngle_Setup, .Tick  = Task_MotorAngle_Tick,
                           .IsExit = Task_MotorAngle_IsExit },
};

// ==================== 辅助函数 ====================
static const char* Task_Type_Str(int type)
{
    switch (type) {
        case TASK_MOTOR_SPEED: return "SPEED";
        case TASK_MOTOR_ANGLE: return "ANGLE";
        case TASK_WAIT_TIME:   return "WAIT";
        default:               return "IDLE";
    }
}

// ==================== Mode 生命周期 ====================

void Mode_2_Setup(void)
{
    OLED_Clear();

    // 初始化任务调度器（清空队列 + 终止旧任务）
    Con_Task_Init(Mode2_Task_Table, TASK_COUNT);

    // 预设任务序列
    Con_Task_Enqueue(TASK_MOTOR_SPEED, 30, 5000, 0, 0);   // 速度30rpm, 持续5s
    Con_Task_Enqueue(TASK_MOTOR_ANGLE, 360, 20, 0, 0);    // 旋转到360°, 容差20°
}

void Mode_2_Loop(void)
{
    // 任务调度
    Con_Task_Loop();

    // ---- 按键动态入队 ----
    if (Key_Check(KEY_1, KEY_SINGLE))
        Con_Task_Enqueue(TASK_MOTOR_SPEED, 30, 5000, 0, 0);

    if (Key_Check(KEY_2, KEY_SINGLE))
        Con_Task_Enqueue(TASK_MOTOR_ANGLE, 360, 20, 0, 0);

    if (Key_Check(KEY_3, KEY_SINGLE))
    {
        Con_Task_Clear();
        Motor_Stop(&Motor_A);
    }

    // ---- OLED 显示 ----
    // Line 0: 队列状态
    OLED_Printf(0, 0,  OLED_6X8, "Q:%d %s",
                Con_Task_Remaining(), Task_Type_Str(Con_Task_CurrType()));

    // Line 1: 电机参数
    OLED_Printf(0, 8,  OLED_6X8, "Spd:%.1f Ang:%.1f",
                Motor_A.PID_s.realPoint_Now, Motor_Get_Angle(&Motor_A));

    // Line 2: 按键提示
    OLED_Printf(0, 16, OLED_6X8, "K1:+Spd K2:+Ang K3:STOP");
}

void Mode_2_Tick(void)
{
    // 速度环 PID（始终运行，因为速度任务是纯 Setup+IsExit，无 Tick）
    Motor_Speed_Update_Tick(20);
}

void Mode_2_Exit(void)
{
    Motor_Stop(&Motor_A);
    OLED_Clear();
}
