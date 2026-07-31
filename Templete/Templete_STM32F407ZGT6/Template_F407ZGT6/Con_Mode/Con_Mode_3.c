#include "Con_Mode_3.h"
// 小车在静止状态时，要求摆杆控制装置控制小球从摆杆中心点O往+5cm处运行，
// 到达后折返，再运行到-5cm处并稳定在该点附近，要求运行时间≤5s，
// ±5cm处的最大误差绝对值≤1cm。

// 阶跃方向宏: +1=先+5cm再-5cm, -1=先-5cm再+5cm
#define CON_MODE_3_STEP_DIR  (-1)

// KEY1 阶跃+斜坡测试状态机
enum { S_IDLE, S_STEP, S_RAMP };
static int  test_state = S_IDLE;
static int  ramp_cnt   = 0;       // 斜坡计数 0~100 (2s / 20ms = 100)
static int  step_wait  = 0;       // 阶跃后最小等待帧数(防误触发)

void Con_Mode_3_Setup(void)
{
    Oran_PID_Init() ;
    PID_Param_Reset(&PID_Oran) ;  // 清空误差/积分历史,防止上一模式残留
    PID_Oran.goalPoint = 0.0f ;   // goal恒为0, 偏移通过RealOff模拟
    Oran_Real_Offset = 0.0f ;
    test_state = S_IDLE;
    // 内环角度PID: 不依赖启动默认值, 防止被Mode_5改掉后切回时残留
    Stepper_PWM_Angle_Gains_Set(&Stepper1, 4.0f, 0.0f, 0.829f, 50.0f, -50.0f);
}

#include "math.h"

void Con_Mode_3_Loop(void)
{
	OLED_Printf(0, 0, OLED_6X8, "=====Con_Mode_3=====") ;
	Serial_SetFloatData(&Serial1, "Kp", "Kp=%f", &Oran_Damping_K);
	Serial_SetFloatData(&Serial1, "Ki", "Ki=%f", &PID_Oran.Ki);
	Serial_SetFloatData(&Serial1, "Kd", "Kd=%f", &PID_Oran.Kd);
	Serial_SetFloatData(&Serial1, "Goal", "Goal=%f", &Oran_Real_Offset);
	Serial_SetFloatData(&Serial1, "KpHi", "KpHi=%f", &Oran_KpHi);
	// OLED展示
	OLED_Printf(0,10,OLED_6X8,"%.1f,%.1f,%.1f",PID_Oran.Kp , PID_Oran.Ki , PID_Oran.Kd) ;
	OLED_Printf(0,20,OLED_6X8,"%.1f,%.1f,%.1f", PID_Oran.goalPoint, PID_Oran.realPoint_Now, PID_Oran.setPoint);
	// KEY1 单击 → 启动阶跃+斜坡测试
	if (Key_Check(KEY_1 , KEY_SINGLE))
	{
		Oran_Real_Offset = CON_MODE_3_STEP_DIR * Oran_Single_Pos * 5 ;
		test_state = S_STEP;
		ramp_cnt   = 0;
		step_wait  = 0;
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
//	Serial_printf(&Serial1 , "%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%d,%.1f\n",
//	PID_Oran.goalPoint, PID_Oran.realPoint_Now,
//	PID_Oran.pout, PID_Oran.iout, PID_Oran.dout, PID_Oran.setPoint, Oran_Speed,Y8U_GetSpeed());
	
//	Serial_printf(&Serial1 , "%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%d,%.1f\n",PID_Oran.goalPoint, PID_Oran.realPoint_Now
//								,PID_Oran.setPoint, Y8U_GetSpeed(), IMU_Yaw_Abs_Get() , ff_angle * 1000 , Oran_Speed , IMU_Get_Ax() * 1000);
	
	Serial_printf(&Serial1 , "%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f\n",PID_Oran.goalPoint, PID_Oran.realPoint_Now
								,PID_Oran.setPoint, Y8U_PID.goalPoint, Y8U_PID.realPoint_Now,Y8U_PID.setPoint, IMU_Yaw_Abs_Get());

	// === 阶跃+斜坡状态机 ===
	switch (test_state)
	{
		case S_STEP:
			// 至少等25帧(0.5s) + 球稳定: |realPoint_Now| < 20
			step_wait++;
			if (step_wait > 25 && fabs(PID_Oran.realPoint_Now) < Oran_Single_Pos / 2)
			{
				test_state = S_RAMP;
				ramp_cnt   = 0;
			}
			break;

		case S_RAMP:
			// 2秒内从 first_target → -first_target (100 ticks × 20ms)
			ramp_cnt++;
			Oran_Real_Offset = CON_MODE_3_STEP_DIR * (Oran_Single_Pos * 5 - (Oran_Single_Pos / 100.0f) * ramp_cnt);
			if (ramp_cnt >= 100)
			{
				Oran_Real_Offset = -CON_MODE_3_STEP_DIR * Oran_Single_Pos * 5;
				test_state = S_IDLE;
			}
			break;

		default: break;
	}
}

void Con_Mode_3_Exit(void)
{
	Stepper_PWM_Stop(&Stepper1);
}
