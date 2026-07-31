// Orange 模块 — 初始化与通信更新
#include "Orange.h"
#include "AllHeader.h"

// STM32->Orange
int Oran_Goal = 0 ;	// STM32发送给Orange的参数

uint8_t Oran_cmd = 0 ;	// 0. 指令模式
int Oran_real = 0 ;			// 1. 偏差
int Oran_Speed=0	;			// 2. 钢球速度

int Oran_Single_Pos = 39 ;

// α-β滤波器状态 (10ms定频, Oran_Filter_10ms使用)
static float   pos_est   = 0 ;
static float   spd_est   = 0 ;
static int     oran_meas = 0 ;   // 最新位置测量
static uint8_t oran_new  = 0 ;   // 有新测量标志
static uint8_t filt_init = 1 ;   // 滤波器初始化

// Orange通信脱机阈值调节:暂时设置6个
int Oran_Param[6] ;
/*
串口Orange通信解析帧意义:
	Oran_cmd：模式，0为工作模式
	Oran_real:距离目标位置的偏差,需要-1000
	Oran_Speed:钢球移动的速度
*/

#ifdef ORAN_OUTLIER_FILTER
// ============== Oran_real 滑窗异常剔除（参考 Y8U_CheckFinishLine） ==============
// 返回 1=正常(接受), 0=异常(拒绝,保持上一帧,异常值不污染窗口)
static uint8_t Oran_Outlier_Check(int raw)
{
    static int     window[ORAN_WINDOW_SIZE] = {0};
    static uint8_t idx     = 0;
    static uint8_t filled  = 0;
    static uint8_t rej_cnt = 0;   // 连续拒绝计数

    // |raw| ≤ MIN: 中心小摆动, 不检测, 直接接受（也不入窗, 避免零附近拖低基线）
    int abs_raw = (raw > 0) ? raw : -raw;
    if (abs_raw <= ORAN_WINDOW_MIN)
        return 1;

    // 首帧: 无条件接受（播种窗口）
    int n = filled ? ORAN_WINDOW_SIZE : idx;
    if (n == 0)
    {
        window[idx++] = raw;
        return 1;
    }

    // 窗口均值
    int sum = 0;
    for (int i = 0; i < n; i++) sum += window[i];
    int avg = sum / n;
    int abs_avg = (avg > 0) ? avg : -avg;

    // 阈值: |avg| 过小时用 MIN 兜底
    int thr = (abs_avg < ORAN_WINDOW_MIN) ? ORAN_WINDOW_MIN : abs_avg;
    int abnormal = (abs_raw > (int)((float)thr * ORAN_WINDOW_RATIO));

    if (abnormal)
    {
        // 连续异常 → 真实快速运动, 强制接受并重播种
        if (++rej_cnt >= ORAN_WINDOW_MAX_REJ)
        {
            rej_cnt = 0;
            for (int i = 0; i < ORAN_WINDOW_SIZE; i++) window[i] = raw;
            idx = 0; filled = 1;
            return 1;
        }
        return 0;    // 单帧跳变: 拒绝
    }

    // 正常: 加入环形窗口
    rej_cnt = 0;
    window[idx] = raw;
    idx = (idx + 1) % ORAN_WINDOW_SIZE;
    if (!filled && idx == 0) filled = 1;
    return 1;
}
#endif

