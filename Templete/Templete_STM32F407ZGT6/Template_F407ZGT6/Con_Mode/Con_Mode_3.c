#include "Con_Mode_3.h"
// 小车在静止状态时，要求摆杆控制装置控制小球从摆杆中心点O往+5cm处运行，
// 到达后折返，再运行到-5cm处并稳定在该点附近，要求运行时间≤5s，
// ±5cm处的最大误差绝对值≤1cm。

// 阶跃方向宏: +1=先+5cm再-5cm, -1=先-5cm再+5cm
#define CON_MODE_3_STEP_DIR  (1)

// KEY1 S曲线轨迹测试状态机
enum { S_IDLE, S_GOTO_POS, S_HOLD_POS, S_GOTO_NEG, S_HOLD_NEG };
static int  test_state = S_IDLE;
static int  traj_tick  = 0;       // 轨迹计时 tick (20ms/tick)

void Con_Mode_3_Setup(void)
{
    Oran_PID_Init() ;
    PID_Param_Reset(&PID_Oran) ;  // 清空误差/积分历史,防止上一模式残留
    PID_Oran.goalPoint = 0.0f ;   // goal恒为0, 偏移通过RealOff模拟
    Oran_Real_Offset = 0.0f ;
    test_state = S_IDLE;
    Oran_FF_Enable = 0.0f ;   // 小车静止, 关闭加速度前馈
    // 内环角度PID: 不依赖启动默认值, 防止被Mode_5改掉后切回时残留
    Stepper_PWM_Angle_Gains_Set(&Stepper1, 4.0f, 0.0f, 0.829f, 50.0f, -50.0f);
}

#include "math.h"

// S曲线平滑阶跃: f(0)=0, f(1)=1, f'(0)=f'(1)=0
static float s_curve_step(int tick, int total)
{
    if (tick >= total) return 1.0f;
    float x = (float)tick / (float)total;
    return 3.0f * x * x - 2.0f * x * x * x;
}

// 双层速度阻尼: 轨迹轻阻尼防冲, 保持强阻尼制动
#define HOLD_DAMPING_K   0.12f    // 保持阶段: 强阻尼制动
#define TRAJ_DAMPING_K   0.09f    // 轨迹阶段: 轻阻尼, 允许运动但不放任

void Con_Mode_3_Loop(void)
{
	OLED_Printf(0, 0, OLED_6X8, "=====Con_Mode_3=====") ;
	Serial_SetFloatData(&Serial1, "Kp", "Kp=%f", &PID_Oran.Kp);
	Serial_SetFloatData(&Serial1, "Ki", "Ki=%f", &PID_Oran.Ki);
	Serial_SetFloatData(&Serial1, "Kd", "Kd=%f", &PID_Oran.Kd);
	Serial_SetFloatData(&Serial1, "Goal", "Goal=%f", &Oran_Real_Offset);
	// OLED展示
	OLED_Printf(0,10,OLED_6X8,"%.1f,%.1f,%.1f",PID_Oran.Kp , PID_Oran.Ki , PID_Oran.Kd) ;
	OLED_Printf(0,20,OLED_6X8,"%.1f,%.1f,%.1f", PID_Oran.goalPoint, PID_Oran.realPoint_Now, PID_Oran.setPoint);
	// KEY1 单击 → 启动 S 曲线轨迹: O→+5cm→-5cm
	if (Key_Check(KEY_1 , KEY_SINGLE))
	{
		PID_Param_Reset(&PID_Oran);         // 清积分历史
		Oran_Damping_K = TRAJ_DAMPING_K;    // 轨迹轻阻尼
		test_state = S_GOTO_POS;
		traj_tick  = 0;
	}
}

void Con_Mode_3_Tick(void)
{
	Oran_Update() ;              // 刷新位置数据,确保PID用最新值
	// 位置PID → 位置 (已内置 Damping_K × Oran_Speed 速度阻尼)
	Oran_PID_Update() ;
	// RGB_R指示灯: |real|>45 亮红灯
	{
		float r = PID_Oran.realPoint_Now ;
		RGB_Set_Color((r > Oran_Single_Pos || r < -Oran_Single_Pos) ? 1 : 0, 0, 0) ;
	}
	// 位置PID CSV: Goal, Real(Re+Off), Pout, Iout, Dout, Output, Spd
	Serial_printf(&Serial1 , "%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%d,%.1f\n",
	PID_Oran.goalPoint, PID_Oran.realPoint_Now,
	PID_Oran.pout, PID_Oran.iout, PID_Oran.dout, PID_Oran.setPoint, Oran_Speed,Y8U_GetSpeed());
	
//	Serial_printf(&Serial1 , "%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%d,%.1f\n",PID_Oran.goalPoint, PID_Oran.realPoint_Now
//								,PID_Oran.setPoint, Y8U_GetSpeed(), IMU_Yaw_Abs_Get() , ff_angle * 1000 , Oran_Speed , IMU_Get_Ax() * 1000);
	
//	Serial_printf(&Serial1 , "%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f\n",PID_Oran.goalPoint, PID_Oran.realPoint_Now
//								,PID_Oran.setPoint, Y8U_PID.goalPoint, Y8U_PID.realPoint_Now,Y8U_PID.setPoint, IMU_Yaw_Abs_Get());

	// === S 曲线轨迹状态机 ===
	switch (test_state)
	{
		case S_GOTO_POS:
			// S曲线: 0 → +5cm, 75 ticks (1.5s), 轻阻尼
			traj_tick++;
			{
				float frac = s_curve_step(traj_tick, 75);
				Oran_Real_Offset = CON_MODE_3_STEP_DIR * Oran_Single_Pos * 5 * frac;
				if (traj_tick >= 75)
				{
					Oran_Real_Offset = CON_MODE_3_STEP_DIR * Oran_Single_Pos * 5;
					PID_Oran.SumError = 0.0f;              // 清积分防过冲
					Oran_Damping_K   = HOLD_DAMPING_K;     // 强阻尼制动
					test_state = S_HOLD_POS;
					traj_tick  = 0;
				}
			}
			break;

		case S_HOLD_POS:
			// 稳定 25 ticks (0.5s), 强阻尼制动中
			traj_tick++;
			if (traj_tick >= 25)
			{
				PID_Oran.SumError = 0.0f;                  // 清积分, 新轨迹从零开始
				Oran_Damping_K   = TRAJ_DAMPING_K;         // 切轻阻尼, 平滑过渡不断崖
				test_state = S_GOTO_NEG;
				traj_tick  = 0;
			}
			break;

		case S_GOTO_NEG:
			// S曲线: +5cm → -5cm, 100 ticks (2.0s), 轻阻尼
			traj_tick++;
			{
				float frac = s_curve_step(traj_tick, 100);
				Oran_Real_Offset = CON_MODE_3_STEP_DIR * Oran_Single_Pos * 5 * (1.0f - 2.0f * frac);
				if (traj_tick >= 100)
				{
					Oran_Real_Offset = -CON_MODE_3_STEP_DIR * Oran_Single_Pos * 5;
					PID_Oran.SumError = 0.0f;              // 清积分防过冲
					Oran_Damping_K   = HOLD_DAMPING_K;     // 强阻尼制动
					test_state = S_HOLD_NEG;
				}
			}
			break;

		case S_HOLD_NEG:
			// 任务完成, 保持在 -5cm, 强阻尼制动中
			break;

		default: break;
	}
}

void Con_Mode_3_Exit(void)
{
	Stepper_PWM_Stop(&Stepper1);
}
