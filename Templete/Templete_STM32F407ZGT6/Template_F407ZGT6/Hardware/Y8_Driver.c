#include "Y8_Driver.h"
#include <math.h>
#include <string.h>
#include "Con_Motor.h"
#include "IMU.h"

// Y8寻迹: 1为空 0为有
#define Y8_Get 0	// 包含寻迹点
#define Y8_Nul 1	// 没有寻迹到
#define Y8_Length 132.5f	// Y8到中心的Y偏移(cm)
#define Y8_FILTER_WIN 5	// 中值滤波窗口大小

uint8_t Y8_Data[8]  		 = {0};
const int8_t Y8_Width[8] = {-42,-30,-18,-6,6,18,30,42} ;	// Y8各个位置到中心的距离
float Y8_Bias 					 = 0  ;														// 每次20ms的时候查看的Y8的偏移角,进入PID计算

// ==================================================== 底层驱动 ====================================================
// 1. 微秒延时 (168MHz F407) 
// F103(72MHz): us * 8 → F407(168MHz): us * 19
// 软件延时精度±30%, 协议容差大(1~100μs均可)
static void Y8_Delay_us(uint32_t us)
{
    uint32_t count = us * 19;
    for (volatile uint32_t i = 0; i < count; i++);
}

// 2. 读取8路传感器原始值 
// 协议: CLK起始低5μs → 8个CLK脉冲(高5μs+低5μs)
//       每个下降沿后读取DAT → MSB先出, 存入LSB
// 返回: 8位数据, bit0=第1个CLK, bit7=第8个CLK, 1=白0=黑
static uint8_t Y8_Read_Sensor(void)
{
    uint8_t data = 0;
    MyGPIO_WritePin(&MYGPIO_Y8_CLK, 0);
    Y8_Delay_us(5);
    for (int i = 0; i < 8; i++) {
        MyGPIO_WritePin(&MYGPIO_Y8_CLK, 1);
        Y8_Delay_us(5);
        MyGPIO_WritePin(&MYGPIO_Y8_CLK, 0);
        Y8_Delay_us(5);
        if (MyGPIO_ReadPin(&MYGPIO_Y8_DAT))
            data |= (1 << i);
    }
    return data;
}
// ==================================================== Mid层数据滤波 ====================================================
// 1. 初始化
// CLK/DAT引脚已由CubeMX在gpio.c中配置:
//   CLK=PF15: 输出推挽, 无上下拉
//   DAT=PF13: 输入, 上拉 (匹配辅助板开漏输出)
void Y8_Drive_Init(void)
{
    MyGPIO_WritePin(&MYGPIO_Y8_CLK, 0);
}

// 2. 数据更新 (中断中调用) 
// 读取传感器 → 展开各位到 Y8_Data[]
// 注意: 辅助板未校准时始终返回0x00
void Y8_Data_Update(void)
{
    uint8_t raw = Y8_Read_Sensor();
    for (int i = 0; i < 8; i++) {
        Y8_Data[i] = (raw >> i) & 0x01;
    }
}