// 香橙派数据更新,在Mode_G实现20ms固定更新
void Oran_Update(void)
{
	// 读取Serial2的消息
	if (Serial_GetNewPackageFlag_HEX(&Serial2))
	{
		// 第0位是cmd!!!所以后续都必须从1开始
		Oran_cmd = Serial_GetHexData(&Serial2 , 0) ;
		// 钢球识别模式
		if (Oran_cmd == 0)
		{
			int raw_real = Serial_GetHexData(&Serial2 , 1) - 1000;
			int raw_spd  = Serial_GetHexData(&Serial2 , 2) - 1000 ;
#ifdef ORAN_OUTLIER_FILTER
			if (Oran_Outlier_Check(raw_real))
#endif
			{
				Oran_real = raw_real;
				Oran_Speed = raw_spd ;
				oran_meas = Oran_real ;
				oran_new  = 1 ;
			}
			// 开启滤波时, 拒绝帧: Oran_real/Oran_Speed 保持上一帧, oran_new 不置位
		}
		// 
		else if (Oran_cmd == 1)
		{
			Oran_Param[0] = Serial_GetHexData(&Serial2 , 1) ;
			Oran_Param[1] = Serial_GetHexData(&Serial2 , 2) ;
			Oran_Param[2] = Serial_GetHexData(&Serial2 , 3) ;
			Oran_Param[3] = Serial_GetHexData(&Serial2 , 4) ;
			Oran_Param[4] = Serial_GetHexData(&Serial2 , 5) ;
			Oran_Param[5] = Serial_GetHexData(&Serial2 , 6) ;
		}
	}
}

// α-β滤波器: 10ms定频, 位置残差预测速度
void Oran_Filter_10ms(void)
{
	if (oran_new)
	{
		if (filt_init)
		{
			pos_est = (float)oran_meas ;
			spd_est = 0 ;
			filt_init = 0 ;
		}
		else
		{
			float pos_pred = pos_est + spd_est * 0.01f ;          // 匀速预测
			float residual = (float)oran_meas - pos_pred ;         // 偏移值
			pos_est = pos_pred + 0.30f * residual ;                // α
			spd_est = spd_est + 0.02f * residual / 0.01f ;        // β
			// 低速区(|spd|<30): 切到Orange Pi原始速度(更精确)
//			float abs_est = (spd_est > 0) ? spd_est : -spd_est ;
//			if (abs_est < 30.0f)
//				spd_est = (float)oran_raw_spd ;
		}
		oran_new = 0 ;
	}
	else if (!filt_init)
	{
		// 无新数据: 纯预测, 速度不变
		pos_est = pos_est + spd_est * 0.01f ;
	}
	// 速度死区: |spd|<8 视为静止, 抑制零附近抖动
	if (spd_est > -8.0f && spd_est < 8.0f)
		spd_est = 0 ;
	Oran_Speed = (int)spd_est ;
}

// 香橙派处理
void Oran_Send_Data(int* Data)
{
	if (Data == &Oran_Param[0]) {Serial_printf(&Serial2 , "@Oran_Param_1:%d$#",Oran_Param[0]);}
	if (Data == &Oran_Param[1]) {Serial_printf(&Serial2 , "@Oran_Param_2:%d$#",Oran_Param[1]);}
	if (Data == &Oran_Param[2]) {Serial_printf(&Serial2 , "@Oran_Param_3:%d$#",Oran_Param[2]);}
	if (Data == &Oran_Param[3]) {Serial_printf(&Serial2 , "@Oran_Param_4:%d$#",Oran_Param[3]);}
	if (Data == &Oran_Param[4]) {Serial_printf(&Serial2 , "@Oran_Param_5:%d$#",Oran_Param[4]);}
	if (Data == &Oran_Param[5]) {Serial_printf(&Serial2 , "@Oran_Param_6:%d$#",Oran_Param[5]);}
}
// ============================================== 香橙派位置PID(Mode2使用) ==============================================
Pid_Typedef PID_Oran ;	// 铁球PID
float Oran_Real_Offset = 0.0f ;	// real偏移量, 模拟Orange Pi发送的偏移, 可串口在线调
// === 变Kp: 球速越低Kp越大(球停→硬推, 球快→软控), 均可串口在线调 ===
float Oran_KpHi         = 0.50f ;  // 低速Kp(球停了就加力)
float Oran_KpLo         = 0.16f ;  // 高速Kp(不振荡)
float Oran_KpSpdThrLo   = 5.0f ;   // <此值→KpHi
float Oran_KpSpdThrHi   = 20.0f ;  // >此值→KpLo

// ================== IMU加速度前馈 ==================
// stepper° = ax × Len / Lift
float Oran_FF_Alpha = 0.30f ;  // 低通系数
float Oran_FF_Len   = 22.0f ;  // 板长 cm
float Oran_FF_Lift  = 0.038f ; // 步进每度升降 cm/°
float Oran_Damping_K = 0.0f ;  // 速度阻尼系数, 默认0=关闭, 串口在线调
float Oran_FF_Enable  = 1.0f ;  // 加速度前馈使能: 1=开(默认), 0=关(静止场景)
float ff_angle = 0 ;					 // 前馈补偿角度

