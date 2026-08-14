// Orange 模块 — 初始化与通信更新
#include "Orange.h"
#include "AllHeader.h"
// 脱机调节阈值
int Pink_Sat_Lower = 0 ;
int Pink_Sat_Upper = 0 ;
int Start_x = 0 ;
int Start_y = 0 ;
int Tolerance = 0 ;
int t = 0;

// STM32->Orange
int Oran_Goal = 0 ;	// STM32发送给Orange的参数

uint8_t Oran_cmd = 0 ;	// 0. 指令模式
int Oran_real = 0 ;			// 1. 偏差
int Oran_Speed=0	;			// 2. 钢球速度

int Oran_Single_Pos = 39 ;

// Orange通信脱机阈值调节:暂时设置6个
int Oran_Param[6] ;
/*
串口Orange通信解析帧意义:
	Oran_cmd：模式，0为工作模式
	Oran_real:距离目标位置的偏差,需要-1000
	Oran_Speed:钢球移动的速度
*/

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
			Oran_real   = Serial_GetHexData(&Serial2 , 1) - 1000;
			Oran_Speed  = Serial_GetHexData(&Serial2 , 2) - 1000 ;
		}
		// 脱机阈值模式
		else if (Oran_cmd == 1)
		{
			Pink_Sat_Lower = Serial_GetHexData(&Serial2 , 1) ;
			Pink_Sat_Upper = Serial_GetHexData(&Serial2 , 2) ;
			Start_x = Serial_GetHexData(&Serial2 , 3) ;
			Start_y= Serial_GetHexData(&Serial2 , 4) ;
			Tolerance = Serial_GetHexData(&Serial2 , 5) ;
			t = Serial_GetHexData(&Serial2 , 6) ;
		}
	}
}

// 香橙派处理
void Oran_Send_Data(int* Data)
{
	if (Data == &Pink_Sat_Lower) {Serial_printf(&Serial2 , "@Pink_Sat_Lower:%d$#",Pink_Sat_Lower);}
	if (Data == &Pink_Sat_Upper) {Serial_printf(&Serial2 , "@Pink_Sat_Upper:%d$#",Pink_Sat_Upper);}
	if (Data == &Start_x) {Serial_printf(&Serial2 , "@Start_x:%d$#",Start_x);}
	if (Data == &Start_y) {Serial_printf(&Serial2 , "@Start_y:%d$#",Start_y);}
	if (Data == &Tolerance) {Serial_printf(&Serial2 , "@Tolerance:%d$#",Tolerance);}
	if (Data == &t) {Serial_printf(&Serial2 , "@t:%d$#",t);}
}
// ============================================== 香橙派位置PID(Mode2使用) ==============================================
Pid_Typedef PID_Oran ;	// 铁球PID
float Oran_Real_Offset = 0.0f ;	// real偏移量, 模拟Orange Pi发送的偏移, 可串口在线调

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

// ============================================== 球速PID->弃用 ==============================================
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

// ============================================== 串级PID -> 弃用 ==============================================
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


