// 3. 得到20ms情况下的偏移角(先进行滤波)(用来计算PID)
float Y8_Angle_Bias_Get(uint16_t cnt)
{
	// 阶段0: 安全钳
	if (cnt == 0) {cnt = 1 ;}

	// ==================== 阶段1: 多采样投票 ====================
	uint8_t vote[8] = {0};
	for (uint16_t s = 0; s < cnt; s++)
	{
		uint8_t raw = Y8_Read_Sensor();
		for (int i = 0; i < 8; i++)
		{
			if (((raw >> i) & 0x01) == Y8_Get)		// 使用宏判断黑线
				vote[i]++;
		}
	}

	// ==================== 阶段2: 多数确认 ====================
	uint8_t confirm[8] = {0};
	uint8_t cnt_line = 0;
	for (int i = 0; i < 8; i++)
	{
		if (vote[i] > cnt / 2)			// 严格多数: > cnt/2 确认
		{
			confirm[i] = 1;
			cnt_line++;
		}
	}

	// 更新全局 Y8_Data 为滤波后结果（之后不再需要单独调 Y8_Data_Update）
	for (int i = 0; i < 8; i++)
		Y8_Data[i] = confirm[i];

	// ==================== 阶段3: 投票加权位置 → 角度 ====================
	static float last_valid_angle = 0.0f;

	float raw_angle;

	if (cnt_line == 0)			// 丢线: 保持上次有效角，EWMA 自然维持，丝滑回线
	{
		raw_angle = last_valid_angle;
	}
	else											// 正常: 投票权重加权平均（连续过渡，无阶梯）
	{
		// 投票权重加权: vote[i] 越大 → 该路越可信 → 权重越大
		int pos_sum = 0;
		int vote_sum = 0;
		for (int i = 0; i < 8; i++)
		{
			pos_sum  += vote[i] * Y8_Width[i];
			vote_sum += vote[i];
		}
		float pos = (float)pos_sum / (float)vote_sum;

		// 位置 → 角度: atan2(pos, Y8_Length)
		raw_angle = atan2f(pos, (float)Y8_Length) * 180.0f / 3.14159265359f;
		last_valid_angle = raw_angle;
	}

	// ==================== 阶段4: 时序中值滤波 ====================
	static float buf[Y8_FILTER_WIN] = {0.0f};
	static uint8_t buf_idx = 0;
	static uint8_t buf_full = 0;

	// 丢线帧不写入窗口（避免填充旧值，EWMA 自然维持输出）
	if (cnt_line > 0)
	{
		buf[buf_idx] = raw_angle;
		buf_idx = (buf_idx + 1) % Y8_FILTER_WIN;
		if (buf_full < Y8_FILTER_WIN) buf_full++;
	}

	float filtered_angle;
	if (buf_full >= Y8_FILTER_WIN)
	{
		// 拷贝 → 排序 → 取中位数
		float temp[Y8_FILTER_WIN];
		memcpy(temp, buf, sizeof(buf));
		for (int i = 0; i < Y8_FILTER_WIN - 1; i++)
		{
			for (int j = 0; j < Y8_FILTER_WIN - 1 - i; j++)
			{
				if (temp[j] > temp[j + 1])
				{
					float t = temp[j];
					temp[j] = temp[j + 1];
					temp[j + 1] = t;
				}
			}
		}
		filtered_angle = temp[Y8_FILTER_WIN / 2];	// 中位数
	}
	else
	{
		filtered_angle = raw_angle;		// 窗口未满, 直接用原始值
	}

	// ==================== 阶段5: EWMA + 自适应丢线衰减 + 死区 → 更新全局并返回 ====================
	// 核心: 丢线首帧快照角度+角速度→决定保持时长, 超时才衰减
	//   中心振荡: 小角度+低角速 → 短保持 → 快速衰减打破振荡
	//   S弯脱线: 大角度+高角速 → 长保持 → 维持转向等线回来
	static float ewma_bias = 0.0f;
	static uint8_t loss_ticks  = 0;  // 连续丢线计数 (1 tick = 20ms)
	static uint8_t hold_ticks  = 3;  // 本次丢线的允许保持时长 (首帧快照后不变)

	#define Y8_EWMA_ALPHA   0.45f  // S弯跟不上就往上加，抖就往下减
	#define Y8_LOSS_HOLD_MIN  3    // 最小保持 60ms (噪声免疫)
	#define Y8_LOSS_HOLD_MAX 30    // 最大保持 500ms (S弯极限)
	#define Y8_LOSS_DECAY    0.85f // 超时后每 tick 衰减 15%
	#define Y8_DEADBAND      2.7f  // 死区(°): 略高于最内圈传感器2.64°

	// 丢线计数 + 首帧快照自适应保持时长
	if (cnt_line > 0)
	{
		loss_ticks = 0;     // 有线→清零
	}
	else
	{
		if (loss_ticks == 0)  // 刚丢线首帧: 快照当前角度+角速度, 一次性决定 hold_ticks
		{
			float abs_ang = (last_valid_angle < 0.0f) ? -last_valid_angle : last_valid_angle;
			float gyro    = IMU_Yaw_Gyro_Get();   // Z轴角速度绝对值 (°/s)
			int h = (int)(Y8_LOSS_HOLD_MIN + abs_ang * 0.5f + gyro * 0.3f);
			if (h < Y8_LOSS_HOLD_MIN) h = Y8_LOSS_HOLD_MIN;
			if (h > Y8_LOSS_HOLD_MAX) h = Y8_LOSS_HOLD_MAX;
			hold_ticks = (uint8_t)h;
		}
		loss_ticks++;
	}

	// EWMA 平滑 (有线帧正常更新; 丢线帧 filtered_angle 冻结在最后一个中位数, EWMA 自然维持)
	ewma_bias += (filtered_angle - ewma_bias) * Y8_EWMA_ALPHA;

	// 自适应丢线衰减: 超时后逐步拉回直行, 只在大角度/高角速时长时间保持
	if (cnt_line == 0 && loss_ticks > hold_ticks)
		ewma_bias *= Y8_LOSS_DECAY;

	// 软死区 (smoothstep): 消除硬死区的阶跃响应
	//   中心→DEADBAND: 输出≈0 (噪声免疫)
	//   DEADBAND→2×DEADBAND: 连续递增, 无阶跃 (直道不抖)
	//   >2×DEADBAND: 全增益 (正常PID控制)
	float abs_b = (ewma_bias < 0.0f) ? -ewma_bias : ewma_bias;
	if (abs_b < Y8_DEADBAND * 2.0f) {
		float t = abs_b / (Y8_DEADBAND * 2.0f);    // 0(中心) → 1(2×死区)
		ewma_bias *= t * t * (3.0f - 2.0f * t);     // smoothstep: C²连续
	}
	Y8_Bias = ewma_bias;
	return Y8_Bias;
}