#define Oran_PID_Dir (1)
#include "math.h"

// 积分分离: |球偏离中心|>45时清零积分, 用realPoint_Now避免goal跳变误杀I
static void PID_Oran_IntSep(void)
{
	PID_Oran.SumError *= 0.95f;
	if (fabs(PID_Oran.realPoint_Now) > Oran_Single_Pos && (Oran_Speed > 8.0f || Oran_Speed < -8.0f))
		PID_Oran.SumError = 0.0f ;
}

void Oran_PID_Init(void)
{
	// PID初始化: Kd 3.4→1.5(降噪声), Ki 0.002→0(PD先行,后续按需加)
	PID_Init(&PID_Oran , 0.16f , 0.013f , 5.0f , 200 , -200 , 1000) ;
	PID_Oran.PID_Func = PID_Oran_IntSep ;  // 注册积分分离回调
	PID_Oran.d_filter = 0.4f ;
}

void Oran_PID_Update(void)
{
	// 1. 香橙派更新数据,得到Real值:这个是全局任务，直接放在Mode_G
	// Oran_Update() ;
	// 2. PID数据更新:real更新 goal为0 set需要求
	
	PID_Oran.realPoint_Now = Oran_real + Oran_Real_Offset ;  // real叠加偏移, 模拟Orange Pi输入
	// 4. PID计算
	PID_Update(&PID_Oran, PID_Oran.realPoint_Now) ;
	// 5. IMU前馈: 小车加减速补偿
	{
		static float ff_ax_filt = 0.0f ;
		float ax = IMU_Get_Ax() ;
		ff_ax_filt = Oran_FF_Alpha * ax + (1.0f - Oran_FF_Alpha) * ff_ax_filt ;
		ff_angle = ff_ax_filt * Oran_FF_Len / Oran_FF_Lift * Oran_FF_Enable ;
		Stepper_Set_Angle(&Stepper1 , ff_angle + Oran_Damping_K * Oran_Speed - (PID_Oran.setPoint) * Oran_PID_Dir) ;
//		Stepper_Set_Angle(&Stepper1 , 0 + Oran_Damping_K * Oran_Speed - 0) ;
	}
}

// ============================================== 球速PID(速度环, Mode_5使用) ==============================================
Pid_Typedef PID_Oran_Speed ;	// 球速PID(独立于位置环)
#define Oran_Speed_PID_Dir (1)   //

// 变增益: 低速->高Kp(对小扰动敏感), 高速->低Kp(避免大扰动过冲)
	float Oran_SPD_Kp_Lo = 0.1f ;	// Kp_Lo: 高速时用低增益(抑制过冲)
	float Oran_SPD_Kp_Hi = 0.27f  ;	// Kp_Hi: 低速时用高增益(对小扰动敏感)
	int   Oran_SPD_Thr_Lo = 20 ;		// 低速阈值 |speed|<20 -> Kp=Kp_Hi
	int   Oran_SPD_Thr_Hi = 50 ;		// 高速阈值 |speed|>50 -> Kp=Kp_Lo

void Oran_Speed_PID_Init(void)
{
	PID_Init(&PID_Oran_Speed , 0.08f , 0.0f , 0.5f , 200 , -200 , 1000) ;   // 保守起步值, 串口ABC在线调
//	PID_Oran_Speed.d_style = 1.0f ;  // 微分先行: goal变化不产生微分冲击
	PID_Param_Reset(&PID_Oran_Speed) ;

	// 配置 Stepper1 角度跟踪 PID（内环：角度误差→速度，1ms执行）
	Stepper_PWM_Angle_Reset(&Stepper1);
}

