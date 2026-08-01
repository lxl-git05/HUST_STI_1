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
    Oran_Cascade_Init() ;          // 串级PID: 位置环→速度环
    // 位置环(外环)参数
    PID_Oran.Kp = 1.04931f ;
    PID_Oran.Ki = 0.05317f ;
    PID_Oran.Kd = 0.849f   ;
    // 速度环(内环)参数: 已在Oran_Cascade_Init→Oran_Speed_PID_Init中设为Kp=0.07277, Ki=Kd=0
    PID_Param_Reset(&PID_Oran) ;
    PID_Oran.goalPoint = 0.0f ;    // 目标位置=中心
    test_state = S_IDLE;
    // 内环角度PID: 不依赖启动默认值, 防止被Mode_5改掉后切回时残留
    Stepper_PWM_Angle_Gains_Set(&Stepper1, 4.0f, 0.0f, 0.829f, 200.0f, -200.0f);
}

#include "math.h"

void Con_Mode_3_Loop(void)
{
	OLED_Printf(0, 0, OLED_6X8, "=====Con_Mode_3=====") ;
	// 命令: 调节外环(位置环)PID
	Serial_SetFloatData(&Serial1, "Kp", "Kp=%f", &PID_Oran.Kp);
	Serial_SetFloatData(&Serial1, "Ki", "Ki=%f", &PID_Oran.Ki);
	Serial_SetFloatData(&Serial1, "Kd", "Kd=%f", &PID_Oran.Kd);
	Serial_SetFloatData(&Serial1, "Goal", "Goal=%f", &PID_Oran.goalPoint);
	// OLED展示
	OLED_Printf(0,10,OLED_6X8,"Kp:%.2f Ki:%.3f Kd:%.1f", PID_Oran.Kp, PID_Oran.Ki, PID_Oran.Kd) ;
	OLED_Printf(0,20,OLED_6X8,"Goal:%.0f Real:%d", PID_Oran.goalPoint, Oran_real);
	// KEY1 单击 → 启动阶跃+斜坡测试
	if (Key_Check(KEY_1 , KEY_SINGLE))
	{
		PID_Oran.goalPoint = CON_MODE_3_STEP_DIR * Oran_Single_Pos * 5 ;
		test_state = S_STEP;
		ramp_cnt   = 0;
		step_wait  = 0;
	}
}

void Con_Mode_3_Tick(void)
{
	Oran_Update() ;              // 刷新位置数据
	// 串级PID: 位置环→速度环→步进电机
	Oran_Cascade_Update() ;

	// RGB_R指示灯: |位置误差|>1cm 亮红灯
	{
		float err = PID_Oran.PreError ;  // goal - real
		RGB_Set_Color((err > Oran_Single_Pos || err < -Oran_Single_Pos) ? 1 : 0, 0, 0) ;
	}
	// 串级CSV: GoalPos, RealPos, CalcSpd, StepperSet, sPout, sIout, sDout
	Serial_printf(&Serial1 , "%.1f,%d,%d,%.1f,%.1f,%.1f,%.1f\n",
		PID_Oran.goalPoint, Oran_real, Oran_Speed_Calc,
		PID_Oran_Speed.setPoint, PID_Oran_Speed.pout, PID_Oran_Speed.iout, PID_Oran_Speed.dout);

	// === 阶跃+斜坡状态机 ===
	switch (test_state)
	{
		case S_STEP:
			// 至少等25帧(0.5s) + 球稳定: |位置误差| < 0.5cm
			step_wait++;
			if (step_wait > 25 && fabs(PID_Oran.PreError) < Oran_Single_Pos / 2)
			{
				test_state = S_RAMP;
				ramp_cnt   = 0;
			}
			break;

		case S_RAMP:
			// 2秒内从 first_target → -first_target (100 ticks × 20ms)
			ramp_cnt++;
			PID_Oran.goalPoint = CON_MODE_3_STEP_DIR * (Oran_Single_Pos * 5 - (Oran_Single_Pos / 100.0f) * ramp_cnt);
			if (ramp_cnt >= 100)
			{
				PID_Oran.goalPoint = -CON_MODE_3_STEP_DIR * Oran_Single_Pos * 5;
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