// ==================================================== 应用层PID计算 ====================================================
// Y8巡线初始化
Pid_Typedef PID_Track ;
#define PID_Track_Dir (-1)
float Track_Base_Speed = 0 ;

void Y8_Init(void)
{
	// 硬件初始化
	Y8_Drive_Init() ;
	// PID初始化
	PID_Init(&PID_Track , 4.52f , 0.0f , 5.0f , 60 , -60 , 1000) ;
}

// Y8巡线更新 + 巡线
void Y8_PID_Update(void)
{
	// 更新数据
	// PID计算:更新真实值(目标值是0)
	PID_Track.realPoint_Now = Y8_Angle_Bias_Get(10) ;
	PID_Update(&PID_Track , PID_Track.realPoint_Now) ;

	// 输出限幅: IMU角速度融合, 区分"直道漂移"和"弯道转向"
	//   角速度<5°/s  → 直道模式: 立方限幅(中心3rpm, 极柔和)
	//   角速度5~20°/s → 过渡区: 平滑混合直道/弯道限幅
	//   角速度>20°/s → 弯道模式: 全权限60rpm, 不限制
	//   limit_straight = 3 + 57×(angle/15°)³
	{
		float abs_b = (Y8_Bias < 0.0f) ? -Y8_Bias : Y8_Bias;
		float gyro   = IMU_Yaw_Gyro_Get();

		// 直道限幅 (立方曲线, 中心几乎不动作)
		float t = abs_b / 15.0f;
		if (t > 1.0f) t = 1.0f;
		float limit_straight = 3.0f + 37.0f * t * t * t;

		// 弯道限幅 (全权限)
		float limit_curve = 40.0f;

		// 角速度→混合系数 (smoothstep, 无跳变)
		float blend;
		if (gyro < 5.0f)           blend = 0.0f;   // 纯直道
		else if (gyro > 20.0f)     blend = 1.0f;   // 纯弯道
		else {
			float bt = (gyro - 5.0f) / 15.0f;
			blend = bt * bt * (3.0f - 2.0f * bt);  // smoothstep
		}

		float limit = limit_straight + (limit_curve - limit_straight) * blend;
		if (PID_Track.setPoint >  limit) PID_Track.setPoint =  limit;
		if (PID_Track.setPoint < -limit) PID_Track.setPoint = -limit;
	}

	// 配置速度
	Motor_SetSpeed(&Motor_A , Track_Base_Speed + PID_Track.setPoint * (PID_Track_Dir)) ;
	Motor_SetSpeed(&Motor_B , Track_Base_Speed + PID_Track.setPoint *(-PID_Track_Dir)) ;
}







