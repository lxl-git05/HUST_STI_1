// ==================== Mode_3-PWM驱动步进电机 ====================
#include "Mode_3.h"
#include "AllHeader.h"

/* 当前驱动器细分按3200 STEP/圈配置；修改驱动器细分时必须同步修改。 */
#define MODE3_PULSES_PER_REV      3200L

/*
 * KEY1单击的往返丢步压力测试。
 * 最大速度8000pps = 8000/3200*60 = 150RPM，低于用户要求的500RPM。
 * 最大位置为正负4圈；如果机构存在机械限位，应先减小目标数组再运行。
 */
typedef struct
{
	int32_t target_pulses;
	uint32_t max_speed_pps;
	uint32_t accel_pps2;
	uint32_t decel_pps2;
} Mode3_TestSegment_t;

static const Mode3_TestSegment_t Mode3_StressSequence[] =
{
	{  3 * MODE3_PULSES_PER_REV, 6000U,  9000U,  9000U }, /* +3圈，112.5RPM */
	{ -3 * MODE3_PULSES_PER_REV, 7500U, 12000U, 10000U }, /* 反向跨越6圈 */
	{  4 * MODE3_PULSES_PER_REV, 8000U, 14000U, 12000U }, /* +4圈，150RPM */
	{ -4 * MODE3_PULSES_PER_REV, 6500U, 10000U, 14000U }, /* -4圈，反向跨越8圈 */
	{  2 * MODE3_PULSES_PER_REV, 7000U, 12000U, 12000U }, /* +2圈 */
	{ -2 * MODE3_PULSES_PER_REV, 5000U,  7000U,  9000U }, /* -2圈，降低速度 */
	{  0,                          8000U, 12000U, 12000U }  /* 最终回到软件零点 */
};

#define MODE3_STRESS_SEGMENT_COUNT \
	((uint8_t)(sizeof(Mode3_StressSequence) / sizeof(Mode3_StressSequence[0])))

static bool Mode3_StressRunning = false;
static uint8_t Mode3_StressIndex = 0U;

/* 任何人工控制命令都会取消自动序列，避免下一段覆盖人工命令。 */
static void Mode_3_CancelStressTest(void)
{
	Mode3_StressRunning = false;
	Mode3_StressIndex = 0U;
}

static bool Mode_3_StartStressSegment(uint8_t index)
{
	const Mode3_TestSegment_t *segment = &Mode3_StressSequence[index];
	Stepper_Status_t status;

	status = Stepper_MoveToTrapezoid(&Stepper1,
									 segment->target_pulses,
									 segment->max_speed_pps,
									 segment->accel_pps2,
									 segment->decel_pps2);
	return status == STEPPER_STATUS_OK;
}

static void Mode_3_StartStressTest(void)
{
	/* 只允许从静止状态启动，防止把正在执行的定位命令突然覆盖。 */
	if (Stepper_IsBusy(&Stepper1)) return;

	Mode3_StressIndex = 0U;
	Mode3_StressRunning = true;
	if (!Mode_3_StartStressSegment(Mode3_StressIndex))
	{
		Mode_3_CancelStressTest();
	}
}

static void Mode_3_ServiceStressTest(void)
{
	if (!Mode3_StressRunning || Stepper_IsBusy(&Stepper1)) return;

	/* ERROR/DISABLED不是正常到达，禁止继续下发后续运动。 */
	if (Stepper_GetState(&Stepper1) == STEPPER_STATE_ERROR ||
		Stepper_GetState(&Stepper1) == STEPPER_STATE_DISABLED)
	{
		Mode_3_CancelStressTest();
		return;
	}

	Mode3_StressIndex++;
	if (Mode3_StressIndex >= MODE3_STRESS_SEGMENT_COUNT)
	{
		/* 最后一段目标就是0；到这里说明软件脉冲位置已准确回零。 */
		Mode_3_CancelStressTest();
		return;
	}

	if (!Mode_3_StartStressSegment(Mode3_StressIndex))
	{
		Mode_3_CancelStressTest();
	}
}

void Mode_3_Setup(void)
{
	Mode_3_CancelStressTest();
	Stepper_Init(&Stepper1) ;
	Stepper_Enable(&Stepper1) ;
}