void Oran_Speed_PID_Update(void)
{
//	int abs_spd = (Oran_Speed > 0) ? Oran_Speed : -Oran_Speed ;
//
//	// 变增益: 低速->高Kp(对小扰动敏感), 高速->低Kp(避免大扰动过冲)
//	if (abs_spd < Oran_SPD_Thr_Lo)
//		PID_Oran_Speed.Kp = Oran_SPD_Kp_Hi ;
//	else if (abs_spd > Oran_SPD_Thr_Hi)
//		PID_Oran_Speed.Kp = Oran_SPD_Kp_Lo ;
//	else      
//	{
//		float t = (float)(abs_spd - Oran_SPD_Thr_Lo) / (Oran_SPD_Thr_Hi - Oran_SPD_Thr_Lo) ;
//		PID_Oran_Speed.Kp = Oran_SPD_Kp_Hi + (Oran_SPD_Kp_Lo - Oran_SPD_Kp_Hi) * t ;
//	}

	PID_Oran_Speed.realPoint_Now = Oran_Speed ;
	PID_Update(&PID_Oran_Speed, PID_Oran_Speed.realPoint_Now) ;
	// 驱动步进电机1
//	Stepper_PWM_Speed_Set(&Stepper1 , PID_Oran_Speed.setPoint * Oran_Speed_PID_Dir , 0) ;
	Stepper_Set_Angle(&Stepper1 , -PID_Oran_Speed.setPoint * Oran_Speed_PID_Dir) ;
}

// ============================================== 串级PID(位置环→速度环, Mode_6使用) ==============================================
void Oran_Cascade_Init(void)
{
	// 位置环(外环): 输出=目标速度, 增益远小于速度环
	PID_Init(&PID_Oran , 0.473f , 0.0f , 2.689f , 100 , -100 , 300) ;
	PID_Oran.deadspace = 10 ;
	PID_Param_Reset(&PID_Oran) ;
	// 速度环(内环): 沿用Mode_5调好的参数
	Oran_Speed_PID_Init() ;
}

void Oran_Cascade_Update(void)
{
	static uint16_t dead_cnt = 0 ;  // 死区持续时间(10ms/tick)

	// 死区时长判定: |偏差|<15 且持续500ms才停
	if (Oran_real > -15 && Oran_real < 15)
	{
		dead_cnt++ ;
		if (dead_cnt > 50)
		{
			PID_Param_Reset(&PID_Oran) ;
			PID_Param_Reset(&PID_Oran_Speed) ;
			Stepper_PWM_Stop(&Stepper1) ;
			return ;
		}
	}
	else
	{
		dead_cnt = 0 ;  // 离开死区, 清零
	}

	// 外环: 位置偏差 → 目标速度
	PID_Oran.realPoint_Now = (float)Oran_real ;
	PID_Update(&PID_Oran, PID_Oran.realPoint_Now) ;

	// 位置环输出EWMA平滑, 再给速度环goal
	{
		static float goal_smooth = 0 ;
		goal_smooth = 0.30f * PID_Oran.setPoint + 0.70f * goal_smooth ;
		PID_Oran_Speed.goalPoint = goal_smooth ;
	}

	// 内环: 球速 → 步进电机
	PID_Oran_Speed.realPoint_Now = (float)Oran_Speed ;
	PID_Update(&PID_Oran_Speed, PID_Oran_Speed.realPoint_Now) ;
	Stepper_PWM_Speed_Set(&Stepper1 , PID_Oran_Speed.setPoint * Oran_Speed_PID_Dir , 0) ;
}



// ============================================== 角度跟踪测试（Mode_2独立测试用） ==============================================
float Oran_Angle_Test_Target = 0.0f ;  // 目标角度（度），可串口修改

void Oran_Angle_Test_Init(void)
{
	// 配置 Stepper1 角度跟踪 PID（内环：角度误差→速度）
	Stepper_PWM_Angle_Reset(&Stepper1);
	Oran_Angle_Test_Target = 0.0f;
	Stepper_Set_Angle(&Stepper1, 0.0f);
}

void Oran_Angle_Test_Update(void)
{
	// 每10ms更新目标角度（不变也写，确保角度模式持续激活）
	Stepper_Set_Angle(&Stepper1, Oran_Angle_Test_Target);
}

// 测试


