void Mode_3_Loop(void)
{
	if (Key_Check(KEY_1, KEY_LONG))
	{
		/* KEY1长按：紧急停止当前运动，并取消自动往返测试。 */
		Mode_3_CancelStressTest();
		Stepper_StopImmediate(&Stepper1) ;
	}
	else if (Key_Check(KEY_1, KEY_DOUBLE))
	{
		/* KEY1双击：取消测试，立即反向以1000pps连续运行。 */
		Mode_3_CancelStressTest();
		Stepper_RunImmediate(&Stepper1, -1000) ;
	}
	else if (Key_Check(KEY_1, KEY_SINGLE))
	{
		/* KEY1单击：执行多速度、大幅正反转压力测试，正常结束回到0。 */
		Mode_3_StartStressTest();
	}

	if (Key_Check(KEY_2, KEY_LONG))
	{
		/* KEY2长按：取消自动测试，并以6000pps^2减速到停止。 */
		Mode_3_CancelStressTest();
		Stepper_StopRamp(&Stepper1, 6000) ;
	}
	else if (Key_Check(KEY_2, KEY_DOUBLE))
	{
		/* KEY2双击：平滑过零并斜坡运行到-3000pps。 */
		Mode_3_CancelStressTest();
		Stepper_RunRamp(&Stepper1, -3000, 6000, 6000) ;
	}
	else if (Key_Check(KEY_2, KEY_SINGLE))
	{
		/* KEY2单击：斜坡运行到+3000pps。 */
		Mode_3_CancelStressTest();
		Stepper_RunRamp(&Stepper1, 3000, 6000, 6000) ;
	}

	if (Key_Check(KEY_3, KEY_LONG))
	{
		/* KEY3长按：仅空闲时把当前位置定义为新的软件零点。 */
		Mode_3_CancelStressTest();
		if (!Stepper_IsBusy(&Stepper1))
		{
			Stepper_SetPosition(&Stepper1, 0) ;
		}
	}
	else if (Key_Check(KEY_3, KEY_SINGLE ))
	{
		/* KEY3单击：梯形规划反向移动3200脉冲（按当前细分反向一圈）。 */
		Mode_3_CancelStressTest();
		Stepper_MoveByTrapezoid(&Stepper1, -3200, 3000, 6000, 6000) ;
	}
	else if (Key_Check(KEY_3, KEY_DOUBLE ))
	{
		/* KEY3双击：以800pps定速正向移动800脉冲。 */
		Mode_3_CancelStressTest();
		Stepper_MoveByConstant(&Stepper1, 800, 800) ;
	}

	/* 自动序列只在上一段完全停止后下发下一段，不阻塞按键和OLED。 */
	Mode_3_ServiceStressTest();

	OLED_Printf(0,  0, OLED_6X8, "===Mode_3 PWM===") ;
	OLED_Printf(0, 15, OLED_6X8, "S:%d V:%ld", (int)Stepper_GetState(&Stepper1),
				(long)Stepper_GetSpeed(&Stepper1)) ;
	OLED_Printf(0, 30, OLED_6X8, "P:%ld T:%ld", (long)Stepper_GetPosition(&Stepper1),
				(long)Stepper1.target_position) ;
	OLED_Printf(0, 45, OLED_6X8, "R:%lu Q:%u/%u", (unsigned long)Stepper1.move_remaining_pulses,
				(unsigned int)(Mode3_StressRunning ? Mode3_StressIndex + 1U : 0U),
				(unsigned int)MODE3_STRESS_SEGMENT_COUNT) ;
}

void Mode_3_Tick(void)
{

}

void Mode_3_1ms_Tick(void)
{
	/* 速度斜坡和定位制动距离统一在1ms节拍中计算。 */
	Stepper_Tick1ms(&Stepper1) ;
}

void Mode_3_Exit(void)
{
	/* 离开模式必须停止脉冲，防止后台继续驱动电机。 */
	Mode_3_CancelStressTest();
	Stepper_StopImmediate(&Stepper1) ;
	Stepper_Disable(&Stepper1) ;
}
